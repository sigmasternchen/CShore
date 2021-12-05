#include <headers.h>

#include "common.h"

void headers_merge(struct headers* dst, struct headers* src) {
	for (int i = 0; i < src->number; i++) {
		headers_mod(dst, src->headers[i].key, src->headers[i].value);
	}
}
