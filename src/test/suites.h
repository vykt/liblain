#ifndef SUITES_H
#define SUITES_H

//standard library
#include <stdbool.h>

//external libraries
#include <check.h>


//modify behaviour a debugger breaks
extern bool _DEBUG_ACTIVE;


//unit test suites
Suite * krncry_iface_suite();
Suite * procfs_iface_suite();
Suite * map_suite();
Suite * map_util_suite();
Suite * util_suite();

//other tests
//TODO

#endif
