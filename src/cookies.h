#ifndef COOKIES_H_
#define COOKIES_H_

#include "request.h"

char* getCookie(ctx_t, const char*);
void setCookie(ctx_t, const char*, const char*);

#endif
