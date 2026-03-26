#include <stdio.h>
#include <time.h>
#include <string.h>
#include "base64.h"

unsigned char original[64] = "urR4C,$9r3V87z89#t*KEXO@73j$241(3m9A+1T))OlSLD5tg2blpa5JaOTzxaGl";
unsigned char expected[88] = "dXJSNEMsJDlyM1Y4N3o4OSN0KktFWE9ANzNqJDI0MSgzbTlBKzFUKSlPbFNMRDV0ZzJibHBhNUphT1R6eGFHbA==";

int main()
{
	unsigned char encoded[88];
	unsigned char decoded[64];
	unsigned char encoded_ext[92];
	size_t dlen;

	encode_base64(original,64,encoded);

	printf("\n");
	printf("Encoded:\n%.*s\n",88,encoded);
	printf("Expected:\n%.*s\n",88,expected);

	decode_base64(0,encoded,88,decoded,&dlen);

	printf("\n");
	printf("Decoded: %zd\n%.*s\n",dlen,64,decoded);
	printf("Original:\n%.*s\n\n",64,original);

	memcpy(encoded_ext,expected,40);
	memcpy(encoded_ext + 40,"\n$",2);
	memcpy(encoded_ext + 42,expected+40,46);
	memcpy(encoded_ext + 88,"\t.\t.",4);

	decode_base64(0,encoded_ext,92,decoded,&dlen);

	printf("\n");
	printf("Encoded with extra chars:\n%.*s\n",92,encoded_ext);
	printf("Decoded: %zd\n%.*s\n",dlen,64,decoded);
	printf("Original:\n%.*s\n\n",64,original);

	return 0;
}
