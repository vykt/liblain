//external libraries
#include <check.h>



/*
 *  `libcheck` requires that a test suite is constructed with at least
 *  one suite. However, all tests should be optional and be selected
 *  through command-line arguments. To circumvent this limitation, the
 *  test runner is constructed with a fake_suite.
 */

Suite * fake_suite() {

    //create a placeholder suite
    Suite * s = suite_create("fake suite");

    return s;
}
