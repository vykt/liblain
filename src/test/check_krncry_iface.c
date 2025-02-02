//standard library
#include <stdlib.h>
#include <stdint.h>

//system headers
#include <unistd.h>

//external libraries
#include <cmore.h>
#include <check.h>

//local headers
#include "suites.h"
#include "iface_helper.c"

//test target headers
#include "../lib/memcry.h"



/*
 *  [BASIC TEST]
 */

/*
 *  NOTE: Unit tests for interfaces are standardised through the iface helper.
 */



/*
 *  --- [HELPERS] ---
 */

static void _assert_session(mc_session * se, pid_t pid) {

    ck_assert_int_ne(se->major, -1);
    ck_assert_int_eq(se->fd_dev_krncry, pid);

    return;
}



/*
 *  --- [UNIT TESTS] ---
 */

//krncry_open() & krncry_close() [no fixture]
START_TEST(test_krncry_mc_open_close) {

    assert_iface_open_close(KRNCRY, _assert_session);
    return;
   
} END_TEST



//krncry_update_map() [no fixture]
START_TEST(test_krncry_mc_update_map) {

    assert_iface_update_map(KRNCRY);
    return;
    
} END_TEST



//krncry_read() & krncry_write() [no fixture]
START_TEST(test_krncry_mc_read_write) {

    assert_iface_read_write(KRNCRY);
    return;
    
} END_TEST



/*
 * --- [SUITE] ---
 */

Suite * krncry_iface_suite() {

    //test cases
    TCase * tc_krncry_mc_open_close;
    TCase * tc_krncry_mc_update_map;
    TCase * tc_krncry_mc_read_write;

    Suite * s = suite_create("krncry_iface");


    //tc_krncry_mc_open_close
    tc_krncry_mc_open_close = tcase_create("krncry_mc_open_close");
    tcase_add_test(tc_krncry_mc_open_close, test_krncry_mc_open_close);

    //tc_krncry_mc_update_map
    tc_krncry_mc_update_map = tcase_create("krncry_mc_update_map");
    tcase_add_test(tc_krncry_mc_update_map, test_krncry_mc_update_map);

    //tc_krncry_mc_read_write
    tc_krncry_mc_read_write = tcase_create("krncry_mc_read_write");
    tcase_add_test(tc_krncry_mc_read_write, test_krncry_mc_read_write);


    //add test cases to krncry interface test suite
    suite_add_tcase(s, tc_krncry_mc_open_close);
    suite_add_tcase(s, tc_krncry_mc_update_map);
    suite_add_tcase(s, tc_krncry_mc_read_write);

    return s;
}
