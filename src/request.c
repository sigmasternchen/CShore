#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <alloca.h>
#include <sys/stat.h>
#include <stdarg.h>

#include <headers.h>
#include <util.h>
#include <mime.h>
#include <status.h>

#include <json.h>
#include <marshaller.h>
#include <templates.h>

#include "request.h"

response_t emptyResponse() {
	return (response_t) {
		status: 500,
		headers: headers_create(),
		output: NULL
	};
}

static errorformat_t errorformat = JSON;

void setDefaultErrorFormat(errorformat_t format) {
	errorformat = format;
}

static void rawOutputAndFree(FILE* out, void* _userData, ctx_t* ctx) {
	fprintf(out, "%s", (char*) _userData);
	
	free(_userData);
}

static void rawOutput(FILE* out, void* _userData, ctx_t* ctx) {
	fprintf(out, "%s", (const char*) _userData);
}

response_t _rawResponse(int status, char* txt, bool free) {
	response_t response = emptyResponse();
	response.status = status;

	size_t size = strlen(txt);
	int length = strlenOfNumber(size);
	char* tmp = alloca(length + 1);
	snprintf(tmp, length + 1, "%ld", size);

	headers_mod(&response.headers, "Content-Length", tmp);
	response._userData = (void*) txt;
	
	if (free) {
		response.output = rawOutputAndFree;
	} else {
		response.output = rawOutput;
	}
	return response;
}

response_t rawResponse(int status, const char* txt) {
	return _rawResponse(status, (char*) txt, false);
}

struct statusdata {
	int status;
	const char* message;
};

static void statusOutput(FILE* out, void* _userData, ctx_t* ctx) {
		struct statusdata* data = (struct statusdata*) _userData;
		
		const char* statusString = getStatusStrings(data->status).statusString;
		char* tmp = getTimestamp();
		jsonValue_t* json = json_object(true, 5,
			"timestamp", json_string(tmp),
			"status", json_long(data->status),
			"error", json_string(statusString),
			"message", json_string(data->message),
			"path", json_string(ctx->path)
		);
		free(data);
		free(tmp);
		tmp = json_stringify(json);
		json_free(json);
		fputs(tmp, out);
		free(tmp);
}

response_t statusResponse(int status, const char* message) {
	if (message == NULL) {
		message = "";
	}
	
	char* tmp;
	const char* statusString;
	struct statusdata* data;
	switch(errorformat) {
		case RAW:
			statusString = getStatusStrings(status).statusString;
			tmp = malloc(strlen(statusString) + 1 + strlen(message) + 1);
			sprintf(tmp, "%s\n%s", statusString, message);
			return _rawResponse(status, tmp, true);
		case JSON:
			data = malloc(sizeof(struct statusdata));
			if (data == NULL) {
				return rawResponse(500, "Internal Server Error\ncouldn't prepare status message");
			}
			data->status = status;
			data->message = message;
			
			response_t response = emptyResponse();
			response.status = status;
			response._userData = data;
			response.output = statusOutput;
			
			return response;
		default:
			return rawResponse(500, "Internel Server Error\nunknown error format");
	}
}
response_t errorResponse(int status, const char* message) {
	return statusResponse(status, message);
}

static void fileOutput(FILE* out, void* _userData, ctx_t* ctx) {
	FILE* in = (FILE*) _userData;

	#define READ_BUFFER_SIZE (1024)

	char buffer[READ_BUFFER_SIZE];

	while((fgets(buffer, READ_BUFFER_SIZE, in)) != NULL) {
		fputs(buffer, out);
	}

	fclose(in);
}

response_t fileResponse(const char* file) {
	response_t response = emptyResponse();
	
	struct stat statObj;
	if (stat(file, &statObj) < 0) {
		return errorResponse(500, strerror(errno));
	}

	if (!S_ISREG(statObj.st_mode)) {
		return errorResponse(500, "not a file");
	}

	FILE* stream = fopen(file, "r");
	if (stream == NULL) {
		return errorResponse(500, strerror(errno));
	}

	int length = strlenOfNumber(statObj.st_size);
	char* tmp = alloca(length + 1);
	snprintf(tmp, length + 1, "%ld", statObj.st_size);

	headers_mod(&response.headers, "Content-Type", getMineFromFileName(file));
	headers_mod(&response.headers, "Content-Length", tmp);

	response.status = 200;
	response._userData = stream;
	response.output = fileOutput;

	return response;
}

static void jsonOutput(FILE* output, void* _userData, ctx_t* ctx) {
	jsonValue_t* json = (jsonValue_t*) _userData;
	
	char* result = json_stringify(json);
	json_free(json);
	
	fprintf(output, "%s", result);
	free(result);
}

response_t _jsonResponse(int status, const char* type, void* value) {
	response_t response = emptyResponse();
	
	jsonValue_t* json = _json_marshall_value(type, value);
	if (json == NULL) {
		return statusResponse(500, strerror(errno));
	}
	
	response.status = status;
	response._userData = json;
	response.output = jsonOutput;
	
	return response;
}

extern size_t _sizeTemplate(const char*, va_list);
extern void _renderTemplate(const char*, FILE*, va_list);

response_t templateResponse(int status, const char* name, ...) {
	response_t response = emptyResponse();
	
	size_t length;
	char* buffer;
	
	va_list argptr, argptr2;
	va_start(argptr, name);
	va_copy(argptr2, argptr);
	
	length = _sizeTemplate(name, argptr);
	
	va_end(argptr);
	
	buffer = malloc(length + 1);
	if (buffer == NULL) {
		va_end(argptr2);
		return statusResponse(500, strerror(errno));
	}
	
	FILE* out = fmemopen(buffer, length + 1, "w");
	if (out == NULL) {
		va_end(argptr2);
		return statusResponse(500, strerror(errno));
	}
	
	_renderTemplate(name, out, argptr2);
	fclose(out);
	
	va_end(argptr2);
	
	buffer[length] = '\0';
	
	response.status = status;
	response._userData = buffer;
	response.output = rawOutputAndFree;
	
	printf("%s\n", buffer + 100);
	printf("%zd - %zd\n", length, strlen(buffer));
	
	size_t tmpLength = snprintf(NULL, 0, "%zd", length);
	char* tmp = malloc(tmpLength + 1);
	if (tmp != NULL) {
		snprintf(tmp, tmpLength + 1, "%zd", length);
		headers_mod(&response.headers, "Content-Length", tmp);
		free(tmp);
	} else {
		// not a huge deal
		// CFloor should be able to deal with a missing content-length header
	}
	
	return response;
}

response_t next() {
	response_t response = emptyResponse();
	response.status = NEXT_RESPONSE_STATUS;
	return response;
}

response_t basicAuthResponse(int status, const char* realm) {
	response_t response = emptyResponse();
	response.status = status;
	response._userData = "";
	response.output = rawOutput;
	
	size_t bufferLength = strlen(realm) + 14;
	char buffer[bufferLength + 1];
	snprintf(buffer, bufferLength + 1, "Basic realm=\"%s\"", realm);
	headers_mod(&response.headers, "WWW-Authenticate", buffer);
	
	return response;
}
