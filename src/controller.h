#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <stdio.h>

#include "handler.h"
#include "router.h"
#include "common.h"

#define _CAT(a, b, c, d) a ## b ## c ## d
#define _CAT2(a, b, c, d) _CAT(a, b, c, d)

#define GET(p, f) handle_t f; __attribute__((constructor)) static void _CAT2(_register_route_, f, _, __LINE__) () { if (registerRoute(HTTP_GET, p, &f) < 0) { fprintf(stderr, "ERROR: couldn't add route %s %s (%s): already registered\n", "GET", p, #f); }; }

#endif
