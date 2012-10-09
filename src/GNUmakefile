# Use -O4 to enable LTO
CFLAGS=-O4 -Wall -Werror -Wextra

# Development CFLAGS
#CFLAGS=-g -O0 -Wall -Werror -Wextra

test_encoding: test_encoding.o varint.o
	clang $(CFLAGS) -o test_encoding test_encoding.o varint.o

test_encoding.o:  test_encoding.c
	clang $(CFLAGS) -c -o test_encoding.o test_encoding.c

varint.o: varint.c
	clang $(CFLAGS) -c -o varint.o varint.c

clean::
	rm -f test_encoding *.o
