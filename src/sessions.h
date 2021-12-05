#ifndef SESSIONS_H_
#define SESSIONS_H_

#include <time.h>

#include <uuid/uuid.h>

#ifndef SESSION_PTR_TYPE
	#pragma GCC warning "session ptr type not defined"
	#define SESSION_PTR_TYPE int
#endif

#ifndef SESSION_LENGTH
	#define SESSION_LENGTH (24*60*60*1000)
#endif

#ifndef SESSION_COOKIE_NAME
	#define SESSION_COOKIE_NAME "cshore_session"
#endif

#include "request.h"

#define ERROR_NO_SESSION         (-2)
#define ERROR_CONCURRENT_SESSION (-1)

void* _session_start(ctx_t*, const char*, size_t);
int _session_update(ctx_t*, size_t);
void session_end(ctx_t*);

#define session_start(c) ((SESSION_PTR_TYPE*) _session_start(c, SESSION_COOKIE_NAME, sizeof(SESSION_PTR_TYPE)))
#define session_update(c) _session_update(c, sizeof(SESSION_PTR_TYPE))

#endif
