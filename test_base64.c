#include <stdio.h>
#include <time.h>
#include <string.h>
#include "base64.h"

unsigned char original[64] = "urR4C,$9r3V87z89#t*KEXO@73j$241(3m9A+1T))OlSLD5tg2blpa5JaOTzxaGl";
unsigned char expected[88] = "dXJSNEMsJDlyM1Y4N3o4OSN0KktFWE9ANzNqJDI0MSgzbTlBKzFUKSlPbFNMRDV0ZzJibHBhNUphT1R6eGFHbA==";

#define TSIZE 6
struct test {
	unsigned char *data;
	size_t len;
	int flags;
} tests[TSIZE] = {
	{ (unsigned char *) "ABCDE", 5, 0 },
	{ (unsigned char *) "ABCDE", 5, DECODE_TRAILING1 },
	{ (unsigned char *) "ABC DEF;#$%!", 11, 0 },
	{ (unsigned char *) "ABC DEF=;#$%", 11, 0 },
	{ (unsigned char *) "ABC DEF==;#$", 11, 0 },
	{ (unsigned char *) "ABC DEF===;1", 11, 0 },
};

void test(struct test *tst)
{
	unsigned char decoded[20];

	printf("\n");
	printf("Original: flags=%d\n%.*s\n",tst->flags,(int)tst->len,tst->data);

	size_t dlen;
	size_t slen = tst->len;
	int r = decode_base64(tst->flags,tst->data,&slen,decoded,&dlen);

	printf("Decoded: result=%d slen=%zd\n",r,slen);
	for (size_t i = 0; i < dlen; i++) printf(" %02hhx",decoded[i]);
	printf("  dlen=%zd\n\n",dlen);
}


int main()
{
	unsigned char encoded[88];
	unsigned char decoded[64];
	unsigned char encoded_ext[92];
	size_t slen,dlen;

	encode_base64(original,64,encoded);

	printf("\n");
	printf("Encoded:\n%.*s\n",88,encoded);
	printf("Expected:\n%.*s\n",88,expected);

	slen = 88;
	decode_base64(0,encoded,&slen,decoded,&dlen);

	printf("\n");
	printf("Decoded: %zd\n%.*s\n",dlen,64,decoded);
	printf("Original:\n%.*s\n\n",64,original);

	memcpy(encoded_ext,expected,40);
	memcpy(encoded_ext + 40,"\n$",2);
	memcpy(encoded_ext + 42,expected+40,46);
	memcpy(encoded_ext + 88,"\t.\t.",4);

	slen = 92;
	decode_base64(0,encoded_ext,&slen,decoded,&dlen);

	printf("\n");
	printf("Encoded with extra chars:\n%.*s\n",92,encoded_ext);
	printf("Decoded: %zd\n%.*s\n",dlen,64,decoded);
	printf("Original:\n%.*s\n\n",64,original);

	for (int i = 0; i < TSIZE; i++) {
		test(tests+i);
	}

	return 0;
}
