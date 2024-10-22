.RECIPEPREFIX:=>

#TODO [set as required] TODO
CC=gcc
CFLAGS=-ggdb -Wall -fPIC
INCLUDE=-lcmore

LIB_DIR="./src/lib"
TEST_DIR="./src/test"
TGT_DIR="./src/tgt"

SRC_DIR=$(shell pwd)/src
BUILD_DIR=$(shell pwd)/build
DOC_DIR=$(shell pwd)/doc

INSTALL_DIR=/usr/local
LD_DIR=/etc/ld.so.conf.d


#[set build options]
ifeq ($(build),debug)
	CFLAGS += -O0
else
	CFLAGS += -O2
endif


#[process targets]
all: lib test tgt

install:
> cp ${BUILD_DIR}/lib/liblain.so ${INSTALL_DIR}/lib
> mkdir -pv ${INSTALL_DIR}/include
> cp ${SRC_DIR}/lib/liblain.h ${INSTALL_DIR}/include
> mkdir -pv ${INSTALL_DIR}/share/man
> cp -R ${DOC_DIR}/roff/* ${INSTALL_DIR}/share/man
> echo "${INSTALL_DIR}/lib" > ${LD_DIR}/90lain.conf
> ldconfig

install_doc:
> mkdir -pv ${INSTALL_DIR}/share/doc/liblain
> cp ${DOC_DIR}/md/* ${INSTALL_DIR}/share/doc/liblain

uninstall:
> rm -vf ${INSTALL_DIR}/lib/liblain.so ${INSTALL_DIR}/include/liblain.h \
	${INSTALL_DIR}/share/man/man3/liblain_* ${INSTALL_DIR}/share/doc/liblain/*
> rmdir ${INSTALL_DIR}/share/doc/liblain
> rm ${LD_DIR}/90lain.conf
> ldconfig

tgt:
> $(MAKE) -C ${TGT_DIR} tgt CC='${CC}' BUILD_DIR='${BUILD_DIR}'

test: lib
> $(MAKE) -C ${TEST_DIR} test CC='${CC}' BUILD_DIR='${BUILD_DIR}'

lib:
> $(MAKE) -C ${LIB_DIR} lib CC='${CC}' CFLAGS='${CFLAGS}' INCLUDE='${INCLUDE}' BUILD_DIR='${BUILD_DIR}'

clean:
> $(MAKE) -C ${TEST_DIR} clean_all CC='${CC}' BUILD_DIR='${BUILD_DIR}'
> $(MAKE) -C ${LIB_DIR} clean_all CC='${CC}' CFLAGS='${CFLAGS}' BUILD_DIR='${BUILD_DIR}'
> ${MAKE} -C ${TGT_DIR} clean_all CC='${CC}' BUILD_DIR='${BUILD_DIR}'
