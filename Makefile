CC=gcc
CFLAGS=-O0 -ggdb -Wall -fpic
CFLAGS_DEBUG=-O0 -ggdb -Wall -L/home/vykt/programming/libpwu/libpwu -lpwu
CFLAGS_TARGET=-O0 -ggdb -Wall

VPATH=debug libpwu tgt

CLEAN_TARGETS=libpwu.so libpwu/libpwu.o
CLEAN_TARGETS_ALL=libpwu/libpwu.so libpwu/main.o libpwu/process_maps.o libpwu/vector.o DEBUG debug/DEBUG.o target tgt/target.o

all: libpwu.so DEBUG target

libpwu.so: libpwu/main.o libpwu/process_maps.o libpwu/process_maps.h libpwu/vector.o libpwu/vector.h
	${CC} ${CFLAGS} -shared -o libpwu/libpwu.so libpwu/main.o libpwu/process_maps.{c,h} libpwu/vector.{o,h}

main.o: libpwu/main.c
	${CC} ${CFLAGS} -c libpwu/main.c

process_maps.o: libpwu/process_maps.c libpwu/process_maps.h libpwu/vector.h
	${CC} ${CFLAGS} -c libpwu/process_maps.c libpwu/process_maps.h libpwu/vector.h

vector.o: libpwu/vector.c libpwu/vector.h
	${CC} ${CFLAGS} -c libpwu/vector.c libpwu/vector.h

DEBUG: debug/DEBUG.o
	${CC} ${CFLAGS_DEBUG} -o DEBUG debug/DEBUG.o

DEBUG.o: debug/DEBUG.c
	${CC} ${CFLAGS_DEBUG} -c debug/DEBUG.c

target: tgt/target.o
	${CC} ${CFLAGS_TARGET} -o target tgt/target.o

target.o: tgt/target.c
	${CC} ${CFLAGS_TARGET} -c tgt/target.c

clean:
	rm ${CLEAN_TARGETS_ALL}
