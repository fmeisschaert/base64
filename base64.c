#include <string.h>
#include <stddef.h>
#include <stdint.h>

#ifndef __STDC_VERSION__
#error "at least c17 required"
#elif __STDC_VERSION__ < 201710L
#error "at least c17 required"
#endif

static inline
unsigned char base64_char_value(unsigned char c)
{
	return c >= 'A' && c <= 'Z' ? c - 'A'
	     : c >= 'a' && c <= 'z' ? c - 'a' + 26
	     : c >= '0' && c <= '9' ? c - '0' + 52
	     : c == '+'             ? 62
	     : c == '/'             ? 63
	     : c == '='             ? 0x40
	     :                        0xc0
	     ;
}

static inline
unsigned char base64_value_char(unsigned char v)
{
	 if (v >= 64) return 0;
	 v += 65;  // A-Z
	 if (v < 91) return v;
	 v += 6;   // a-z
	 if (v < 123) return v;
	 v -= 75;  // 0-9
	 if (v < 66) return v;
	 if (v == 66) return '+';
	 if (v == 67) return '/';
	 return 0;
}

// assume dst has enough space for ((len + 2) / 3) * 4 characters
void encode_base64(const unsigned char *src, size_t len,unsigned char *dst)
{
	size_t q = len / 3;
	size_t r = len % 3;

	for (size_t i = 0; i < q; i++) {
		unsigned char c0 = src[3*i];
		unsigned char c1 = src[3*i+1];
		unsigned char c2 = src[3*i+2];

		unsigned char v0 = c0 >> 2;
		unsigned char v1 = (c0 << 4 & 0x3f) | c1 >> 4;
		unsigned char v2 = (c1 << 2 & 0x3f) | c2 >> 6; 
		unsigned char v3 = c2 & 0x3f;

		dst[4*i]   = base64_value_char(v0);
		dst[4*i+1] = base64_value_char(v1);
		dst[4*i+2] = base64_value_char(v2);
		dst[4*i+3] = base64_value_char(v3);
	}

	switch (r) {
		case 1: {
				unsigned char c0 = src[3*q];

				unsigned char v0 = c0 >> 2;
				unsigned char v1 = c0 << 4 & 0x3f;

				dst[4*q]   = base64_value_char(v0);
				dst[4*q+1] = base64_value_char(v1);
				dst[4*q+2] = '=';
				dst[4*q+3] = '=';
			}
			break;
		case 2: {
				unsigned char c0 = src[3*q];
				unsigned char c1 = src[3*q+1];

				unsigned char v0 = c0 >> 2;
				unsigned char v1 = (c0 << 4 & 0x3f) | c1 >> 4;
				unsigned char v2 = c1 << 2 & 0x3f; 

				dst[4*q]   = base64_value_char(v0);
				dst[4*q+1] = base64_value_char(v1);
				dst[4*q+2] = base64_value_char(v2);
				dst[4*q+3] = '=';
			}
			break;
	}
}

#define DECODE_STRICT    0x1
#define DECODE_ERR_MOD1  0x2

#define OK 0
#define ERROR_INVALID_CHAR   1
#define ERROR_TRAILING_MOD1  2

// assume dst has enough space for ((len + 3) / 4) * 3 characters
int decode_base64(int mode, const unsigned char *src, size_t len,unsigned char *dst, size_t *dlen)
{
	size_t j = 0;

	int decode_strict = mode & DECODE_STRICT;
	int decode_err_mod1 = mode & DECODE_ERR_MOD1;

	uint_least32_t w = 0;
	int r = 0;

	for (size_t i = 0; i < len; i++) {
		unsigned char v = base64_char_value(src[i]);

		if (v == 0xc0) {
			if (decode_strict) {
				*dlen = j;
				return ERROR_INVALID_CHAR;
			}
			continue;
		}

		if (v == 0x40) {
			break;
		}

		w <<= 6;
                w |= v;
                r++;

		if (r == 4) {
			dst[j++] = (unsigned char) (w >> 16);  // w = * aaaaaa bb
			dst[j++] = (unsigned char) (w >> 8);   //                bbbb cccc
			dst[j++] = (unsigned char) w;          //                         cc dddddd
			r = 0;
		}
	}

	switch (r) {
		case 2:
			dst[j++] = (unsigned char) (w >> 4);   // w = * aaaaaa bb ....
			break;
		case 3:
			dst[j++] = (unsigned char) (w >> 10);  // w = * aaaaaa bb
			dst[j++] = (unsigned char) (w >> 2);   //                bbbb cccc ..
			break;
		case 1:
			if (decode_err_mod1) {
				*dlen = j;
				return ERROR_TRAILING_MOD1;
			}
			dst[j++] = (unsigned char) (w << 2);   // w = * aaaaaa
			break;
	}

	*dlen = j;
	return OK;
}

