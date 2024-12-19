//standard library
#include <stdio.h>

//system headers
#include <unistd.h>
#include <getopt.h>

//external libraries
#include <check.h>

//local headers
#include "suites.h"



enum _test_mode {UNIT};

//determine which tests to run
static enum _test_mode _get_test_mode(int argc, char ** argv) {

    const struct option long_opts[] = {
        {"unit-tests", no_argument, NULL, 'u'},
        {0,0,0,0}
    };

    int opt;
    enum _test_mode test_mode = UNIT;

    
    while((opt = getopt_long(argc, argv, "u", long_opts, NULL)) != -1 
          && opt != 0) {

        //determine parsed argument
        switch (opt) {

            case 'u':
                test_mode = UNIT;
                break;
        }
    }

    return test_mode;
}



//run unit tests
static void _run_unit_tests() {

    Suite * s_iface;
    Suite * s_krncry_iface;
    Suite * s_procfs_iface;
    Suite * s_map;
    Suite * s_map_util;
    Suite * util;

    SRunner * sr;

    //initialise test suites
    // TODO s_test1 = test1_suite();
    
    //create suite runner
    //sr = srunner_create(s_test1);
    //srunner_add_suite(sr, s_test2);

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
    }
    
    return 0;
}
