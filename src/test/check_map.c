//standard library
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//system headers
#include <unistd.h>
#include <linux/limits.h>

//external libraries
#include <cmore.h>
#include <check.h>

//local headers
#include "suites.h"

//test target headers
#include "../lib/memcry.h"
#include "../lib/map.h"



/*
 *  --- [FIXTURES] ---
 */

//globals
static mc_vm_map m;



//empty virtual memory map setup
static void _setup_empty_vm_map() {

    mc_new_vm_map(&m);

    return;
}



static void _teardown() {

    mc_del_vm_map(&m);

    return;
}



/*
 *  --- [HELPERS] ---
 */

//construct a vm_entry stub
static void _init_vm_entry(struct vm_entry * entry, unsigned long vm_start, 
                           unsigned long vm_end, unsigned long file_off, 
                           krncry_pgprot_t prot, char * file_path) {

    entry->vm_start = vm_start;
    entry->vm_end   = vm_end;
    entry->file_off = file_off;
    entry->prot     = prot;
    strncpy(entry->file_path, file_path, PATH_MAX);

    return;
}



//integration test for CMore
static void _assert_lst_len(cm_lst * list, int len) {

    ck_assert_int_eq(list->len, len);

    //if length is zero (0), ensure head is null
    if (len == 0) {
        ck_assert_ptr_null(list->head);
        return;
    }

    //if length is one (1), ensure head is not null
    ck_assert_ptr_nonnull(list->head);
    cm_lst_node * iter = list->head;
    if (len == 1) return;

    //if length is greater than 1 (1), iterate over nodes to ensure length
    ck_assert_ptr_nonnull(iter->next);
    iter = iter->next;

    for (int i = 1; i < len; ++i) {

        ck_assert(iter != list->head);
        iter = iter->next;
    }
    
    return;
}



static void _assert_vm_obj(mc_vm_obj * obj, char * pathname, char * basename,
                           uintptr_t start_addr, uintptr_t end_addr,
                           int vm_areas_len, int last_vm_areas_len,
                           int id, bool mapped) {

    //check names
    ck_assert_str_eq(obj->pathname, pathname);
    ck_assert_str_eq(obj->basename, basename);

    //check addresses
    ck_assert_int_eq(obj->start_addr, start_addr);
    ck_assert_int_eq(obj->end_addr, end_addr);

    //check area node lists are initialised
    _assert_lst_len(&obj->vm_area_node_ps, vm_areas_len);
    _assert_lst_len(&obj->last_vm_area_node_ps, last_vm_areas_len);

    //check the object id is correctly set
    ck_assert_int_eq(obj->id, id);

    //check the object is set as mapped
    ck_assert(obj->mapped == mapped);

    return;
}



static void _assert_vm_area(mc_vm_area * area, char * pathname, char * basename,
                            uintptr_t start_addr, uintptr_t end_addr,
                            cm_byte access, cm_lst_node * obj_node_p,
                            cm_lst_node * last_obj_node_p, 
                            int id, bool mapped) {

    //check names
    ck_assert_str_eq(area->pathname, pathname);
    ck_assert_str_eq(area->basename, basename);

    //check addresses
    ck_assert_int_eq(area->start_addr, start_addr);
    ck_assert_int_eq(area->end_addr, end_addr);

    //check access
    ck_assert(area->access == access);

    //check object pointers
    ck_assert_ptr_eq(area->obj_node_p, obj_node_p);
    ck_assert_ptr_eq(area->last_obj_node_p, last_obj_node_p);

    //check the area id is correctly set
    ck_assert_int_eq(area->id, id);

    //check the area is mapped
    ck_assert(area->mapped == mapped);

    return;
}



/*
 *  --- [UNIT TESTS] ---
 */

//mc_new_vm_map() & mc_del_vm_map() [no fixture]
START_TEST(test_mc_new_del_vm_map) {

    mc_vm_obj * zero_obj;

    mc_new_vm_map(&m);

    //check list lengths
    _assert_lst_len(&m.vm_areas, 0);
    _assert_lst_len(&m.vm_objs, 1);
    _assert_lst_len(&m.vm_areas_unmapped, 0);
    _assert_lst_len(&m.vm_objs_unmapped, 0);

    //check zero object
    zero_obj = MC_GET_NODE_OBJ(m.vm_objs.head);
    _assert_vm_obj(zero_obj, "0x0", "0x0", 0x0, 0x0, 0, 0, ZERO_OBJ_ID, true);

    mc_del_vm_map(&m);

    return;
    
} END_TEST



//_map_new_vm_obj() & _map_del_vm_obj() [empty fixture]
START_TEST(test__map_new_del_vm_obj) {

    mc_vm_obj obj, * obj_p;

    _map_new_vm_obj(&obj, &m, "/foo/bar");

    //check object creation    
    obj_p = MC_GET_NODE_OBJ(m.vm_objs.head);
    _assert_vm_obj(obj_p, "/foo/bar", "bar", 0x0, 0x0, 0, 0, 0, true);

    //check the map's next_id_obj is incremented
    ck_assert_int_eq(m.next_id_obj, 1);

    return;
}



//_map_init_vm_area() [empty fixture]
START_TEST(test__map_init_vm_area) {

    mc_vm_obj obj;
    mc_vm_area area;
    struct vm_entry entry;
    cm_lst_node obj_node;

    //create an object for the area to reference
    _map_new_vm_obj(&obj, &m, "/foo/bar");

    //create a stub list node to hold the object
    obj_node.data = &obj;
    obj_node.next = obj_node.prev = NULL;


    //create a stub entry & initialise a new area
    _init_vm_entry(&entry, 0x1000, 0x2000, 0x800, 
                   MC_ACCESS_READ, "/foo/bar");
    _map_init_vm_area(&area, &m, &obj_node, NULL, &entry);

    //check area is correctly created
    _assert_vm_area(&area, "/foo/bar", "bar", 0x1000, 0x2000, 
                    MC_ACCESS_READ, &obj_node, NULL, 0, true);

    //check map's next id is incremented
    ck_assert_int_eq(m.next_id_area, 1);


    //create a stub entry & initialise another new area
    _init_vm_entry(&entry, 0x2000, 0x4000, 0x800, 
                   MC_ACCESS_READ | MC_ACCESS_WRITE, "/purr/meow");
    _map_init_vm_area(&area, &m, NULL, &obj_node, &entry);

    //check area is correctly created
    _assert_vm_area(&area, NULL, NULL, 0x2000, 0x4000, 
                    MC_ACCESS_READ | MC_ACCESS_WRITE, NULL, &obj_node, 1, true);

    //check map's next id is incremented
    ck_assert_int_eq(m.next_id_area, 1);


    //deallocate the object
    _map_del_vm_obj(&obj);
 
} END_TEST
