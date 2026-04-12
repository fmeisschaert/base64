FLAGS = -O2
TFLAGS = -Wno-pointer-sign -Wno-unused-variable -Wno-unused-but-set-variable -Wno-format -Wno-incompatible-pointer-types-discards-qualifiers -Werror

base64.o: base64.c
	cc -std=c17 $(FLAGS) -Wall -Werror -c base64.c

libbase64.so: base64.c
	cc -std=c17 $(FLAGS) -Wall -Werror -fPIC -shared -o libbase64.so base64.c

test_base64.out : test_base64.c base64.c
	cc -std=c17 $(FLAGS) -Wall $(TFLAGS) -o test_base64.out test_base64.c base64.c

test: test_base64.out
	./test_base64.out

clean:
	rm -f *.o *.s *.out
	rm -rf test_base64.dSYM
