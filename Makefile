CC=gcc
CFLAGS=-O0 -ggdb -Wall -fpic
CFLAGS_DEBUG=-O0 -ggdb -Wall -L/home/vykt/programming/libpwu/libpwu -lpwu
CFLAGS_TARGET=-O0 -ggdb -Wall

VPATH=debug libpwu tgt

CLEAN_TARGETS=libpwu/libpwu.so libpwu/process_maps.o libpwu/util.o libpwu/rdwr_mem.o libpwu/puppet.o libpwu/name_pid.o libpwu/pattern.o libpwu/vector.o DEBUG debug/DEBUG.o
CLEAN_TARGETS_TGT=target tgt/target.o

all: libpwu.so DEBUG

libpwu.so: libpwu/libpwu.h libpwu/process_maps.o libpwu/process_maps.h libpwu/util.o libpwu/util.h libpwu/rdwr_mem.o libpwu/rdwr_mem.h libpwu/puppet.o libpwu/puppet.h libpwu/name_pid.o libpwu/name_pid.h libpwu/pattern.o libpwu/pattern.h libpwu/vector.o libpwu/vector.h
	${CC} ${CFLAGS} -shared -o libpwu/libpwu.so libpwu/libpwu.h libpwu/util.{o,h} libpwu/rdwr_mem.{c,h} libpwu/process_maps.{o,h} libpwu/name_pid.{o,h} libpwu/puppet.{o,h} libpwu/pattern.{o,h} libpwu/vector.{o,h}

process_maps.o: libpwu/process_maps.c libpwu/process_maps.h libpwu/vector.h libpwu/libpwu.h
	${CC} ${CFLAGS} -c libpwu/process_maps.{c,h} libpwu/libpwu.h libpwu/vector.h

util.o: libpwu/utils.c libpwu/utils.h libpwu/libpwu.h
	${CC} ${CFLAGS} -c libpwu/utils.{c,h} libpwu/libpwu.h

rdwr_mem.o: libpwu/rdwr_mem.c libpwu/rdwr_mem.h libpwu/libpwu.h
	${CC} ${CFLAGS} -c libpwu/rdwr_mem.{c,h} libpwu/libpwu.h

puppet.o: libpwu/puppet.c libpwu/puppet.h libpwu/libpwu.h
	${CC} ${CFLAGS} -c libpwu/puppet.{c,h} libpwu/libpwu.h

name_pid.o: libpwu/name_pid.c libpwu/name_pid.h libpwu/vector.h libpwu/libpwu.h
	${CC} ${CFLAGS} -c libpwu/name_pid.{c,h} libpwu/vector.h libpwu/libpwu.h

pattern.o: libpwu/pattern.c libpwu/pattern.h libpwu/vector.h libpwu/libpwu.h
	${CC} ${CFLAGS} -c libpwu/pattern.{c,h} libpwu/vector.h libpwu/libpwu.h

vector.o: libpwu/vector.c libpwu/vector.h libpwu/libpwu.h
	${CC} ${CFLAGS} -c libpwu/vector.{c,h} libpwu/libpwu.h

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

clean_target:
	rm ${CLEAN_TARGETS_TGT}
