#ifndef COMMON_H_
#define COMMON_H_

#include <misc.h>

typedef enum method method_t;

#define HTTP_GET (GET)
#define HTTP_POST (POST)
#define HTTP_HEAD (HEAD)
#define HTTP_PUT (PUT)
#define HTTP_DELETE (DELETE)
#define HTTP_TRACE (TRACE)
#define HTTP_OPTIONS (OPTIONS)
#define HTTP_CONNECT (CONNECT)
#define HTTP_PATCH (PATCH)

void headers_merge(struct headers*, struct headers*);

#endif
