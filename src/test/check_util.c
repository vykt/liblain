//standard library
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

//system headers
#include <unistd.h>

#include <linux/limits.h>

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
static mc_session s;
static pid_t pid;



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

//mc_pathname_to_basename() [no fixture]
START_TEST(test_mc_pathname_to_basename) {

    char * pathname, * basename;


    //first test: pathname is a path
    pathname = "/foo/bar";
    basename = mc_pathname_to_basename(pathname);

    ck_assert_str_eq(basename, "bar");


    //second test: basename is not a path
    pathname = "baz";
    basename = mc_pathname_to_basename(pathname);

    ck_assert_str_eq(basename, "baz");

    return;

} END_TEST


//mc_pid_by_name() [target fixture]
START_TEST(test_mc_pid_by_name) {

    pid_t pid;
    cm_vct v;


    //first test: target exists
    pid = mc_pid_by_name(TARGET_NAME, &v);

    ck_assert_int_ne(pid, -1);
    ck_assert_int_eq(v.len, 1);

    cm_del_vct(&v);


    //second test: target does not exist
    pid = mc_pid_by_name("foo", &v);

    ck_assert_int_eq(pid, -1);
    ck_assert_int_eq(v.len, 0);

    cm_del_vct(&v);

    return;

} END_TEST


//mc_name_by_pid() [no fixture]
START_TEST(test_mc_name_by_pid) {

    int ret;
    
    pid_t pid;
    char name_buf[NAME_MAX];


    //setup test
    pid = start_target();
    ck_assert_int_ne(pid, -1);


    //first test: target exists
    ret = mc_name_by_pid(pid, name_buf);
    ck_assert_int_eq(ret, 0);
    ck_assert_str_eq(name_buf, TARGET_NAME);


    //second test: target does not exist
    memset(name_buf, 0, NAME_MAX);
    ret = mc_name_by_pid((int) (pow(2, 32)) - 1, name_buf);
    ck_assert_int_eq(ret, -1);
    ck_assert_str_eq(name_buf, "\0");


    //cleanup
    end_target(pid);

    return;
   
} END_TEST


//mc_bytes_to_hex() [no fixture]
START_TEST(test_mc_bytes_to_hex) {

    int ret;


    //only test: convert ascii "foo" to hex representation
    cm_byte str[3]  = "foo";
    char hex_str[6] = {0};

    mc_bytes_to_hex(str, 3, hex_str);
    ret = memcmp(hex_str, "666f6f", 6); 
    ck_assert_int_eq(ret, 0);

    return;
    
} END_TEST



/*
 *  --- [SUITE] ---
 */

Suite * util_suite() {

    //test cases
    TCase * tc_pathname_to_basename;
    TCase * tc_pid_by_name;
    TCase * tc_name_by_pid;
    TCase * tc_bytes_to_hex;
    
    Suite * s = suite_create("util");


    //tc_pathname_to_basename
    tc_pathname_to_basename = tcase_create("pathname_to_basename");
    tcase_add_test(tc_pathname_to_basename, test_mc_pathname_to_basename);

    //tc_pid_by_name
    tc_pid_by_name = tcase_create("pid_by_name");
    tcase_add_checked_fixture(tc_pid_by_name, _setup_target, _teardown_target);
    tcase_add_test(tc_pid_by_name, test_mc_pid_by_name);

    //tc_name_by_pid
    tc_name_by_pid = tcase_create("name_by_pid");
    tcase_add_checked_fixture(tc_name_by_pid, _setup_target, _teardown_target);
    tcase_add_test(tc_name_by_pid, test_mc_name_by_pid);

    //tc_bytes_to_hex
    tc_bytes_to_hex = tcase_create("bytes_to_hex");
    tcase_add_test(tc_bytes_to_hex, test_mc_bytes_to_hex);


    //add test cases to util test suite
    suite_add_tcase(s, tc_pathname_to_basename);
    suite_add_tcase(s, tc_pid_by_name);
    suite_add_tcase(s, tc_name_by_pid);
    suite_add_tcase(s, tc_bytes_to_hex);

    return s;
}
