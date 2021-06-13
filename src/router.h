#ifndef ROUTER_H
#define ROUTER_H

#include "handler.h"

int registerRoute(method_t method, const char* path, ...);

response_t routerHandler(ctx_t* ctx);

#endif
