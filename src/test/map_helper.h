#ifndef MAP_HELPER_H
#define MAP_HELPER_H

//standard library
#include <stdint.h>

//system headers
#include <unistd.h>

#include <linux/limits.h>

//external libraries
#include <cmore.h>

//test target headers
#include "../lib/memcry.h"


//map test structures
struct obj_check {

    char basename[NAME_MAX];
    uintptr_t start_addr;
    uintptr_t end_addr;
};


struct area_check {

    char basename[NAME_MAX];
    uintptr_t start_addr;
    uintptr_t end_addr;
};


//map helper functions
void create_lst_wrapper(cm_lst_node * node, void * data);
void assert_lst_len(cm_lst * list, int len);

void assert_names(char * name_1, char * name_2);
void assert_vm_map(mc_vm_map * map, int vm_areas_len, int vm_objs_len,
                   int vm_areas_unmapped_len, int vm_objs_unmapped_len,
                   int next_id_area, int next_id_obj);

void assert_vm_map_objs(cm_lst * lst, struct obj_check * obj_checks,
                        int start_index, int len, bool mapped);
void assert_vm_map_objs_aslr(cm_lst * lst, char (* basenames)[NAME_MAX],
                             int start_index, int len, bool mapped);

void assert_vm_map_areas(cm_lst * lst, struct area_check * area_checks,
                         int start_index, int len, bool mapped);
void assert_vm_map_areas_aslr(cm_lst * lst, char (* basenames)[NAME_MAX],
                              int start_index, int len, bool mapped);

void assert_vm_obj(mc_vm_obj * obj, char * pathname, char * basename,
                   uintptr_t start_addr, uintptr_t end_addr, int vm_areas_len,
                   int last_vm_areas_len, int id, bool mapped);
void assert_vm_obj_list(cm_lst * outer_node_lst,
                        uintptr_t * start_addrs, int start_addrs_len);

void assert_vm_area(mc_vm_area * area, char * pathname, char * basename,
                    uintptr_t start_addr, uintptr_t end_addr,
                    cm_byte access, cm_lst_node * obj_node_p,
                    cm_lst_node * last_obj_node_p, int id, bool mapped);

#endif
