.RECIPEPREFIX:=>

#TODO [set as required] TODO
CC=gcc
CFLAGS=-ggdb -Wall -fPIC
INCLUDE=-lcmore

LIB_DIR="./src/lib"
TEST_DIR="./src/test"
TGT_DIR="./src/tgt"

BUILD_DIR=$(shell pwd)/build


#[set build options]
ifeq ($(build),debug)
	CFLAGS += -O0
else
	CFLAGS += -O2
endif


#[process targets]
all: lib test tgt

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
