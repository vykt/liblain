.RECIPEPREFIX:=>


#[set as required]
INSTALL_DIR=/usr/local/lib
INCLUDE_INSTALL_DIR=/usr/local/include
MAN_INSTALL_DIR=/usr/local/share/man
MD_INSTALL_DIR=/usr/local/share/doc/lain
LD_DIR=/etc/ld.so.conf.d

CC=gcc
CFLAGS=
CFLAGS_DBG=-ggdb -O0
WARN_OPTS=-Wall -Wextra
LDFLAGS=


#[build constants]
LIB_DIR="./src/lib"
TEST_DIR="./src/test"
DOC_DIR=./doc
BUILD_DIR=${shell pwd}/build


#[installation constants]
SHARED=liblain.so
STATIC=liblain.a
HEADER=lain.h


#[set build options]
ifeq ($(build),debug)
	CFLAGS     += -O0 -ggdb -fsanitize=address -DDEBUG
	CFLAGS_DBG += -DDEBUG
	LDFLAGS    += -static-libasan
else
	CFLAGS += -O3
endif


#[set static analysis options]
ifeq ($(fanalyzer),true)
	CFLAGS += -fanalyzer
endif


#[process targets]
test: shared
> $(MAKE) -C ${TEST_DIR} tests CC='${CC}' _CFLAGS='${CFLAGS_DBG}' \
		                       _WARN_OPTS='${WARN_OPTS}' \
							   BUILD_DIR='${BUILD_DIR}/test' \
                               LIB_BIN_DIR='${BUILD_DIR}/lib'

all: shared static

shared:
> $(MAKE) -C ${LIB_DIR} shared CC='${CC}' _CFLAGS='${CFLAGS} -fPIC' \
	                           _WARN_OPTS='${WARN_OPTS}' \
							   _LDFLAGS='${LDFLAGS}' \
	                           BUILD_DIR='${BUILD_DIR}/lib'

static:
> $(MAKE) -C ${LIB_DIR} static CC='${CC}' _CFLAGS='${CFLAGS}' \
	                           _WARN_OPTS='${WARN_OPTS}' \
	                           _LDFLAGS='${LDFLAGS}' \
	                           BUILD_DIR='${BUILD_DIR}/lib'

docs:
> $(MAKE) -C ${DOC_DIR} all

clean:
> $(MAKE) -C ${TEST_DIR} clean CC='${CC}' BUILD_DIR='${BUILD_DIR}/test'
> $(MAKE) -C ${LIB_DIR} clean CC='${CC}' BUILD_DIR='${BUILD_DIR}/lib'

install:
> mkdir -pv ${INSTALL_DIR}
> cp -v ${BUILD_DIR}/lib/{${SHARED},${STATIC}} ${INSTALL_DIR}
> mkdir -pv ${INCLUDE_INSTALL_DIR}
> cp -v ${LIB_DIR}/${HEADER} ${INCLUDE_INSTALL_DIR}
> mkdir -pv ${MAN_INSTALL_DIR}
> cp -Rv ${DOC_DIR}/groff/man/* ${MAN_INSTALL_DIR}
> echo "${INSTALL_DIR}" > ${LD_DIR}/90cmore.conf
> ldconfig

install_docs:
> mkdir -pv ${MD_INSTALL_DIR}
> cp -v ${DOC_DIR}/md/* ${MD_INSTALL_DIR}

uninstall:
> -rm -v ${INSTALL_DIR}/{${SHARED},${STATIC}}
> -rm -v ${INCLUDE_INSTALL_DIR}/${HEADER}
> -rm -v ${MAN_INSTALL_DIR}/man7/cmore_*.7
> -rm -v ${MD_INSTALL_DIR}/*.md
> -rmdir ${MD_INSTALL_DIR}
> -rm ${LD_DIR}/90cmore.conf
> ldconfig
