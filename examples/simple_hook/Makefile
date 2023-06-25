#NOTE: libpwu needs to be installed for this example to work

CC=gcc
ASM=nasm
CFLAGS_EXEC=-O0 -ggdb -Wall
CFLAGS_TARGET=-O0 -ggdb -Wall --static
ASMFLAGS_PAYL=-O0

simple_hook: exec.c
	${CC} ${CFLAGS_EXEC} -o exec exec.c -lpwu -ldl

target: target.c
	${CC} ${CFLAGS_TARGET} -o target target.c

payload: payload.asm
	${ASM} ${ASMFLAGS_PAYL} payload.asm
	mv payload payload.o

clean:
	rm exec
