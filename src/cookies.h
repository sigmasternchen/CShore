#ifndef COOKIES_H_
#define COOKIES_H_

#include <time.h>
#include <stdbool.h>
#include <limits.h>

#include "request.h"

char* getCookie(ctx_t*, const char*);

#define COOKIE_NO_EXPIRES (0)
#define COOKIE_NO_MAX_AGE (LONG_MIN)

typedef struct {
	time_t expires;
	long maxAge;
	const char* domain;
	const char* path;
	bool secure;
	bool httpOnly;
} cookieSettings_t;

cookieSettings_t cookieSettingsNull();

int setCookie(ctx_t*, const char*, const char*, cookieSettings_t);

#endif
