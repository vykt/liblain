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



/*
 *  [BASIC TEST]
 */



//globals
mc_session s;
pid_t pid;


/*
 *  --- [HELPERS] ---
 */



/*
 *  --- [FIXTURES] ---
 */

//initialise the target
static void _setup_target() {

    int ret;


    pid = start_target();
    ck_assert_int_ne(pid, -1);

    ret = mc_open(&s, PROCFS, pid);
    ck_assert_int_eq(ret, 0);

    return;
}



//teardown the target
static void _teardown_target() {

    int ret;

 
    ret = mc_close(&s);
    ck_assert_int_eq(ret, 0);

    end_target(pid);

    return;
}



/*
 *  --- [UNIT TESTS] ---
 */

//mc_pathname_to_basename() [target fixture]
START_TEST(test_mc_pathname_to_basename) {


} END_TEST



//mc_pid_by_name() [target fixture]
START_TEST(test_mc_pid_by_name) {


} END_TEST



//mc_name_by_pid() [target fixture]
START_TEST(test_mc_name_by_pid) {

   
} END_TEST



//mc_bytes_to_hex() [target fixture]
START_TEST(test_mc_bytes_to_hex) {

    
} END_TEST
