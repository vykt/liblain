.RECIPEPREFIX:=>

# This makefile takes the following variables:
#
# CC          - Compiler.
# BUILD_DIR   - Unit test build directory.


CFLAGS=-O0 -ggdb
WARN_OPTS+=${_WARN_OPTS} -Wno-unused-variable -Wno-unused-but-set-variable
LDFLAGS=-L${LIB_BIN_DIR} -Wl,-rpath=${LIB_BIN_DIR} -lcmore -lcheck

SOURCES_MANUAL_TARGET=manual_target.c
OBJECTS_MANUAL_TARGET=${SOURCES_MANUAL_TARGET:%.c=${BUILD_DIR}/%.o}

SOURCES_UNIT_TARGET=unit_target.c
OBJECTS_UNIT_TARGET=${SOURCES_UNIT_TARGET:%.c=${BUILD_DIR}/%.o}

MANUAL_TARGET=manual_target
UNIT_TARGET=unit_target

TARGETS=${MANUAL_TARGET} ${UNIT_TARGET}


targets: ${TARGETS}
> mkdir -p ${BUILD_DIR}
> mv ${TARGETS} ${BUILD_DIR}

${MANUAL_TARGET}: ${OBJECTS_MANUAL_TARGET}
> ${CC} ${CFLAGS} ${WARN_OPTS} -o $@ $^ ${LDFLAGS}

${UNIT_TARGET}: ${OBJECTS_UNIT_TARGET}
> ${CC} ${CFLAGS} ${WARN_OPTS} -o $@ $^ ${LDFLAGS}

${BUILD_DIR}/%.o: %.c
> ${CC} ${CFLAGS} ${WARN_OPTS} -c $< -o $@

clean:
> -rm -v ${BUILD_DIR}/${MANUAL_TARGET}
> -rm -v ${BUILD_DIR}/${UNIT_TARGET}
