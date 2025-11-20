CFLAGS = -c -Wall
CC = gcc
LIBS = -lm -lcmocka

all: scanner

scanner: scanner.o reader.o charcode.o token.o error.o
	${CC} scanner.o reader.o charcode.o token.o error.o -o scanner

test: clean_test_objs unit_test_scanner
	./unit_test_scanner

clean_test_objs:
	rm -f unit_test_scanner.o error_test.o reader_test.o charcode_test.o token_test.o

unit_test_scanner: unit_test_scanner.o error_test.o reader_test.o charcode_test.o token_test.o
	${CC} unit_test_scanner.o error_test.o reader_test.o charcode_test.o token_test.o -o unit_test_scanner ${LIBS}

unit_test_scanner.o: unit_test_scanner.c scanner.c
	${CC} ${CFLAGS} -DUNIT_TEST_MODE unit_test_scanner.c

error_test.o: error.c
	${CC} ${CFLAGS} -DUNIT_TEST_MODE error.c -o error_test.o

reader_test.o: reader.c
	${CC} ${CFLAGS} -DUNIT_TEST_MODE reader.c -o reader_test.o

charcode_test.o: charcode.c
	${CC} ${CFLAGS} -DUNIT_TEST_MODE charcode.c -o charcode_test.o

token_test.o: token.c
	${CC} ${CFLAGS} -DUNIT_TEST_MODE token.c -o token_test.o

reader.o: reader.c
	${CC} ${CFLAGS} reader.c

scanner.o: scanner.c
	${CC} ${CFLAGS} scanner.c

charcode.o: charcode.c
	${CC} ${CFLAGS} charcode.c

token.o: token.c
	${CC} ${CFLAGS} token.c

error.o: error.c
	${CC} ${CFLAGS} error.c

clean:
	rm -f *.o *~ scanner unit_test_scanner temp.txt


