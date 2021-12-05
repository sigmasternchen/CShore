#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include <pthread.h>

#include <headers.h>

// SESSION_PTR_TYPE doesn't matter for this file
#define SESSION_PTR_TYPE int
#include "sessions.h"

#include "cookies.h"

struct session {
	bool inUse;
	uuid_t id;
	time_t lastAccess;
    time_t lastWrite;
	void* data;
};

static struct session* sessions = NULL;
static size_t sessionno = 0;
static pthread_mutex_t globalLock = PTHREAD_MUTEX_INITIALIZER;

#define SESSION_BLOCK_SIZE (128)

int resizeSessionList() {
	struct session* tmp = realloc(sessions, sizeof(struct session) * (sessionno + SESSION_BLOCK_SIZE));
	if (tmp == NULL) {
		return -1;	
	}
	for (size_t i = 0; i < SESSION_BLOCK_SIZE; i++) {
		struct session* session = &tmp[sessionno + i];
		session->inUse = 0;
	} 
	sessions = tmp;
	sessionno += SESSION_BLOCK_SIZE;

	return 0;
}

struct session* newSession(size_t size) {
	for (size_t i = 0; i < sessionno; i++) {
		if (!sessions[i].inUse) {
			sessions[i].inUse = true;
			sessions[i].data = malloc(size);
			if (sessions[i].data == NULL) {
				sessions[i].inUse = false;
				return NULL;
			}
			memset(sessions[i].data, 0, size);
			sessions[i].lastWrite = 0;
			return &(sessions[i]);
		}
	}
	
	// no free session slot
	if (resizeSessionList() < 0) {
		return NULL;
	}
	
	return newSession(size);
}

struct session* findSession(uuid_t id) {
	for (size_t i = 0; i < sessionno; i++) {
		if (!sessions[i].inUse) {
			continue;
		}
		if (uuid_compare(sessions[i].id, id) == 0) {
			return &(sessions[i]);
		}
	}
	
	return NULL;
}

void* _session_start(ctx_t* ctx, const char* cookie, size_t size) {
	char* cookieValue = getCookie(ctx, cookie);
	
	bool isValid = false;
	uuid_t id;
	struct session* session = NULL;
	
	if (cookieValue != NULL) {
		if (uuid_parse(cookieValue, id) == 0) {
			isValid = true;
		}
		
		free(cookieValue);
	} else {
		isValid = false;
	}
	
	pthread_mutex_lock(&globalLock);
	
	if (isValid) {
		session = findSession(id);
	}
	
	if (session == NULL) {
		session = newSession(size);
		if (session == NULL) {
			return NULL;
		}
		
		uuid_generate_time(session->id);
		
		char buffer[36 + 1];
		uuid_unparse(session->id, buffer);
		
		setCookie(ctx, cookie, buffer, cookieSettingsNull());
	}
	
	pthread_mutex_unlock(&globalLock);
	
	session->lastAccess = time(NULL);

	ctx->session.session = session;
	ctx->session.accessTime = time(NULL);

	void* requestSessionData = malloc(size);
	if (requestSessionData == NULL) {
		return NULL;
	}
	memcpy(requestSessionData, session->data, size);
	ctx->session.data = requestSessionData;
	
	return requestSessionData;
}

int _session_update(ctx_t* ctx, size_t size) {
	struct session_ctx* sessionCtx = &(ctx->session);
	struct session* session = (struct session*) sessionCtx->session;
	if (session == NULL) {
		return ERROR_NO_SESSION;
	}

	pthread_mutex_lock(&globalLock);

	if (session->lastWrite > sessionCtx->accessTime) {
		pthread_mutex_unlock(&globalLock);
		return ERROR_CONCURRENT_SESSION;
	}

	session->lastWrite = time(NULL);
	sessionCtx->accessTime = session->lastWrite;
	memcpy(session->data, sessionCtx->data, size);

	pthread_mutex_unlock(&globalLock);

	return 0;
}

void session_end(ctx_t* ctx) {
	if (ctx->session.session != NULL) {
		free(ctx->session.data);
	}
}
