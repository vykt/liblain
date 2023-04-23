CC=gcc
CFLAGS=-O0 -ggdb -Wall -fpic
CFLAGS_DEBUG=-O0 -ggdb -Wall -L/home/vykt/programming/libpwu/libpwu -lpwu
CFLAGS_TARGET=-O0 -ggdb -Wall

VPATH=debug libpwu tgt

CLEAN_TARGETS=libpwu/libpwu.so libpwu/main.o libpwu/process_maps.o libpwu/patterns.o libpwu/vector.o DEBUG debug/DEBUG.o
CLEAN_TARGETS_TGT=target tgt/target.o

all: libpwu.so DEBUG

libpwu.so: libpwu/libpwu.h libpwu/process_maps.o libpwu/process_maps.h libpwu/patterns.o libpwu/patterns.h libpwu/vector.o libpwu/vector.h
	${CC} ${CFLAGS} -shared -o libpwu/libpwu.so libpwu/libpwu.h libpwu/process_maps.{c,h} libpwu/patterns.{o,h} libpwu/vector.{o,h}

process_maps.o: libpwu/process_maps.c libpwu/process_maps.h libpwu/libpwu.h libpwu/vector.h
	${CC} ${CFLAGS} -c libpwu/process_maps.c libpwu/process_maps.h libpwu/libpwu.h libpwu/vector.h

patterns.o: libpwu/patterns.c libpwu/patterns.h libpwu/vector.h libpwu/libpwu.h
	${CC} ${CFLAGS} -c libpwu/patterns.{c,h} libpwu/vector.h libpwu/libpwu.h

vector.o: libpwu/vector.c libpwu/vector.h libpwu/libpwu.h
	${CC} ${CFLAGS} -c libpwu/vector.c libpwu/vector.h libpwu/libpwu.h

DEBUG: debug/DEBUG.o
	${CC} ${CFLAGS_DEBUG} -o DEBUG debug/DEBUG.o

DEBUG.o: debug/DEBUG.c
	${CC} ${CFLAGS_DEBUG} -c debug/DEBUG.c

target: tgt/target.o
	${CC} ${CFLAGS_TARGET} -o target tgt/target.o

target.o: tgt/target.c
	${CC} ${CFLAGS_TARGET} -c tgt/target.c

clean:
	rm ${CLEAN_TARGETS}

clean_tgt:
	rm ${CLEAN_TARGETS_TGT}
