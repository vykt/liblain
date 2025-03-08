.RECIPEPREFIX:=>

# This makefile takes the following variables:
#
# CC          - Compiler.
# BUILD_DIR   - Unit test build directory.


CFLAGS=-O0 -ggdb
WARN_OPTS=-Wno-unused-variable -Wno-unused-but-set-variable
LDFLAGS=-L${LIB_BIN_DIR} -Wl,-rpath=${LIB_BIN_DIR}

SOURCES_UNIT_TARGET=unit_target.c
OBJECTS_UNIT_TARGET=${SOURCES_UNIT_TARGET:%.c=${BUILD_DIR}/%.o}

UNIT_TARGET=unit_target

target: ${UNIT_TARGET}
> mkdir -p ${BUILD_DIR}
> mv ${UNIT_TARGET} ${BUILD_DIR}

${UNIT_TARGET}: ${OBJECTS_UNIT_TARGET}
> ${CC} ${CFLAGS} ${WARN_OPTS} -o $@ $^ ${LDFLAGS}

${BUILD_DIR}/%.o: %.c
> ${CC} ${CFLAGS} ${WARN_OPTS} -c $< -o $@

clean:
> -rm -v ${BUILD_DIR}/${UNIT_TARGET}
