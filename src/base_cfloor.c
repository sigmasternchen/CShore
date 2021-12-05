#ifdef BASE_CFLOOR

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include <networking.h>
#include <logging.h>

#include "router.h"
#include "auth.h"

struct networkingConfig netConfig;

void session_end(ctx_t*);

static void handler(struct request request, struct response _response) {
	ctx_t ctx = {
		method: request.metaData.method,
		path: request.metaData.path,
		queryString: request.metaData.queryString,
		peerAddr: request.peer.addr,
		peerPort: request.peer.port,
		auth: getAuthData(request.headers),
		requestHeaders: *request.headers,
		responseHeaders: headers_create(),
		session: EMPTY_SESSION_CTX,
	};

	response_t response = routerHandler(&ctx);
	if (response.output == NULL) {
		response = errorResponse(500, "route did not provide a reponse handler");
	}
	
	freeAuthData(ctx.auth);
	session_end(&ctx);
	
	headers_merge(&ctx.responseHeaders, &response.headers);

	int fd = _response.sendHeader(response.status, &ctx.responseHeaders, &request);
	headers_free(&response.headers);
	headers_free(&ctx.responseHeaders);

	if (fd < 0) {
		error("csite: sendHeader: %s", strerror(errno));
		return;
	}

	FILE* out = fdopen(fd, "w");
	if (out == NULL) {
		error("csite: fdopen: %s", strerror(errno));
		return;
	}

	response.output(out, response._userData, &ctx);

	fclose(out);
}

static struct handler handlerGetter(struct metaData metaData, const char* host, struct bind* bind) {
	return (struct handler) {
		handler: &handler
	};
}

int main() {
	struct bind bind = {
		address: "0.0.0.0",
		port: "1337",
		ssl: false
	};
	struct headers headers = headers_create();
	headers_mod(&headers, "Server", "CShore 0.1");
	netConfig = (struct networkingConfig) {
		binds: {
			number: 1,
			binds: &bind 
		},
		connectionTimeout: DEFAULT_CONNECTION_TIMEOUT,
		maxConnections: DEFAULT_MAX_CONNECTIONS,
		defaultHeaders: headers,
		getHandler: handlerGetter
	};

	setLogging(stdout, WARN, true);
	//setLogging(stdout, HTTP_ACCESS, false);

	networking_init(netConfig);

	while (true) {
		sleep(0xffff);
	}
	return 0;
}

#endif
