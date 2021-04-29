#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <alloca.h>
#include <sys/stat.h>

#include <headers.h>
#include <util.h>
#include <mime.h>
#include <status.h>

#include <json.h>


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

void _rawOutputAndFree(FILE* out, void* _userData) {
	fprintf(out, "%s", (char*) _userData);
	
	free(_userData);
}

void _rawOutput(FILE* out, void* _userData) {
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
		response.output = _rawOutputAndFree;
	} else {
		response.output = _rawOutput;
	}
	return response;
}

response_t rawResponse(int status, const char* txt) {
	return _rawResponse(status, (char*) txt, false);
}

response_t errorResponse(int status, const char* message) {
	const char* statusString = getStatusStrings(status).statusString;
	
	if (message == NULL) {
		message = "";
	}
	
	char* tmp;
	switch(errorformat) {
		case RAW:
			tmp = malloc(strlen(statusString) + 1 + strlen(message) + 1);
			sprintf(tmp, "%s\n%s", statusString, message);
			return _rawResponse(status, tmp, true);
		case JSON:
			tmp = getTimestamp();
			jsonValue_t* json = json_object(true, 5,
				"timestamp", json_string(tmp),
				"status", json_long(status),
				"error", json_string(statusString),
				"message", json_string(message),
				"path", json_string("not yet implemented")
			);
			free(tmp);
			tmp = json_stringify(json);
			json_free(json);
			return _rawResponse(status, tmp, true);
		default:
			return rawResponse(500, "Internel Server Error\nunknown error format");
	}
}

void _fileOutput(FILE* out, void* _userData) {
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
	response.output = _fileOutput;

	return response;
}
