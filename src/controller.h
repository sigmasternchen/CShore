#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <stdio.h>

#include "handler.h"
#include "router.h"
#include "common.h"

#define _CAT(a, b, c, d) a ## b ## c ## d
#define _CAT2(a, b, c, d) _CAT(a, b, c, d)

#define CONTROLLER(m, p, f) handle_t f; __attribute__((constructor)) static void _CAT2(_register_route_, f, _, __LINE__) () { if (registerRoute(m, p, &f) < 0) { fprintf(stderr, "ERROR: couldn't add route %s %s (%s): already registered\n", #m, p, #f); }; }

#define GET(p, f)     CONTROLLER(HTTP_GET, p, f)
#define POST(p, f)    CONTROLLER(HTTP_POST, p, f)
#define HEAD(p, f)    CONTROLLER(HTTP_HEAD, p, f)
#define PUT(p, f)     CONTROLLER(HTTP_PUT, p, f)
#define DELETE(p, f)  CONTROLLER(HTTP_DELETE, p, f)
#define TRACE(p, f)   CONTROLLER(HTTP_TRACE, p, f)
#define OPTION(p, f)  CONTROLLER(HTTP_OPTIONS, p, f)
#define CONNECT(p, f) CONTROLLER(HTTP_CONNECT, p, f)
#define PATCH(p, f)   CONTROLLER(HTTP_PATCH, p, f)

#endif
