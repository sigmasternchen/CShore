#ifndef REQUEST_H
#define REQUEST_H

#include <stdio.h>
#include <stdbool.h>

#include <headers.h>

#include "common.h"

typedef struct {
	method_t method;
	const char* path;
	const char* queryString;
} ctx_t;

typedef struct {
	int status;
	struct headers headers;

	void* _userData;
	void (*output) (FILE* conenction, void* _userData);
} response_t;

response_t emptyResponse();

response_t rawResponse(int status, const char* txt);

response_t fileResponse(const char* file);

#endif
