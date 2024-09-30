.RECIPEPREFIX:=>

# This makefile takes the following variables:
#
#	CC        - Compiler.
#	CFLAGS    - Compiler flags.
#   INCLUDE   - Shared objects to link.
#   BUILD_DIR - Base build directory.

SOURCES_LIB=error.c iface.c lainko_iface.c map.c map_util.c procfs_iface.c util.c
HEADERS_LIB=error.h iface.h lainko.h lainko_iface.h liblain.h map.h map_util.h procfs_iface.h util.h
OBJECTS_LIB=${SOURCES_LIB:.c=.o}

LIB=liblain.so


WARN_OPTS := -Wno-unused-but-set-variable -Wno-stringop-truncation -Wno-maybe-uninitialized

lib: ${LIB}
> mkdir -p ${BUILD_DIR}/lib
> mv ${LIB} ${BUILD_DIR}/lib

${LIB}: ${OBJECTS_LIB}
> ${CC} ${CFLAGS} -shared -o ${LIB} ${OBJECTS_LIB} ${HEADERS_LIB} ${WARN_OPTS} ${INCLUDE}

${OBJECTS_LIB}: ${SOURCES_LIB} ${HEADERS_LIB}
> ${CC} ${CFLAGS} -c ${SOURCES_LIB} ${WARN_OPTS} ${INCLUDE}

clean_all: clean_src clean_build

clean_src:
> -rm -f *.o

clean_build:
> -rm ${BUILD_DIR}/lib/${LIB}
