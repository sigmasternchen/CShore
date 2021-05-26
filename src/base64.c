#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "base64.h"

static unsigned char charTable[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
static unsigned char inverseTable[256];

__attribute__((constructor)) static void buildInverseTable() {
	memset(inverseTable, 0x80, 256);

	size_t tableLength = strlen((const char*) charTable);
	for (unsigned int i = 0; i < tableLength; i++) {
		inverseTable[charTable[i]] = i;
	}
	inverseTable['='] = 0;
}

char* base64_encode(const char* decoded) {
	size_t length = strlen(decoded) * 4 / 3 + 4;
	char* encoded = malloc(length + 1);
	if (encoded == NULL) {
		return encoded;
	}
	
	size_t i = 0, j = 0;
	
	while(length - j >= 3) {
		encoded[j++] = charTable[  decoded[i + 0] >> 2];
		encoded[j++] = charTable[((decoded[i + 0] & 0x03) << 4) | (decoded[i + 1] >> 4)];
		encoded[j++] = charTable[((decoded[i + 1] & 0x0f) << 2) | (decoded[i + 2] >> 6)];
		encoded[j++] = charTable[  decoded[i + 2] & 0x3f];
		i += 3;
	}
	
	if (length - j > 0) {
		encoded[j++] = charTable[decoded[i] >> 2];
		if (length - i == 1) {
			encoded[j++] = charTable[(decoded[i] & 0x03) << 4];
			encoded[j++] = '=';
		} else {
			encoded[j++] = charTable[((decoded[i] & 0x03) << 4) | (decoded[i + 1] >> 4)];
			encoded[j++] = charTable[(decoded[i + 1] & 0x0f) << 2];
		}
		encoded[j++] = '=';
	}
	
	encoded[j++] = '\0';
	
	return encoded;
}

char* base64_decode(const char* _encoded) {
	size_t length = strlen(_encoded);
	size_t count = 0;
	
	unsigned const char* encoded = (unsigned char*) _encoded;
	
	for (size_t i = 0; i < length; i++) {
		if (inverseTable[encoded[i]] != 0x80) {
			count++;
		}
	}
	
	if (count == 0 || count % 4 != 0) {
		return NULL;
	}
	
	char* decoded = malloc(count / 4 * 3);
	if (decoded == NULL) {
		return NULL;
	}
	
	int pad = 0;
	unsigned char block[4];
	
	count = 0;
	for (size_t i = 0, j = 0; i < length; i++) {
		unsigned char tmp = inverseTable[encoded[i]];
		if (tmp == 0x80) {
			continue;
		}
		
		if (encoded[i] == '=') {
			pad++;
		}
		
		block[count++] = tmp;
		if (count == 4) {
			decoded[j++] = (block[0] << 2) | (block[1] >> 4);
			decoded[j++] = (block[1] << 4) | (block[2] >> 2);
			decoded[j++] = (block[2] << 6) | (block[3]);
			count = 0;
			if (pad == 1) {
				j--;
				break;
			} else if (pad == 2) {
				j -= 2;
				break;
			} else if (pad > 0) {
				free(decoded);
				return NULL;
			}
		}
	}
	
	return decoded;
}
