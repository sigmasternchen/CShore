#ifndef REQUEST_H
#define REQUEST_H

#include <stdio.h>
#include <stdbool.h>

#include <headers.h>

#include "common.h"
#include "auth.h"

#define NEXT_RESPONSE_STATUS (0)

typedef struct {
	method_t method;
	const char* path;
	const char* queryString;
	const char* peerAddr;
	int peerPort;
	struct auth auth;
} ctx_t;

typedef struct {
	int status;
	struct headers headers;

	void* _userData;
	void (*output) (FILE* conenction, void* _userData, ctx_t ctx);
} response_t;

typedef enum {
	RAW,
	JSON
} errorformat_t;

void setDefaultErrorFormat(errorformat_t format);

response_t emptyResponse();

response_t next();

response_t basicAuthResponse(int status, const char* realm);

response_t statusResponse(int status, const char* message);
response_t errorResponse(int status, const char* message);

response_t rawResponse(int status, const char* txt);

response_t fileResponse(const char* file);

response_t _jsonResponse(int status, const char* type, void* value);
#define jsonResponse(s, t, v) _jsonResponse(s, # t, v)

response_t templateResponse(int status, const char* name, ...);

#endif
