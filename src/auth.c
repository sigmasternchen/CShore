#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <headers.h>

#include "auth.h"
#include "base64.h"

struct auth getAuthData(struct headers* headers) {
	struct auth auth = {
		.type = NONE,
		.basic = {
			.user = NULL,
			.password = NULL,
		}
	};

	const char* header = headers_get(headers, "Authorization");
	if (header == NULL) {
		// no auth header
		return auth;
	}
	
	if (strncmp(header, "Basic ", 6) != 0) {
		// unsupported auth type
		return auth;
	}
	
	header += 6;
	while (*header == ' ') header++;
	
	char* decoded = base64_decode(header);
	if (decoded == NULL) {
		// unable to parse base64
		return auth;
	}
	
	char* username = decoded;
	char* password = decoded;
	
	while(*password != '\0' && *password != ':') password++;
	
	if (*password == '\0') {
		free(decoded);
		// unable to parse credential string
		return auth;
	}
	*password = '\0';
	password++;
	
	auth.type = BASIC;
	auth.basic.user = username;
	auth.basic.password = password;
	
	return auth;
}


void freeAuthData(struct auth auth) {
	if (auth.basic.user != NULL)
		free(auth.basic.user);
		
	// password should be freed with user;
}
