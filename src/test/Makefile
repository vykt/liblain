.RECIPEPREFIX:=>

# This makefile takes the following variables:
#
# CC          - Compiler.
# BUILD_DIR   - Unit test build directory.
# LIB_BIN_DIR - Library artifact directory.
#
# _CFLAGS     - Compiler flags.
# _WARN_OPTS  - Compiler warnings.


CFLAGS=${_CFLAGS} -fsanitize=address
WARN_OPTS+=${_WARN_OPTS} -Wno-unused-variable -Wno-unused-but-set-variable
LDFLAGS=-L${LIB_BIN_DIR} -Wl,-rpath=${LIB_BIN_DIR} -lcmore -lcheck -static-libasan

SOURCES_TEST=main.c
OBJECTS_TEST=${SOURCES_TEST:%.c=${BUILD_DIR}/%.o}

TESTS=test


tests: ${TESTS}
> mkdir -p ${BUILD_DIR}
> mv ${TESTS} ${BUILD_DIR}

${TESTS}: ${OBJECTS_TEST}
> ${CC} ${CFLAGS} ${WARN_OPTS} -o $@ $^ ${LDFLAGS}

${BUILD_DIR}/%.o: %.c
> ${CC} ${CFLAGS} ${WARN_OPTS} -c $< -o $@

clean:
> -rm -v ${BUILD_DIR}/{*.o,${TESTS}}
