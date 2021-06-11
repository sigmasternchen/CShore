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
	
	while((str = strtok_r(str, ";", saveptr)) != NULL) {
		if (strncmp(str, key, keyLength) == 0) {
			str = strtok_r(NULL, "=", saveptr)
			if (str == NULL) {
				// illegal cookie definition; ignore
			} else {
				value = str;
				break;
			}
		}
		
		str = NULL;
	}
	
	if (value != NULL) {
		value = strdup(value);
		if (value == NULL) {
			free(cookieHeader);
			return NULL;
		}
		
		free(cookieHeader);
		return value;
	} else {
		free(cookieHeader);
		return NULL;
	}
}
