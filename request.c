#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <alloca.h>
#include <sys/stat.h>

#include <headers.h>
#include <util.h>
#include <mime.h>


#include "request.h"

response_t emptyResponse() {
	return (response_t) {
		status: 500,
		headers: headers_create(),
		output: NULL
	};
}

void _rawOutput(FILE* out, void* _userData) {
	fprintf(out, "%s", (const char*) _userData);
}

response_t rawResponse(int status, const char* txt) {
	response_t response = emptyResponse();
	response.status = status;

	size_t size = strlen(txt);
	int length = strlenOfNumber(size);
	char* tmp = alloca(length + 1);
	snprintf(tmp, length + 1, "%ld", size);

	headers_mod(&response.headers, "Content-Length", tmp);
	response._userData = (void*) txt;
	response.output = _rawOutput;
	return response;
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
		return rawResponse(500, strerror(errno));
	}

	if (!S_ISREG(statObj.st_mode)) {
		return rawResponse(500, "not a file");
	}

	FILE* stream = fopen(file, "r");
	if (stream == NULL) {
		return rawResponse(500, "");
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
