//TODO make open() & close() perform do the test

//standard library
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

//system headers
#include <unistd.h>

//external libraries
#include <cmore.h>
#include <check.h>

//local headers
#include "suites.h"

//test target headers
#include "../lib/memcry.h"
#include "../lib/iface.h"
#include "../lib/procfs_iface.h"
#include "../lib/krncry_iface.h"



/*
 *  [BASIC TEST]
 *
 *      Virtual interface code is simple; only external functions are tested.
 *
 *      The following functions do not have unit tests:
 *
 *        mc_open() & mc_close():
 *
 *            > Orchestrating the test in a safe manner is too complicated
 *              considering the simplicity of these functions.
 */



//globals
mc_session s;



/*
 *  --- [HELPERS] ---
 */

//interface stub function return codes
#define STUB_OPEN_RET       0x1111
#define STUB_CLOSE_RET      0x2222
#define STUB_UPDATE_MAP_RET 0x3333
#define STUB_READ_RET       0x4444
#define STUB_WRITE_RET      0x5555



//interface stub functions
int stub_open(struct _mc_session * s, int pid) {

    return STUB_OPEN_RET;
}



int stub_close(struct _mc_session * s) {

    return STUB_CLOSE_RET;    
}



int stub_update_map(const struct _mc_session * s, mc_vm_map * m){

    return STUB_UPDATE_MAP_RET;
}



int stub_read(const struct _mc_session * s,
              const uintptr_t off, cm_byte * buf, const size_t sz) {

    return STUB_READ_RET;
}



int stub_write(const struct _mc_session * s,
               const uintptr_t off, const cm_byte * buf, const size_t sz) {

    return STUB_WRITE_RET;
}



/*
 *  --- [FIXTURES] ---
 */

static void _set_stub_session(mc_session * s) {

    s->iface.open       = stub_open;
    s->iface.close      = stub_close;
    s->iface.update_map = stub_update_map;
    s->iface.read       = stub_read;
    s->iface.write      = stub_write;

    return;
 }



/*
 *  --- [UNIT TESTS] ---
 */

//mc_update_map() [stub fixture]
START_TEST(test_mc_update_map) {

    int ret = mc_update_map(&s, 0x0);
    ck_assert_int_eq(ret, STUB_UPDATE_MAP_RET);

    return;
    
} END_TEST



//mc_read() [stub fixture]
START_TEST(test_mc_read) {

    int ret = mc_read(&s, 0x0, 0x0, 0);
    ck_assert_int_eq(ret, STUB_READ_RET);

    return;
    
} END_TEST



//mc_write() [stub fixture]
START_TEST(test_mc_write) {

    int ret = mc_write(&s, 0x0, 0x0, 0);
    ck_assert_int_eq(ret, STUB_WRITE_RET);

    return;
    
} END_TEST
