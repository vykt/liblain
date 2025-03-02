.RECIPEPREFIX:=>

# This makefile takes the following variables:
#
# CC          - Compiler.
# BUILD_DIR   - Unit test build directory.
# LIB_BIN_DIR - Library artifact directory.
#
# _CFLAGS     - Compiler flags.
# _WARN_OPTS  - Compiler warnings.


#[parameters]
CFLAGS=${_CFLAGS} -fsanitize=address -fsanitize-recover=address
WARN_OPTS+=${_WARN_OPTS} -Wno-unused-variable -Wno-unused-but-set-variable
LDFLAGS=-L${LIB_BIN_DIR} \
        -Wl,-rpath=${LIB_BIN_DIR} -lmcry -lcmore -lcheck -lsubunit -static-libasan


#[build constants]
SOURCES_TEST=check_map.c check_procfs_iface.c check_krncry_iface.c \
             check_map_util.c check_util.c iface_helper.c info.c \
			 map_helper.c suites.c target_helper.c main.c
OBJECTS_TEST=${SOURCES_TEST:%.c=${BUILD_DIR}/%.o}
TARGET_DIR=${shell pwd}/target


#[targets]
TESTS=test

tests: tgt ${TESTS}
> mkdir -p ${BUILD_DIR}
> mv ${TESTS} ${BUILD_DIR}

${TESTS}: ${OBJECTS_TEST}
> ${CC} ${CFLAGS} ${WARN_OPTS} -o $@ $^ ${LDFLAGS}

${BUILD_DIR}/%.o: %.c
> ${CC} ${CFLAGS} ${WARN_OPTS} -c $< -o $@

tgt:
> $(MAKE) -C ${TARGET_DIR} targets CC='${CC}' BUILD_DIR='${BUILD_DIR}'

clean:
> ${MAKE} -C ${TARGET_DIR} clean BUILD_DIR='${BUILD_DIR}'
> -rm -v ${BUILD_DIR}/*.o
> -rm -v ${BUILD_DIR}/${TESTS}
