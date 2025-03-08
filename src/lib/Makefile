.RECIPEPREFIX:=>

# This makefile takes the following variables:
#
#	CC         - Compiler.
#   BUILD_DIR  - Library build directory.
#
#	_CFLAGS    - Compiler flags.
#   _WARN_OPTS - Compiler warnings.
#   _LDFLAGS   - Linker flags.


CFLAGS=${_CFLAGS}
WARN_OPTS=${_WARN_OPTS} -Wno-unused-parameter
LDFLAGS=${_LDFLAGS}

SOURCES_LIB=error.c iface.c krncry_iface.c \
            map.c map_util.c procfs_iface.c util.c
OBJECTS_LIB=${SOURCES_LIB:%.c=${BUILD_DIR}/%.o}

SHARED=libmcry.so
STATIC=libmcry.a


shared: ${SHARED}
> mkdir -p ${BUILD_DIR}
> mv ${SHARED} ${BUILD_DIR}

static: ${STATIC}
> mkdir -p ${BUILD_DIR}
> mv ${STATIC} ${BUILD_DIR}

${SHARED}: ${OBJECTS_LIB}
> ${CC} ${CFLAGS} -shared -o $@ $^ ${LDFLAGS}

${STATIC}: ${OBJECTS_LIB}
> ar rcs $@ $^

${BUILD_DIR}/%.o: %.c
> ${CC} ${CFLAGS} ${WARN_OPTS} -c $< -o $@

clean:
> -rm -v ${BUILD_DIR}/*.o
> -rm -v ${BUILD_DIR}/${SHARED}
> -rm -v ${BUILD_DIR}/${STATIC}
