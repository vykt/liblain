//standard library
#include <stdio.h>
#include <stdlib.h>

//system headers
#include <unistd.h>
#include <getopt.h>

//external libraries
#include <cmore.h>
#include <check.h>

//local headers
#include "suites.h"


//manipulated by debug.sh; see `suites.h`
bool _DEBUG_ACTIVE = false;

//tests bitmask
#define MAP_TEST      0x1
#define PROCFS_TEST   0x2
#define KRNCRY_TEST   0x4
#define MAP_UTIL_TEST 0x8
#define UTIL_TEST     0x10


//determine which tests to run
static cm_byte _get_test_mode(int argc, char ** argv) {

    const struct option long_opts[] = {
        {"map", no_argument, NULL, 'm'},
        {"procfs", no_argument, NULL, 'p'},
        {"krncry", no_argument, NULL, 'k'},
        {"map-util", no_argument, NULL, 'n'},
        {"util", no_argument, NULL, 'u'},
        {0,0,0,0}
    };

    int opt;
    cm_byte test_mask = 0;

    
    while((opt = getopt_long(argc, argv, "mpknu", long_opts, NULL)) != -1 
          && opt != 0) {

        //determine parsed argument
        switch (opt) {

            case 'm':
                test_mask |= MAP_TEST;
                break;

            case 'p':
                test_mask |= PROCFS_TEST;
                break;

            case 'k':
                test_mask |= KRNCRY_TEST;
                break;

            case 'n':
                test_mask |= MAP_UTIL_TEST;
                break;

            case 'u':
                test_mask |= UTIL_TEST;
                break;
        }
    }

    return test_mask;
}


//run unit tests
static void _run_unit_tests(cm_byte test_mask) {

    Suite * s_fake;
    Suite * s_map;
    Suite * s_procfs_iface;
    Suite * s_krncry_iface;
    Suite * s_map_util;
    Suite * s_util;

    SRunner * sr;


    //initialise test suites
    s_fake = fake_suite();
    if (test_mask & MAP_TEST) s_map = map_suite();
    if (test_mask & PROCFS_TEST) s_procfs_iface = procfs_iface_suite();
    if (test_mask & KRNCRY_TEST) s_krncry_iface = krncry_iface_suite();
    if (test_mask & MAP_UTIL_TEST) s_map_util = map_util_suite();
    if (test_mask & UTIL_TEST) s_util = util_suite(); 

    //create suite runner
    sr = srunner_create(s_fake);
    if (test_mask & MAP_TEST) srunner_add_suite(sr, s_map);
    if (test_mask & PROCFS_TEST) srunner_add_suite(sr, s_procfs_iface);
    if (test_mask & KRNCRY_TEST) srunner_add_suite(sr, s_krncry_iface);
    if (test_mask & MAP_UTIL_TEST) srunner_add_suite(sr, s_map_util);
    if (test_mask & UTIL_TEST) srunner_add_suite(sr, s_util);

    //run tests
    srunner_run_all(sr, CK_VERBOSE);

    //cleanup
    srunner_free(sr);

    return;
}


//dispatch tests
int main(int argc, char ** argv) {

    cm_byte test_mask = _get_test_mode(argc, argv);
    _run_unit_tests(test_mask);
    
    return 0;
}
