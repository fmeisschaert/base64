#ifndef BASE64_H
#define BASE64_H

#include <stddef.h>
#include <stdint.h>

#define BASE64_DEFAULT       0   // IN
#define BASE64_OK            0   // OUT
#define BASE64_STRICT        1   // IN
#define BASE64_INVALID_CHAR  1   // OUT 
#define BASE64_TRAILING1     2   // IN/OUT
#define BASE64_PADDING       4   // IN/OUT
#define BASE64_END_OF_INPUT  8   // OUT

// assume dst has enough space for ((len + 2) / 3) * 4 characters
int encode_base64(int flags, const unsigned char *src, size_t len,unsigned char *dst, size_t *dst_offset);

// assume dst + offset has enough space for up to ((len + 3) / 4) * 3 characters
//   offset = *dst_offset
//   len = *src_len_offset
int decode_base64_blocks(
	int flags,
	const unsigned char *src,
	size_t *src_len_offset,  // input: length, output: offset after decoding
	unsigned char *dst,
	size_t *dst_offset,  // input: start offset, output: offset after decoding
	uint32_t *state);

// assume dst + offset has enough space for 2 optional characters
int decode_base64_state(
	int flags, uint32_t state,
	unsigned char *dst,
	size_t *dst_offset); // input: start offset, output: offset after decoding

// assume dst + offset has enough space for up to ((len + 3) / 4) * 3 characters
//   offset = *dst_offset
//   len = *src_len_offset
int decode_base64(
	int flags,
	const unsigned char *src, size_t *src_len_offset,
	unsigned char *dst, size_t *dst_offset);

#endif // BASE64_H

