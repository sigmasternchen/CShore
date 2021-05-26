#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <stdio.h>

#include "handler.h"
#include "router.h"
#include "common.h"

#define _CAT(a, b, c, d) a ## b ## c ## d
#define _CAT2(a, b, c, d) _CAT(a, b, c, d)

#define CONTROLLER(m, p, ...) handle_t __VA_ARGS__; __attribute__((constructor)) static void _CAT2(_register_route_, f, _, __LINE__) () { if (registerRoute(m, p, __VA_ARGS__, NULL) < 0) { fprintf(stderr, "ERROR: couldn't add route %s %s (%s): already registered\n", #m, p, #__VA_ARGS__); }; }

#define GET(p, ...)     CONTROLLER(HTTP_GET, p, __VA_ARGS__)
#define POST(p, ...)    CONTROLLER(HTTP_POST, p, __VA_ARGS__)
#define HEAD(p, ...)    CONTROLLER(HTTP_HEAD, p, __VA_ARGS__)
#define PUT(p, ...)     CONTROLLER(HTTP_PUT, p, __VA_ARGS__)
#define DELETE(p, ...)  CONTROLLER(HTTP_DELETE, p, __VA_ARGS__)
#define TRACE(p, ...)   CONTROLLER(HTTP_TRACE, p, __VA_ARGS__)
#define OPTION(p, ...)  CONTROLLER(HTTP_OPTIONS, p, __VA_ARGS__)
#define CONNECT(p, ...) CONTROLLER(HTTP_CONNECT, p, __VA_ARGS__)
#define PATCH(p, ...)   CONTROLLER(HTTP_PATCH, p, __VA_ARGS__)

#endif
