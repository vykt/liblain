#ifndef TEST_H
#define TEST_H

#include <unistd.h>

#include <libcmore.h>

#include "../lib/liblain.h"

//utils
pid_t test_utils(char * comm);

//iface
void test_iface(pid_t pid, int iface, ln_vm_map * vm_map, uintptr_t test_addr);

//map
void test_map(ln_vm_map * vm_map);

//extras
void print_map_area(ln_vm_area * area, char * prefix);
void print_map_obj(ln_vm_obj * obj);
void print_map_areas(ln_vm_map * vm_map);
void print_map_objs(ln_vm_map * vm_map);

#endif
