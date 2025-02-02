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
