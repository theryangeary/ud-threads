#Makefile for UD CISC user-level thread library

CC = gcc
CFLAGS = -g

LIBOBJS = t_lib.o

TSTOBJS = test00.o

# specify the executable

EXECS = test00 test01 test01x test01a test02 test02a test03 test04 test05 test06 test07 test08 test09 test10 test11

tests: test00.c test01.c test01x.c test01a.c test02.c test02a.c test03.c test04.c test05.c test06.c test07.c test08.c test09.c test10.c test11.c t_lib.a Makefile
	${CC} ${CFLAGS} test00.c t_lib.a -o test00
	${CC} ${CFLAGS} test01.c t_lib.a -o test01
	${CC} ${CFLAGS} test01x.c t_lib.a -o test01x
	${CC} ${CFLAGS} test01a.c t_lib.a -o test01a
	${CC} ${CFLAGS} test02.c t_lib.a -o test02
	${CC} ${CFLAGS} test02a.c t_lib.a -o test02a
	${CC} ${CFLAGS} test03.c t_lib.a -o test03
	${CC} ${CFLAGS} test04.c t_lib.a -o test04
	${CC} ${CFLAGS} test05.c t_lib.a -o test05
	${CC} ${CFLAGS} test06.c t_lib.a -o test06
	${CC} ${CFLAGS} test07.c t_lib.a -o test07
	${CC} ${CFLAGS} test08.c t_lib.a -o test08
	# ${CC} ${CFLAGS} test09.c t_lib.a -o test09
	# ${CC} ${CFLAGS} test10.c t_lib.a -o test10
	# ${CC} ${CFLAGS} test11.c t_lib.a -o test11
	${CC} ${CFLAGS} philosophers.c t_lib.a -o philosophers
	${CC} ${CFLAGS} 3test.c t_lib.a -o shone
	./comprehensive_test.sh

# specify the source files

LIBSRCS = t_lib.c

TSTSRCS = test00.c

# ar creates the static thread library

t_lib.a: ${LIBOBJS} Makefile
	ar rcs t_lib.a ${LIBOBJS}

# here, we specify how each file should be compiled, what
# files they depend on, etc.

t_lib.o: t_lib.c t_lib.h Makefile
	${CC} ${CFLAGS} -c t_lib.c

make daemon:
	./continuous-make.sh

clean:
	rm -f t_lib.a ${EXECS} ${LIBOBJS} ${TSTOBJS}
