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
#include "iface_helper.h"

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

    ck_assert_int_ne(se->fd_mem, -1);
    ck_assert_int_eq(se->pid, pid);

    return;
}



/*
 *  --- [UNIT TESTS] ---
 */

//procfs_open() & procfs_close() [no fixture]
START_TEST(test_procfs_mc_open_close) {

    assert_iface_open_close(PROCFS, _assert_session);
    return;
   
} END_TEST



//procfs_update_map() [no fixture]
START_TEST(test_procfs_mc_update_map) {

    assert_iface_update_map(PROCFS);
    return;
    
} END_TEST



//procfs_read() & procfs_write() [no fixture]
START_TEST(test_procfs_mc_read_write) {

    assert_iface_read_write(PROCFS);
    return;
    
} END_TEST



/*
 * --- [SUITE] ---
 */

Suite * procfs_iface_suite() {

    //test cases
    TCase * tc_procfs_mc_open_close;
    TCase * tc_procfs_mc_update_map;
    TCase * tc_procfs_mc_read_write;

    Suite * s = suite_create("procfs_iface");


    //tc_procfs_mc_open_close
    tc_procfs_mc_open_close = tcase_create("procfs_mc_open_close");
    tcase_add_test(tc_procfs_mc_open_close, test_procfs_mc_open_close);

    //tc_procfs_mc_update_map
    tc_procfs_mc_update_map = tcase_create("procfs_mc_update_map");
    tcase_add_test(tc_procfs_mc_update_map, test_procfs_mc_update_map);

    //tc_procfs_mc_read_write
    tc_procfs_mc_read_write = tcase_create("procfs_mc_read_write");
    tcase_add_test(tc_procfs_mc_read_write, test_procfs_mc_read_write);


    //add test cases to procfs interface test suite
    suite_add_tcase(s, tc_procfs_mc_open_close);
    suite_add_tcase(s, tc_procfs_mc_update_map);
    suite_add_tcase(s, tc_procfs_mc_read_write);

    return s;
}
