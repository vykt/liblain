.RECIPEPREFIX:=>

# This makefile takes the following variables:
#
#	CC        - Compiler.
#   BUILD_DIR - Base build directory.

CFLAGS=-O0 -ggdb -Wno-unused-but-set-variable

INCLUDE=-L${BUILD_DIR}/lib -Wl,-rpath=${BUILD_DIR}/lib -lcmore -llain

SOURCES_TEST=iface.c map.c main.c util.c
HEADERS_TEST=test.h
OBJECTS_TEST=${SOURCES_TEST:.c=.o}

TEST=run_tests

test:${TEST}
> echo ${BUILD_DIR}/lib
> mkdir -p ${BUILD_DIR}/test
> mv ${TEST} ${BUILD_DIR}/test

${TEST}: ${OBJECTS_TEST}
> ${CC} ${CFLAGS} ${OBJECTS_TEST} ${HEADERS_TEST} \
  -o ${TEST} ${INCLUDE}

${OBJECTS_LIB}: ${SOURCES_LIB} ${HEADERS_LIB}
> ${CC} ${CFLAGS} -c ${SOURCES_TEST} -o ${OBJECTS_TEST} ${INCLUDE}

clean_all: clean_src clean_build

clean_src:
> -rm -f *.o

clean_build:
> -rm ${BUILD_DIR}/test/${TEST}
