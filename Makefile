base64.o: base64.c
	cc -std=c17 -O2 -Wall -Werror -c base64.c

libbase64.so: base64.c
	cc -std=c17 -O2 -Wall -Werror -fPIC -shared -o libbase64.so base64.c

test: test_base64.c base64.c
	cc -std=c17 -O2 -Wall -Werror -o test_base64 test_base64.c base64.c
	./test_base64
