#NOTE: libpwu needs to be installed for this example to work

CC=gcc
ASM=nasm
CFLAGS_UWU=-O0 -ggdb -Wall -lpwu
CFLAGS_TGT=-O0 -ggdb -Wall
ASMFLAGS_PAYL=-O0

simple_hook: uwu.c target.c payload.asm
	${CC} ${CFLAGS_UWU} -o uwu uwu.c
	${CC} ${CFLAGS_TGT} -o target target.c
	${ASM} ${ASMFLAGS_PAYL} payload.asm

clean:
	rm uwu target payload.o
