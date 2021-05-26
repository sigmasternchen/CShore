#ifndef AUTH_H_
#define AUTH_H_

#include <stdbool.h>

#include <headers.h>

enum authType {
	NONE,
	BASIC
};

struct auth {
	enum authType type;
	struct {
		char* user;
		char* password;
	} basic;
};

struct auth getAuthData(struct headers*);

void freeAuthData(struct auth);

#endif
