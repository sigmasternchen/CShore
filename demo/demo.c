#include <stdlib.h>
#include <stdio.h>

#include <controller.h>

#include "entities.h"


GET("/", hello);
GET("/index.*", hello);
response_t hello(ctx_t ctx) {
	return rawResponse(200, "Hello World\n");
}

GET("/foobar", foobar);
response_t foobar(ctx_t ctx) {
	return fileResponse("demo/foobar.txt");
}

GET("/user", user);
response_t user(ctx_t ctx) {
	user_t user = {
		.username = "overflowerror",
		.github = "https://github.com/overflowerror"
	};

	return jsonResponse(200, user_t, &user);
}

GET("/template", template);
response_t template(ctx_t ctx) {
	return templateResponse(200, "demo.templ", "Page Title", "Overflow");
}
