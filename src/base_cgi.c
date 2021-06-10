#ifdef BASE_CGI

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <errno.h>

#include <headers.h>

#include "common.h"
#include "router.h"

extern char** environ;

void fixHeaderKey(char* key) {
	size_t len = strlen(key);
	bool first = true;
	for (size_t i = 0; i < len; i++) {
		if (key[i] == '_') {
			key[i] = '-';
			first = true;
		} else if (key[i] >= 'A' && key[i] <= 'Z') {
			if (first) {
				first = false;
			} else {
				key[i] = key[i] - 'A' + 'a';
			}
		}
	}
}

int setHttpHeaders(struct headers* headers) {
	char** e = environ;
	
	#define ENV_HEADER_PREFIX ("HTTP_")
	
	for (; *e; e++) {
		if (strncmp(*e, ENV_HEADER_PREFIX, strlen(ENV_HEADER_PREFIX)) == 0) {
			char* clone = strdup(*e);
			if (clone == NULL) {
				return -1;
			}
			
			char* value = clone;
			while (*value != '=') {
				value++;
			}
			*value = '\0';
			value++;
			
			char* key = clone + strlen(ENV_HEADER_PREFIX);
			fixHeaderKey(key);
			
			if (headers_mod(headers, key, value) < 0) {
				free(clone);
				return -1;
			}
			
			free(clone);
		}
	}
	
	return 0;
}

method_t getMethod(const char* methodString) {
	if (strcmp(methodString, "GET") == 0) {
		return HTTP_GET;
	} else if (strcmp(methodString, "POST") == 0) {
		return HTTP_POST;
	} else if (strcmp(methodString, "HEAD") == 0) {
		return HTTP_HEAD;
	} else if (strcmp(methodString, "PUT") == 0) {
		return HTTP_PUT;
	} else if (strcmp(methodString, "DELETE") == 0) {
		return HTTP_DELETE;
	} else if (strcmp(methodString, "TRACE") == 0) {
		return HTTP_TRACE;
	} else if (strcmp(methodString, "OPTIONS") == 0) {
		return HTTP_OPTIONS;
	} else if (strcmp(methodString, "CONNECT") == 0) {
		return HTTP_CONNECT;
	} else if (strcmp(methodString, "PATCH") == 0) {
		return HTTP_PATCH;
	} else {
		return -1;
	}
}

const char* or(const char* v1, const char* v2) {
	if (v1 != NULL) {
		return v1;
	} else {
		return v2;
	}
}

int main(int argc, char** argv) {
	struct headers headers = headers_create();
	if (setHttpHeaders(&headers)) {
		fprintf(stderr, "%s: %s\n", argv[0], strerror(errno));
		exit(1);
	}

	ctx_t ctx = {
		method: getMethod(or(getenv("REQUEST_METHOD"), "GET")),
		path: or(getenv("PATH_INFO"), "/"),
		queryString: or(getenv("QUERY_STRING"), ""),
		peerAddr: or(getenv("REMOTE_ADDR"), ""),
		peerPort: 0, // TODO 
		auth: getAuthData(request.headers),
		headers: headers
	};

	response_t response = routerHandler(ctx);
	if (response.output == NULL) {
		response = errorResponse(500, "route did not provide a reponse handler");
	}
	
	headers_free(&headers);
	freeAuthData(ctx.auth);

	printf("Status: %d\n\r", response.status);
	headers_dump(&response.headers, stdout);
	printf("\n\r");
	
	headers_free(&response.headers);

	response.output(stdout, response._userData, ctx);

	return 0;
}

#endif
