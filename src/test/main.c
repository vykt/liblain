//standard library
#include <stdio.h>
#include <stdlib.h>

//system headers
#include <unistd.h>
#include <getopt.h>

//external libraries
#include <check.h>

//local headers
#include "suites.h"


enum _test_mode {UNIT, EXPL};

//determine which tests to run
static enum _test_mode _get_test_mode(int argc, char ** argv) {

    const struct option long_opts[] = {
        {"unit-tests", no_argument, NULL, 'u'},
        {"explore", no_argument, NULL, 'e'},
        {0,0,0,0}
    };

    int opt;
    enum _test_mode test_mode = UNIT;

    
    while((opt = getopt_long(argc, argv, "ue", long_opts, NULL)) != -1 
          && opt != 0) {

        //determine parsed argument
        switch (opt) {

            case 'u':
                test_mode = UNIT;
                break;

            case 'e':
                test_mode = EXPL;
                break;
        }
    }

    return test_mode;
}


//run unit tests
static void _run_unit_tests() {

    Suite * s_map;
    //Suite * s_procfs_iface;
    //Suite * s_krncry_iface;
    //Suite * s_map_util;
    //Suite * s_util;

    SRunner * sr;


    //initialise test suites
    s_map = map_suite();
    //s_procfs_iface = procfs_iface_suite();
    //s_krncry_iface = krncry_iface_suite();
    //s_map_util = map_util_suite();
    //s_util = util_suite(); 

    //create suite runner
    sr = srunner_create(s_map);
    //srunner_add_suite(sr, s_procfs_iface);
    //srunner_add_suite(sr, s_krncry_iface);
    //srunner_add_suite(sr, s_map_util);
    //srunner_add_suite(sr, s_util);

    //run tests
    srunner_run_all(sr, CK_VERBOSE);

    //cleanup
    srunner_free(sr);

    return;
}


//dispatch tests
int main(int argc, char ** argv) {

    enum _test_mode mode = _get_test_mode(argc, argv);

    switch (mode) {
        case UNIT:
            _run_unit_tests();
            break;

        case EXPL:
            fprintf(stderr, "[ERR] `-e, --expore` not implemented.\n");
            break;    
    }
    
    return 0;
}
