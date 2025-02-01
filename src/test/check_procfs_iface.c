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
#include "target_helper.h"

//test target headers
#include "../lib/memcry.h"
#include "../lib/procfs_iface.h"



/*
 *  [BASIC TEST]
 *
 *      Procfs interface code is simple; only external functions are tested.
 */


//globals
pid_t pid;



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
START_TEST(test_mc_open_close) {

    assert_

    return;
   
} END_TEST



//procfs_update_map() [no fixture]
START_TEST(test_mc_update_map) {

    int ret;
    mc_vm_map m;


    //setup tests
    pid = start_target();
    ck_assert_int_ne(pid, -1);

    ret = procfs_open(&s, pid);
    ck_assert_int_eq(ret, 0);

    mc_new_vm_map(&m);


    //first test: update empty map
    ret = procfs_update_map(&s, &m);
    ck_assert_int_eq(ret, 0);

    assert_target_map(pid, &m);
    

    //second test: update filled map (map new areas)
    change_target_map(pid);

    ret = procfs_update_map(&s, &m);
    ck_assert_int_eq(ret, 0);

    assert_target_map(pid, &m);


    //third test: update filled map (unmap old areas)
    change_target_map(pid);

    ret = procfs_update_map(&s, &m);
    ck_assert_int_eq(ret, 0);

    assert_target_map(pid, &m);


    //fourth test: process exited
    end_target(pid);

    ret = procfs_update_map(&s, &m);
    ck_assert_int_eq(ret, -1);


    //cleanup
    ret = procfs_close(&s);
    ck_assert_int_eq(ret, 0);    

    return;
    
} END_TEST



//procfs_read() & procfs_write() [no fixture]
START_TEST(test_mc_read_write) {

    int ret;
    mc_vm_map m;


    //setup tests
    pid = start_target();
    ck_assert_int_ne(pid, -1);

    ret = procfs_open(&s, pid);
    ck_assert_int_eq(ret, 0);

    mc_new_vm_map(&m);






    return;
    
} END_TEST
