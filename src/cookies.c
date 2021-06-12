#include <stdlib.h>
#include <string.h>

#include <headers.h>

#include "cookies.h"

char* getCookie(ctx_t ctx, const char* key) {
	char* cookieHeader = headers_get(&ctx.headers, "Cookie");
	if (cookieHeader == NULL) {
		return NULL;
	}
	
	cookieHeader = strdup(cookieHeader);
	if (cookieHeader == NULL) {
		return NULL;	
	}
	
	char** saveptr = NULL;
	char* str = cookieHeader;
	size_t keyLength = strlen(key);
	
	char* value = NULL;
	
	while((str = strtok_r(str, "; ", saveptr)) != NULL) {
		char* keyCandidate = str;
		str = strtok_r(NULL, "=", saveptr);
		if (str == NULL) {
			// illegal cookie definition; ignore
			continue;
		}
		
		if (strcmp(keyCandidate, key) == 0) {
			value = str;
		}
		
		str = NULL;
	}
	
	if (value != NULL) {
		value = strdup(value);
	}
	
	free(cookieHeader);
	return value;
}
