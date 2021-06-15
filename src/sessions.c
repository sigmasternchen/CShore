#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <string.h>

#include <uuid/uuid.h>

#include <headers.h>

// SESSION_PTR_TYPE doesn't matter for this file
#define SESSION_PTR_TYPE int
#include "sessions.h"

#include "cookies.h"

static struct session {
	bool inUse;
	uuid_t id;
	time_t lastAccess;
	void* data;
}* sessions = NULL;
static size_t sessionno = 0;

#define SESSION_BLOCK_SIZE (128)

int resizeSessionList() {
	// TODO synchronization
	struct session* tmp = realloc(sessions, sizeof(struct session) * (sessionno + SESSION_BLOCK_SIZE));
	if (tmp == NULL) {
		return -1;	
	}
	memset(tmp + sizeof(struct session) * sessionno, 0, sizeof(struct session) * SESSION_BLOCK_SIZE);
	sessions = tmp;
	sessionno += SESSION_BLOCK_SIZE;
	
	return 0;
}

struct session* newSession(size_t size) {
	for (size_t i = 0; i < sessionno; i++) {
		if (!sessions[i].inUse) {
			// TODO synchronization
			sessions[i].inUse = true;
			sessions[i].data = malloc(size);
			if (sessions[i].data == NULL) {
				sessions[i].inUse = false;
				return NULL;
			}
			memset(sessions[i].data, 0, size);
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
	
	session->lastAccess = time(NULL);
	
	return session->data;
}
