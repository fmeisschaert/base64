#include <string.h>
#include <stddef.h>
#include <stdint.h>

#ifndef __STDC_VERSION__
#error "at least c17 required"
#elif __STDC_VERSION__ < 201710L
#error "at least c17 required"
#endif

#define BASE64_DEFAULT       0   // IN
#define BASE64_OK            0   // OUT
#define BASE64_STRICT        1   // IN
#define BASE64_INVALID_CHAR  1   // OUT
#define BASE64_TRAILING1     2   // IN/OUT
#define BASE64_PADDING       4   // IN/OUT
#define BASE64_END_OF_INPUT  8   // OUT

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
int encode_base64(int flags, const unsigned char *src, size_t len,unsigned char *dst, size_t *dst_offset)
{
	int ret = BASE64_OK;

	size_t q = len / 3;
	size_t r = len % 3;

	size_t di = dst_offset ? *dst_offset : 0;
	unsigned char *_dst = dst + di;

	for (size_t i = 0; i < q; i++) {
		unsigned char c0 = src[3*i];
		unsigned char c1 = src[3*i+1];
		unsigned char c2 = src[3*i+2];

		unsigned char v0 = c0 >> 2;
		unsigned char v1 = (c0 << 4 & 0x3f) | c1 >> 4;
		unsigned char v2 = (c1 << 2 & 0x3f) | c2 >> 6;
		unsigned char v3 = c2 & 0x3f;

		_dst[4*i]   = base64_value_char(v0);
		_dst[4*i+1] = base64_value_char(v1);
		_dst[4*i+2] = base64_value_char(v2);
		_dst[4*i+3] = base64_value_char(v3);
	}

	di += 4 * q;

	switch (r) {
	case 1: {
			unsigned char c0 = src[3*q];

			unsigned char v0 = c0 >> 2;
			unsigned char v1 = c0 << 4 & 0x3f;

			dst[di++] = base64_value_char(v0);
			dst[di++] = base64_value_char(v1);
			if (flags & BASE64_PADDING) {
				dst[di++] = '=';
				dst[di++] = '=';
			}

			ret |= BASE64_PADDING;
		}
		break;
	case 2: {
			unsigned char c0 = src[3*q];
			unsigned char c1 = src[3*q+1];

			unsigned char v0 = c0 >> 2;
			unsigned char v1 = (c0 << 4 & 0x3f) | c1 >> 4;
			unsigned char v2 = c1 << 2 & 0x3f;

			dst[di++] = base64_value_char(v0);
			dst[di++] = base64_value_char(v1);
			dst[di++] = base64_value_char(v2);
			if (flags & BASE64_PADDING) {
				dst[di++] = '=';
			}

			ret |= BASE64_PADDING;
		}
		break;
	}

	*dst_offset = di;
	return ret;
}

// assume dst + offset has enough space for up to ((len + 3) / 4) * 3 characters
//   offset = *dst_offset
//   len = *src_len_offset
int decode_base64_blocks(
	int flags,
	const unsigned char *src,
	size_t *src_len_offset,  // input: length, output: offset after decoding
	unsigned char *dst,
	size_t *dst_offset,  // input: start offset, output: offset after decoding
	uint32_t *state)
{
	int ret = BASE64_OK;

	int decode_strict = flags & BASE64_STRICT;

	uint32_t w = *state >> 2;
	uint32_t r = *state & 0x3;

	size_t len = *src_len_offset;
	size_t di = *dst_offset;

	size_t si;
	for (si = 0; si < len; si++) {
		unsigned char v = base64_char_value(src[si]);

		if (v == 0xc0) {
			ret |= BASE64_INVALID_CHAR;
			if (decode_strict) break;
			continue;
		}

		if (v == 0x40) {
			ret |= BASE64_PADDING;
			break;
		}

		w <<= 6;
		w |= v;
		r++;


		if (r == 4) {
			dst[di++] = (unsigned char) (w >> 16);  // w = * aaaaaa bb
			dst[di++] = (unsigned char) (w >> 8);   //                bbbb cccc
			dst[di++] = (unsigned char) w;          //                         cc dddddd
			r = 0;
		}
	}

	if (si == len) ret |= BASE64_END_OF_INPUT;

	*state = w << 2 | r;
	*src_len_offset = si;
	*dst_offset = di;
	return ret;
}

// assume dst + offset has enough space for 2 optional characters
int decode_base64_state(
	int flags, uint32_t state,
	unsigned char *dst,
	size_t *dst_offset)   // input: start offset, output: offset after decoding
{
	int ret = BASE64_OK;

	size_t di = *dst_offset;

	uint32_t w = state >> 2;
	uint32_t r = state & 0x3;

	switch (r) {
	case 2:
		dst[di++] = (unsigned char) (w >> 4);   // w = * aaaaaa bb ....
		break;
	case 3:
		dst[di++] = (unsigned char) (w >> 10);  // w = * aaaaaa bb
		dst[di++] = (unsigned char) (w >> 2);   //                bbbb cccc ..
		break;
	case 1:
		if (flags & BASE64_TRAILING1) {
			dst[di++] = (unsigned char) (w << 2);   // w = * aaaaaa 00
		}
		ret |= BASE64_TRAILING1;
		break;
	}

	*dst_offset = di;
	return ret;
}

// assume dst + offset has enough space for up to ((len + 3) / 4) * 3 characters
//   offset = *dst_offset
//   len = *src_len_offset
int decode_base64(
	int flags,
	const unsigned char *src, size_t *src_len_offset,
	unsigned char *dst, size_t *dst_offset)
{
	uint32_t state = 0;
	int ret = decode_base64_blocks(flags,src,src_len_offset,dst,dst_offset,&state);
	if (state & 0x3) ret |= decode_base64_state(flags,state,dst,dst_offset);
	return ret;
}

