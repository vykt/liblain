#compilers & assemblers
CC=gcc
ASM=nasm

#flags for each component
CFLAGS_LIB=-O0 -ggdb -Wall -fpic
CFLAGS_TEST=-O0 -ggdb -Wall -L/home/vykt/programming/libpwu -lpwu
CFLAGS_TGT=-O0 -ggdb -Wall
ASMFLAGS_PAYL=-O0

#libpwu sources
SOURCES_LIB=libpwu/process_maps.c libpwu/util.c libpwu/rdwr_mem.c libpwu/hook.c libpwu/inject.c libpwu/puppet.c libpwu/name_pid.c libpwu/pattern.c libpwu/vector.c
HEADERS_LIB=libpwu/process_maps.h libpwu/util.h libpwu/rdwr_mem.h libpwu/inject.h libpwu/hook.h libpwu/puppet.h libpwu/name_pid.h libpwu/pattern.h libpwu/vector.h libpwu/libpwu.h
OBJECTS_LIB=${SOURCES_LIB:.c=.o}

#debug sources
SOURCES_TEST=debug/test.c
HEADERS_TEST=libpwu/libpwu.h
OBJECTS_TEST=${SOURCES_TEST:.c=.o}

SOURCES_TGT=debug/target.c
OBJECTS_TGT=${SOURCES_TGT:.c=.o}

SOURCES_PAYL=debug/payload.asm

#built programs / objects
LIBPWU=libpwu.so
TEST=testing
TARGET=target
PAYLOAD=payload.o


#build targets
lib: ${LIBPWU}
test: ${TEST}
tgt: ${TARGET}
payl: ${PAYLOAD}


#libpwu subtargets
${LIBPWU}: ${OBJECTS_LIB}
	${CC} ${CFLAGS_LIB} -shared -o ${LIBPWU} ${OBJECTS_LIB} ${HEADERS_LIB}

libpwu/%.o: libpwu/%.c ${HEADERS_LIB}
	${CC} ${CFLAGS_LIB} -c $< -o $@

#test subtargets
${TEST}: ${OBJECTS_TEST}
	${CC} ${CFLAGS_TEST} ${OBJECTS_TEST} ${HEADERS_TEST} -o ${TEST}

debug/tes%.o: debug/tes%.c ${HEADERS_TEST}
	${CC} ${CFLAGS_TEST} -c $< -o $@

#target subtargets
${TARGET}: ${OBJECTS_TGT}
	${CC} ${CFLAGS_TGT} ${OBJECTS_TGT} --static -o ${TARGET}

debug/tar%.o: debug/tar%.c
	${CC} ${CFLAGS_TEST} -c $< -o $@

#payload subtargets
${PAYLOAD}: debug/payload.asm
	${ASM} ${ASMFLAGS_PAYL} $< -o $@


#clean targets
#repeat in case of error
clean:
	rm ${OBJECTS_LIB} ${LIBPWU} ${OBJECTS_TEST} ${TEST} ${OBJECTS_TGT} ${TARGET} ${PAYLOAD}

clean_lib:
	rm ${OBJECTS_LIB} ${LIBPWU}

clean_test:
	rm ${OBJECTS_TEST} ${TEST}

clean_tgt:
	rm ${OBJECTS_TGT} ${TARGET}

clean_payl:
	rm ${PAYLOAD}
