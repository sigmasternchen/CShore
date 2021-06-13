#include <stdlib.h>
#include <string.h>
#include <alloca.h>
#include <stdio.h>

#include <headers.h>

#include "cookies.h"

char* getCookie(ctx_t* ctx, const char* key) {
	// ignore const 
	char* cookieHeader = (char*) headers_get(&ctx->requestHeaders, "Cookie");
	if (cookieHeader == NULL) {
		return NULL;
	}
	
	cookieHeader = strdup(cookieHeader);
	if (cookieHeader == NULL) {
		return NULL;	
	}
	
	char** saveptr = NULL;
	char* str = cookieHeader;
	
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

cookieSettings_t cookieSettingsNull() {
	return (cookieSettings_t) {
		.expires = COOKIE_NO_EXPIRES,
		.maxAge = COOKIE_NO_MAX_AGE,
		.domain = NULL,
		.path = NULL,
		.secure = false,
		.httpOnly = false
	};
}

int setCookie(ctx_t* ctx, const char* name, const char* value, cookieSettings_t settings) {
	size_t length = 0;
	length += strlen(name) + 1 + strlen(value);
	if (settings.expires != COOKIE_NO_EXPIRES) {
		length += 2;
		length += strlen("Expires=");
		length += 3 + 2 + 2 + 1 + 3 + 1 + 4 + 1 + 2 + 1 + 2 + 1 + 2 + 1 + 3;
	}
	if (settings.maxAge != COOKIE_NO_MAX_AGE) {
		length += 2;
		length += strlen("Max-Age=");
		long tmp = settings.maxAge;
		if (tmp <= 0) {
			length++;
			tmp *= -1;
		}
		for (; tmp > 0; tmp /= 10) length++;
	}
	if (settings.domain != NULL) {
		length += 2;
		length += strlen("Domain=");
		length += strlen(settings.domain);
	}
	if (settings.path != NULL) {
		length += 2;
		length += strlen("Path=");
		length += strlen(settings.path);
	}
	if (settings.secure) {
		length += 2;
		length += strlen("Secure");
	}
	if (settings.httpOnly) {
		length += 2;
		length += strlen("HttpOnly");
	}
	
	char* buffer = alloca(length + 1);
	char* bufferPtr = buffer;
	size_t tmp;
	
	tmp = snprintf(bufferPtr, length + 1, "%s=%s", name, value);
	bufferPtr += tmp;
	length -= tmp;
	
	if (settings.expires != COOKIE_NO_EXPIRES) {
		struct tm result;
		gmtime_r(&settings.expires, &result);
		
		char* weekday;
		switch(result.tm_wday) {
			case 0:
				weekday = "Sun";
				break;
			case 1:
				weekday = "Mon";
				break;
			case 2:
				weekday = "Tue";
				break;
			case 3:
				weekday = "Wed";
				break;
			case 4:
				weekday = "Thu";
				break;
			case 5:
				weekday = "Fri";
				break;
			case 6:
				weekday = "Sat";
				break;
			default:
				weekday = "err";
				break;
		}
		
		char* month;
		switch(result.tm_mon) {
			case 0:
				month = "Jan";
				break;
			case 1:
				month = "Feb";
				break;
			case 2:
				month = "Mar";
				break;
			case 3:
				month = "Apr";
				break;
			case 4:
				month = "May";
				break;
			case 5:
				month = "Jun";
				break;
			case 6:
				month = "Jul";
				break;
			case 7:
				month = "Aug";
				break;
			case 8:
				month = "Sep";
				break;
			case 9:
				month = "Oct";
				break;
			case 10:
				month = "Nov";
				break;
			case 11:
				month = "Dec";
				break;
			default:
				month = "err";
				break;
		}
		
		tmp = snprintf(bufferPtr, length + 1, "; Expires=%s, %02d %s %d %02d:%02d:%02d GMT", weekday, result.tm_mday, month, result.tm_year + 1900, result.tm_hour, result.tm_min, result.tm_sec);
		bufferPtr += tmp;
		length -= tmp;
	}
	if (settings.maxAge != COOKIE_NO_MAX_AGE) {
		tmp = snprintf(bufferPtr, length + 1, "; Max-Age=%ld", settings.maxAge);
		bufferPtr += tmp;
		length -= tmp;
	}
	if (settings.domain != NULL) {
		tmp = snprintf(bufferPtr, length + 1, "; Domain=%s", settings.domain);
		bufferPtr += tmp;
		length -= tmp;
	}
	if (settings.path != NULL) {
		tmp = snprintf(bufferPtr, length + 1, "; Path=%s", settings.path);
		bufferPtr += tmp;
		length -= tmp;
	}
	if (settings.secure) {
		tmp = snprintf(bufferPtr, length + 1, "; Secure");
		bufferPtr += tmp;
		length -= tmp;
	}
	if (settings.httpOnly) {
		tmp = snprintf(bufferPtr, length + 1, "; HttpOnly");
		bufferPtr += tmp;
		length -= tmp;
	}
	
	headers_mod(&ctx->responseHeaders, "Set-Cookie", buffer);
	
	return 0;
}
