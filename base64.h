#ifndef BASE64_H
#define BASE64_H

#include <stddef.h>

void encode_base64(const unsigned char *src, size_t len,unsigned char *dst);

#define DECODE_STRICT     0x1
#define DECODE_TRAILING1  0x2

#define OK 0
#define ERROR_INVALID_CHAR  1
#define ERROR_TRAILING1     2

int decode_base64(int mode, const unsigned char *src, size_t *slen,unsigned char *dst, size_t *dlen);

#endif // BASE64_H

