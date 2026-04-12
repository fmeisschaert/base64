#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stddef.h>
#include <inttypes.h>
#include "base64.h"

/*
#define BASE64_DEFAULT       0   // IN
#define BASE64_OK            0   // OUT
#define BASE64_STRICT        1   // IN
#define BASE64_INVALID_CHAR  1   // OUT 
#define BASE64_TRAILING1     2   // IN/OUT
#define BASE64_PADDING       4   // IN/OUT
#define BASE64_END_OF_INPUT  8   // OUT
*/

char *ret_str(int r)
{
	static char *s = NULL;
	if (!s) s = malloc(32);

	s[0] = 0;
	s[1] = 0;

	if (r == 0) {
		strcat(s," OK");
		return s + 1;
	}

	if (r & BASE64_INVALID_CHAR) strcat(s," IC");
	if (r & BASE64_TRAILING1)    strcat(s," TR1");
	if (r & BASE64_PADDING)      strcat(s," PAD");
	if (r & BASE64_END_OF_INPUT) strcat(s," EOI");

	return s + 1;
}

char bit(uint32_t st,int n)
{
	return st & 1 << n ? '1' : '0';
}

char *state_str(uint32_t st)
{
	static char *s = NULL;
	if (!s) s = malloc(64);

	char *cp = s;

	unsigned r = st & 0x3;

	switch (r) {
	case 3:
		for (int k = 19; k >= 14; k--) *cp++ = bit(st,k);
		*cp++ = ' ';
	case 2:
		for (int k = 13; k >=  8; k--) *cp++ = bit(st,k);
		*cp++ = ' ';
	case 1:
		for (int k =  7; k >=  2; k--) *cp++ = bit(st,k);
		*cp++ = ' ';
	}
	sprintf(cp,"(%u)",r);

	return s;
}

int idx(char *s, int c)
{
	char *cp = strchr(s,c);
	if (*cp) return cp - s;
	else return strlen(s);
}

const char *original = "eCbnLL$1L3c/kL\303\253~R aQuf3d8_Lh:X,";
const char *expected = "ZUNibkxMJDFMM2Mva0zDq35SIGFRdWYzZDhfTGg6WCw=";

#define ENCLEN(n) (((n+2)/3)*4)
#define DECLEN(n) (((n+3)/4)*3)

int main()
{
	unsigned char *src = NULL, *dst = NULL;
	size_t slo, dlo;  // length / offset
	uint32_t st;
	int r;

	slo = strlen(original);
	dst = realloc(dst,ENCLEN(slo)+1);

	printf("\n### encode with padding ###\n\n");

	dlo = 0;
	r = encode_base64(BASE64_PADDING,original,slo,dst,&dlo);
	dst[dlo] = 0;

	printf("E: %.*s\n",dlo,dst);
	printf("X: %s\n",expected);
	printf("ret = %s\n",ret_str(r));

	printf("\n### encode without padding ###\n\n");

	dlo = 0;
	r = encode_base64(BASE64_DEFAULT,original,slo,dst,&dlo);
	dst[dlo] = 0;

	printf("E: %.*s\n",dlo,dst);
	printf("X: %.*s\n",idx(expected,'='),expected);
	printf("ret = %s\n",ret_str(r));

	printf("\n### decode ###\n\n");

	slo = strlen(expected);
	src = realloc(src,slo+1);
	strcpy(src,original);

	dst = realloc(dst,DECLEN(slo)+4);
	strcpy(dst,"D: ");
	dlo = 3;

	r = decode_base64(BASE64_DEFAULT,expected,&slo,dst,&dlo);

	printf("%.*s\n",dlo,dst);
	printf("O: %s\n",original);
	printf("ret = %s\n",ret_str(r));


	printf("\n### decode in parts ###\n\n");

	strncpy(src,expected,9);
	src[9] = 0;
	slo = strlen(src);
	dlo = 0;
	st = 0;

	printf("B1: '%s'\n",src);
	r = decode_base64_blocks(BASE64_DEFAULT,src,&slo,dst,&dlo,&st);

	printf("after block 1: slo = %zu, dlo = %zu, ret = %s, state = %s\n",
		slo, dlo, ret_str(r), state_str(st));
	printf("D: '%.*s'\n",dlo,dst);

	strcpy(src,expected+9);
	memmove(src+12,src+10,strlen(src+7)+1);
	src[10] = ' ';
	src[11] = ' ';
	slo = strlen(src);

	r = decode_base64_blocks(BASE64_DEFAULT,src,&slo,dst,&dlo,&st);

	printf("\nB2: '%s'\n",src);
	printf("after block 2: slo = %zu, dlo = %zu, ret = %s, state = %s\n",
		slo, dlo, ret_str(r), state_str(st));
	printf("D: '%.*s'\n",dlo,dst);

	printf("\nST\n");
	r = decode_base64_state(BASE64_DEFAULT,st,dst,&dlo);

	printf("after state: slo = %zu, dlo = %zu, ret = %s, state = %s\n",
		slo, dlo, ret_str(r), state_str(st));
	printf("D: '%.*s'\n",dlo,dst);
	printf("O: '%s'\n",original);

	printf("\n");

	return 0;
}
