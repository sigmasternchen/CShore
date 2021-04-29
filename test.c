#include <stdlib.h>
#include <stdio.h>

#include "controller.h"


GET("/", hello);
GET("/index.*", hello);
response_t hello(ctx_t ctx) {
	return rawResponse(200, "Hello World\n");
}

GET("/foobar", foobar);
response_t foobar(ctx_t ctx) {
	return fileResponse("foobar.txt");
}
