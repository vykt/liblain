.RECIPEPREFIX:=>

# This makefile takes the following variables:
#
#	CC        - Compiler.
#   BUILD_DIR - Base build directory.

CFLAGS=-O0 -ggdb -Wno-unused-but-set-variable -L${BUILD_DIR}/lib -Wl,-rpath=${BUILD_DIR}/lib -lcmore -llain

SOURCES_TGT=main.c
HEADERS_TGT=
OBJECTS_TGT=${SOURCES_TGT:.c=.o}

TGT=target

tgt:${TGT}
> mkdir -p ${BUILD_DIR}/tgt
> mv ${TGT} ${BUILD_DIR}/tgt

${TGT}: ${OBJECTS_TGT}
> ${CC} ${CFLAGS} ${OBJECTS_TGT} ${HEADERS_TGT} \
  -o ${TGT}

${OBJECTS_LIB}: ${SOURCES_LIB} ${HEADERS_LIB}
> ${CC} ${CFLAGS} -c ${SOURCES_TGT} -o ${OBJECTS_TGT}

clean_all: clean_src clean_build

clean_src:
> -rm -f *.o

clean_build:
> -rm ${BUILD_DIR}/tgt/${TGT}
