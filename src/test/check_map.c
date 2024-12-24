//standard library
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//system headers
#include <time.h>
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
 *  Functions are not tested in the same order as they appear in the map.c
 *  source file. This assists with bootstrapping many tests. The order of
 *  testing remains close.
 */


/*
 *  -- [UNTESTED FUNCTIONS] ---
 *
 * _map_obj_add_area_insert():
 *
 *     Tested through _map_obj_add_area() and _map_obj_add_last_area()
 *
 * _map_obj_find_area_outer_node():
 *
 *     Tested through _map_obj_rmv_area() and _map_obj_rmv_last_area();
 *
 */


//globals - map

/*
 *  Contains all structures inside a map, artificially allocated statically.
 */

#define STUB_MAP_AREA_NUM 10
#define STUB_MAP_OBJ_NUM 4
static mc_vm_map m;

static mc_vm_area m_a[STUB_MAP_AREA_NUM];
static cm_lst_node m_a_n[STUB_MAP_AREA_NUM];

static mc_vm_obj m_o[STUB_MAP_OBJ_NUM];
static cm_lst_node m_o_n[STUB_MAP_OBJ_NUM];


//globals - standalone object
#define AREAS_NUM 4
#define LAST_AREAS_NUM 2
static mc_vm_obj o;
static cm_lst_node o_n;

static mc_vm_area o_a[AREAS_NUM];
static cm_lst_node o_a_n[AREAS_NUM];

static mc_vm_area o_a_l[LAST_AREAS_NUM];
static cm_lst_node o_a_l_n[LAST_AREAS_NUM];



/*
 *  --- [HELPERS] ---
 */

//initialise a vm_entry stub
static void _init_vm_entry(struct vm_entry * entry, unsigned long vm_start, 
                           unsigned long vm_end, unsigned long file_off, 
                           krncry_pgprot_t prot, char * file_path) {

    //set entry vars
    entry->vm_start = vm_start;
    entry->vm_end   = vm_end;
    entry->file_off = file_off;
    entry->prot     = prot;

    //set file path
    if (file_path != NULL) {
        strncpy(entry->file_path, file_path, PATH_MAX);
    
    } else {
        entry->file_path[0] = '\0';
    }
    
    return;
}



//initialise a _traverse_state
static void _init__traverse_state(_traverse_state * state, 
                                  cm_lst_node * next_area_node,
                                  cm_lst_node * prev_obj_node) {

    state->next_area_node = next_area_node;
    state->prev_obj_node = prev_obj_node;

    return;
}



//initialise a cm_lst_node stub wrapper
static void _create_lst_wrapper(cm_lst_node * node, void * data) {

    node->data = data;
    node->next = node->prev = NULL;    

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



static void _assert_vm_map(mc_vm_map * map, int vm_areas_len, int vm_objs_len,
                           int vm_areas_unmapped_len, int vm_objs_unmapped_len,
                           int next_id_area, int next_id_obj) {

    //check mapped lists
    _assert_lst_len(&map->vm_areas, vm_areas_len);
    _assert_lst_len(&map->vm_objs, vm_objs_len);

    //check unmapped lists
    _assert_lst_len(&map->vm_areas_unmapped, vm_areas_unmapped_len);
    _assert_lst_len(&map->vm_objs_unmapped, vm_objs_unmapped_len);

    //check next IDs
    ck_assert_int_eq(map->next_id_area, next_id_area);
    ck_assert_int_eq(map->next_id_obj, next_id_obj);

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

    //check the object ID is correctly set
    ck_assert_int_eq(obj->id, id);

    //check the object is set as mapped
    ck_assert(obj->mapped == mapped);

    return;
}



static void _assert_vm_obj_list(cm_lst * outer_node_lst,
                                uintptr_t * start_addrs, int start_addrs_len) {

    mc_vm_area * area;
    cm_lst_node * area_node, * iter_node;


    //setup iteration
    iter_node = outer_node_lst->head;

    //if provided lst is empty, return
    if (outer_node_lst->len == 0 && outer_node_lst->head == NULL) return;

    //otherwise iterate over area starting addresses         
    for (int i = 0; i < start_addrs_len; ++i) {

        //check starting address
        area_node = MC_GET_NODE_PTR(iter_node);
        area = MC_GET_NODE_AREA(area_node);
        ck_assert_int_eq(area->start_addr, start_addrs[i]);
    
        //advance iteration
        iter_node = iter_node->next;
    }
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

    //check the area ID is correctly set
    ck_assert_int_eq(area->id, id);

    //check the area is mapped
    ck_assert(area->mapped == mapped);

    return;
}



/*
 *  --- [FIXTURES] ---
 */

//empty map fixture
static void _setup_empty_vm_map() {

    //construct the map
    mc_new_vm_map(&m);

    return;
}



#define STUB_MAP_LEN 10
//stub map fixture
static void _stub_vm_map() {

    /*
     *  Stub map:
     *
     *  0) 0x1000 - 0x2000 /bin/cat  r--
     *  1) 0x2000 - 0x3000 /bin/cat  rw-
     *  2) 0x3000 - 0x4000 /bin/cat  r-x
     *  3) 0x4000 - 0x5000 [heap]    rw- <- gap
     *  4) 0x6000 - 0x7000           rw- <- gap
     *  5) 0x8000 - 0x9000 /lib/foo  rw-
     *  6) 0x9000 - 0xA000 /lib/foo  rw-
     *  7) 0xA000 - 0xB000 /lib/foo  r-x <- gap
     *  8) 0xC000 - 0xD000           rw- <- gap
     *  9) 0xE000 - 0xF000 [stack]   rw-
     */

    int ret;
    struct vm_entry entry;


    //construct the map
    mc_new_vm_map(&m);

    //construct objects
    _map_new_vm_obj(&m_o[0], &m, "/bin/cat");
    _map_new_vm_obj(&m_o[1], &m, "[heap]");
    _map_new_vm_obj(&m_o[2], &m, "/lib/foo");
    _map_new_vm_obj(&m_o[3], &m, "[stack]");

    //construct object nodes
    for (int i = 0; i < STUB_MAP_OBJ_NUM; ++i) {

        _create_lst_wrapper(&m_o_n[i], &m_o[i]);
    }


    //construct areas
    _init_vm_entry(&entry, 0x1000, 0x2000, 0x100, 
                   MC_ACCESS_READ, "/bin/cat");
    _map_init_vm_area(&m_a[0], &m, &m_o_n[0], NULL, &entry);

    _init_vm_entry(&entry, 0x2000, 0x3000, 0x200, 
                   MC_ACCESS_READ | MC_ACCESS_WRITE, "/bin/cat");
    _map_init_vm_area(&m_a[1], &m, &m_o_n[0], NULL, &entry);

    _init_vm_entry(&entry, 0x3000, 0x4000, 0x300, 
                   MC_ACCESS_READ | MC_ACCESS_EXEC, "/bin/cat");
    _map_init_vm_area(&m_a[2], &m, &m_o_n[0], NULL, &entry);

    _init_vm_entry(&entry, 0x4000, 0x5000, 0x0, 
                   MC_ACCESS_READ | MC_ACCESS_WRITE, "[heap]");
    _map_init_vm_area(&m_a[3], &m, &m_o_n[1], NULL, &entry);

    _init_vm_entry(&entry, 0x6000, 0x7000, 0x0, 
                   MC_ACCESS_READ | MC_ACCESS_WRITE, NULL);
    _map_init_vm_area(&m_a[4], &m, NULL, &m_o_n[1], &entry);

    _init_vm_entry(&entry, 0x8000, 0x9000, 0x100, 
                   MC_ACCESS_READ, "/lib/foo");
    _map_init_vm_area(&m_a[5], &m, &m_o_n[2], NULL, &entry);

    _init_vm_entry(&entry, 0x9000, 0xA000, 0x200, 
                   MC_ACCESS_READ | MC_ACCESS_WRITE, "/lib/foo");
    _map_init_vm_area(&m_a[6], &m, &m_o_n[2], NULL, &entry);

    _init_vm_entry(&entry, 0xA000, 0xB000, 0x300, 
                   MC_ACCESS_READ | MC_ACCESS_EXEC, "/lib/foo");
    _map_init_vm_area(&m_a[7], &m, &m_o_n[2], NULL, &entry);

    _init_vm_entry(&entry, 0xC000, 0xD000, 0x0, 
                   MC_ACCESS_READ | MC_ACCESS_WRITE, NULL);
    _map_init_vm_area(&m_a[8], &m, NULL, &m_o_n[2], &entry);

    _init_vm_entry(&entry, 0xE000, 0xF000, 0x0, 
                   MC_ACCESS_READ | MC_ACCESS_WRITE, "[stack]");
    _map_init_vm_area(&m_a[9], &m, &m_o_n[3], NULL, &entry);
    
    //construct area nodes
    for (int i = 0; i < STUB_MAP_AREA_NUM; ++i) {

        _create_lst_wrapper(&m_a_n[i], &m_a[i]);
    }


    //connect areas to objects
    ret = _map_obj_add_area(&m_o[0], &m_a_n[0]);
    ck_assert_int_eq(ret, 0);

    ret = _map_obj_add_area(&m_o[0], &m_a_n[1]);
    ck_assert_int_eq(ret, 0);
    
    ret = _map_obj_add_area(&m_o[0], &m_a_n[2]);
    ck_assert_int_eq(ret, 0);

    ret = _map_obj_add_area(&m_o[1], &m_a_n[3]);
    ck_assert_int_eq(ret, 0);

    ret = _map_obj_add_last_area(&m_o[1], &m_a_n[4]);
    ck_assert_int_eq(ret, 0);

    ret = _map_obj_add_area(&m_o[2], &m_a_n[5]);
    ck_assert_int_eq(ret, 0);

    ret = _map_obj_add_area(&m_o[2], &m_a_n[6]);
    ck_assert_int_eq(ret, 0);

    ret = _map_obj_add_area(&m_o[2], &m_a_n[7]);
    ck_assert_int_eq(ret, 0);

    ret = _map_obj_add_last_area(&m_o[2], &m_a_n[8]);
    ck_assert_int_eq(ret, 0);

    ret = _map_obj_add_area(&m_o[3], &m_a_n[9]);
    ck_assert_int_eq(ret, 0);

    return;
}



static void _teardown_vm_map() {

    mc_del_vm_map(&m);

    return;
}



//empty object fixture
static void _setup_empty_vm_obj() {

    //super of _setup_empty_vm_map
    _setup_empty_vm_map();

    //construct the new object
    _map_new_vm_obj(&o, &m, "/foo/bar");

    //populate the object node
    _create_lst_wrapper(&o_n, &o);

    return;
}



//stub object fixture
static void _setup_stub_vm_obj() {

    /*
     *  Object will have an address range of 0x1000 - 0x5000.
     */

    struct vm_entry entry;
    uintptr_t addr     = 0x1000;
    uintptr_t last_addr = 0x5000;
    uintptr_t file_off = 0x200;

    //super of _setup_empty_vm_obj
    _setup_empty_vm_obj();
    

    //initialise areas
    for (int i = 0; i < AREAS_NUM; ++i) {

        //setup area
        _init_vm_entry(&entry, addr, addr + 0x1000, 
                       file_off, MC_ACCESS_READ, "/foo/bar");
        _map_init_vm_area(&o_a[i], &m, &o_n, NULL, &entry);
        _map_obj_add_area(&o, &o_a_n[i]);

        //advance iteration
        addr += 0x1000;
        file_off += 0x200;
    }


    //initialise last areas
    for (int i = 0; i < LAST_AREAS_NUM; ++i) {

        //setup last area
        _init_vm_entry(&entry, last_addr, last_addr + 0x1000,
                       0x0, MC_ACCESS_READ, NULL);
        _map_init_vm_area(&o_a_l[i], &m, NULL, &o_n, &entry);
        _map_obj_add_last_area(&o, &o_a_l_n[i]);
    
        //advance iteration
        last_addr += 0x1000;
    }

    return;
}



static void _teardown_vm_obj() {

    //destroy the object
    _map_del_vm_obj(&o);

    //super of _teardown_vm_map()
    _teardown_vm_map();

    return;
}



/*
 *  --- [UNIT TESTS] ---
 */

//mc_new_vm_map() & mc_del_vm_map() [no fixture]
START_TEST(test_mc_new_del_vm_map) {

    mc_vm_obj * zero_obj;


    //construct the map
    mc_new_vm_map(&m);

    //assert state
    _assert_vm_map(&m, 0, 1, 0, 0, 0, 0);

    //check the zero is present object
    zero_obj = MC_GET_NODE_OBJ(m.vm_objs.head);
    _assert_vm_obj(zero_obj, "0x0", "0x0", 0x0, 0x0, 0, 0, ZERO_OBJ_ID, true);

    //delete the map
    mc_del_vm_map(&m);

    return;
    
} END_TEST



//_map_new_vm_obj() & _map_del_vm_obj() [empty map fixture]
START_TEST(test__map_new_del_vm_obj) {

    mc_vm_obj obj;

    //construct the object
    _map_new_vm_obj(&obj, &m, "/foo/bar");

    //assert state    
    _assert_vm_obj(&obj, "/foo/bar", "bar", 0x0, 0x0, 0, 0, 0, true);
    _assert_vm_map(&m, 0, 1, 0, 0, 0, 1);

    return;
}



//_map_make_zero_obj() [empty map fixture]
START_TEST(test__map_make_zero_obj) {

    mc_vm_obj zero_obj;


    //create new object
    _map_new_vm_obj(&zero_obj, &m, "0x0");

    //convert new object to pseudo object 
    _map_make_zero_obj(&zero_obj);

    //assert state
    _assert_vm_obj(&zero_obj, "0x0", "0x0", 0x0, 0x0, 0, 0, ZERO_OBJ_ID, true);
    _assert_vm_map(&m, 0, 1, 0, 0, 0, 0);

    //destroy pseudo object
    _map_del_vm_obj(&zero_obj);

    return;
}



//_map_init_vm_area() [empty object fixture]
START_TEST(test__map_init_vm_area) {

    mc_vm_area area;
    struct vm_entry entry;


    //create a stub entry & initialise the new area
    _init_vm_entry(&entry, 0x1000, 0x2000, 0x800, 
                   MC_ACCESS_READ, "/foo/bar");
    _map_init_vm_area(&area, &m, &o_n, NULL, &entry);

    //assert state
    _assert_vm_area(&area, "/foo/bar", "bar", 0x1000, 0x2000, 
                    MC_ACCESS_READ, &o_n, NULL, 0, true);
    _assert_vm_map(&m, 0, 1, 0, 0, 1, 0);


    //create a stub entry & initialise another new area
    _init_vm_entry(&entry, 0x2000, 0x4000, 0x800, 
                   MC_ACCESS_READ | MC_ACCESS_WRITE, "/purr/meow");
    _map_init_vm_area(&area, &m, NULL, &o_n, &entry);

    //assert state
    _assert_vm_area(&area, NULL, NULL, 0x2000, 0x4000, 
                    MC_ACCESS_READ | MC_ACCESS_WRITE, NULL, &o_n, 1, true);
    _assert_vm_map(&m, 0, 1, 0, 0, 2, 0);

    return;
 
} END_TEST



//_map_obj_add_area() [empty object fixture]
START_TEST(test__map_obj_add_area) {

    int ret;

    mc_vm_area area[4];
    cm_lst_node area_node[4], * area_node_ptr;    
    struct vm_entry entry;

    uintptr_t state_first[1] = {0x2000};
    uintptr_t state_lower[2] = {0x1000, 0x2000};
    uintptr_t state_higher[3] = {0x1000, 0x2000, 0x4000};
    uintptr_t state_middle[4] = {0x1000, 0x2000, 0x3000, 0x4000};


    //initialise first area
    _init_vm_entry(&entry, 0x2000, 0x3000, 0x800, MC_ACCESS_READ, "/foo/bar");
    _map_init_vm_area(&area[0], &m, &o_n, NULL, &entry);
    _create_lst_wrapper(&area_node[0], &area[0]);

    //add first area to the backing object
    _map_obj_add_area(&o, &area_node[0]);

    //assert state
    _assert_vm_obj(&o, "/foo/bar", "bar", 0x2000, 0x3000, 1, 0, 0, true);
    _assert_vm_obj_list(&o.vm_area_node_ps, state_first, 1);
    area_node_ptr = MC_GET_NODE_PTR(o.vm_area_node_ps.head);
    _assert_vm_area(MC_GET_NODE_AREA(area_node_ptr), "/foo/bar", "bar", 
                    0x2000, 0x3000, MC_ACCESS_READ, &o_n, NULL, 0, true);


    //initialise lower area
    _init_vm_entry(&entry, 0x1000, 0x2000, 0x600, MC_ACCESS_WRITE, "/foo/bar");
    _map_init_vm_area(&area[1], &m, &o_n, NULL, &entry);
    _create_lst_wrapper(&area_node[1], &area);

    //add lower area to the backing object
    _map_obj_add_area(&o, &area_node[1]);

    //assert state
    _assert_vm_obj(&o, "/foo/bar", "bar", 0x1000, 0x3000, 2, 0, 0, true);
    _assert_vm_obj_list(&o.vm_area_node_ps, state_lower, 2);
    area_node_ptr = MC_GET_NODE_PTR(o.vm_area_node_ps.head);
    _assert_vm_area(MC_GET_NODE_AREA(area_node_ptr), "/foo/bar", "bar", 
                    0x1000, 0x2000, MC_ACCESS_READ, &o_n, NULL, 1, true);
    
    
    //initialise higher area
    _init_vm_entry(&entry, 0x4000, 0x5000, 0x900, MC_ACCESS_EXEC, "/foo/bar");
    _map_init_vm_area(&area[2], &m, &o_n, NULL, &entry);
    _create_lst_wrapper(&area_node[2], &area);

    //add lower area to the backing object
    _map_obj_add_area(&o, &area_node[2]);

    //assert state
    _assert_vm_obj(&o, "/foo/bar", "bar", 0x1000, 0x5000, 3, 0, 0, true);
    _assert_vm_obj_list(&o.vm_area_node_ps, state_higher, 3);
    area_node_ptr = MC_GET_NODE_PTR(o.vm_area_node_ps.head->prev);
    _assert_vm_area(MC_GET_NODE_AREA(area_node_ptr), "/foo/bar", "bar", 
                    0x4000, 0x5000, MC_ACCESS_EXEC, &o_n, NULL, 2, true);


    //initialise middle area
    _init_vm_entry(&entry, 0x3000, 0x4000, 0x880, MC_ACCESS_READ, "/foo/bar");
    _map_init_vm_area(&area[3], &m, &o_n, NULL, &entry);
    _create_lst_wrapper(&area_node[3], &area);

    //add middle area to the backing object
    _map_obj_add_area(&o, &area_node[3]);

    //assert state
    _assert_vm_obj(&o, "/foo/bar", "bar", 0x1000, 0x5000, 4, 0, 0, true);
    _assert_vm_obj_list(&o.vm_area_node_ps, state_middle, 4);
    area_node_ptr = MC_GET_NODE_PTR(o.vm_area_node_ps.head->next->next);
    _assert_vm_area(MC_GET_NODE_AREA(area_node_ptr), "/foo/bar", "bar", 
                    0x3000, 0x4000, MC_ACCESS_EXEC, &o_n, NULL, 3, true);

    return;

} END_TEST



//_map_obj_add_last_area() [empty object fixture]
START_TEST(test__map_obj_add_last_area) {

    int ret;
    
    mc_vm_area last_area[4];
    cm_lst_node last_area_node[4], * last_area_node_ptr;
    struct vm_entry entry;

    uintptr_t state_first[1] = {0x2000};
    uintptr_t state_lower[2] = {0x1000, 0x2000};
    uintptr_t state_higher[3] = {0x1000, 0x2000, 0x4000};
    uintptr_t state_middle[4] = {0x1000, 0x2000, 0x3000, 0x4000};


    //initialise first area
    _init_vm_entry(&entry, 0x2000, 0x3000, 0x800, MC_ACCESS_READ, "anonmap");
    _map_init_vm_area(&last_area[0], &m, &o_n, NULL, &entry);
    _create_lst_wrapper(&last_area_node[0], &last_area[0]);

    //add first area to the backing object
    _map_obj_add_area(&o, &last_area_node[0]);

    //assert state
    _assert_vm_obj(&o, "/foo/bar", "bar", 0x0, 0x0, 0, 1, 0, true);
    _assert_vm_obj_list(&o.last_vm_area_node_ps, state_first, 1);
    last_area_node_ptr = MC_GET_NODE_PTR(o.vm_area_node_ps.head);
    _assert_vm_area(MC_GET_NODE_AREA(last_area_node_ptr), "anonmap", "anonmap", 
                    0x2000, 0x3000, MC_ACCESS_READ, &o_n, NULL, 0, true);


    //initialise lower area
    _init_vm_entry(&entry, 0x1000, 0x2000, 0x600, MC_ACCESS_WRITE, "/bin/cat");
    _map_init_vm_area(&last_area[1], &m, &o_n, NULL, &entry);
    _create_lst_wrapper(&last_area_node[1], &last_area);

    //add lower area to the backing object
    _map_obj_add_area(&o, &last_area_node[1]);

    //assert state
    _assert_vm_obj(&o, "/foo/bar", "bar", 0x0, 0x0, 0, 2, 0, true);
    _assert_vm_obj_list(&o.last_vm_area_node_ps, state_lower, 2);
    last_area_node_ptr = MC_GET_NODE_PTR(o.vm_area_node_ps.head);
    _assert_vm_area(MC_GET_NODE_AREA(last_area_node_ptr), 
                    "/bin/cat", "cat", 0x1000, 0x2000, MC_ACCESS_READ, 
                    &o_n, NULL, 1, true);
    
    
    //initialise higher area
    _init_vm_entry(&entry, 0x4000, 0x5000, 0x900, MC_ACCESS_EXEC, "/lib/std");
    _map_init_vm_area(&last_area[2], &m, &o_n, NULL, &entry);
    _create_lst_wrapper(&last_area_node[2], &last_area);

    //add lower area to the backing object
    _map_obj_add_area(&o, &last_area_node[2]);

    //assert state
    _assert_vm_obj(&o, "/lib/std", "std", 0x0, 0x0, 0, 3, 0, true);
    _assert_vm_obj_list(&o.last_vm_area_node_ps, state_higher, 3);
    last_area_node_ptr = MC_GET_NODE_PTR(o.vm_area_node_ps.head->prev);
    _assert_vm_area(MC_GET_NODE_AREA(last_area_node_ptr), "/foo/bar", "bar", 
                    0x4000, 0x5000, MC_ACCESS_EXEC, &o_n, NULL, 2, true);


    //initialise middle area
    _init_vm_entry(&entry, 0x3000, 0x4000, 0x880, MC_ACCESS_READ, "io");
    _map_init_vm_area(&last_area[3], &m, &o_n, NULL, &entry);
    _create_lst_wrapper(&last_area_node[3], &last_area);

    //add middle area to the backing object
    _map_obj_add_area(&o, &last_area_node[3]);

    //assert state
    _assert_vm_obj(&o, "/foo/bar", "bar", 0x1000, 0x5000, 4, 0, 0, true);
    _assert_vm_obj_list(&o.last_vm_area_node_ps, state_middle, 4);
    last_area_node_ptr = MC_GET_NODE_PTR(o.vm_area_node_ps.head->next->next);
    _assert_vm_area(MC_GET_NODE_AREA(last_area_node_ptr), "io", "io", 
                    0x3000, 0x4000, MC_ACCESS_EXEC, &o_n, NULL, 3, true);
    
    return;

} END_TEST



//_map_obj_rmv_area() [stub object fixture]
START_TEST(test__map_obj_rmv_area) {
    
    int ret;
    
    uintptr_t state_middle[3] = {0x1000, 0x3000, 0x4000};
    uintptr_t state_first[2] = {0x3000, 0x4000};
    uintptr_t state_last[1] = {0x3000};



    //remove middle area
    ret = _map_obj_rmv_area(&o, &o_a_n[1]);
    ck_assert_int_eq(ret, 0);

    //assert state
    _assert_vm_obj(&o, "/foo/bar", "bar", 0x1000, 0x5000, 3, 2, 0, true);
    _assert_vm_obj_list(&o.vm_area_node_ps, state_middle, 3);
    

    //remove first area
    ret = _map_obj_rmv_area(&o, &o_a_n[0]);
    ck_assert_int_eq(ret, 0);

    //assert state
    _assert_vm_obj(&o, "/foo/bar", "bar", 0x3000, 0x5000, 2, 2, 0, true);
    _assert_vm_obj_list(&o.vm_area_node_ps, state_first, 2);


    //remove last area
    ret = _map_obj_rmv_area(&o, &o_a_n[3]);
    ck_assert_int_eq(ret, 0);

    //assert state
    _assert_vm_obj(&o, "/foo/bar", "bar", 0x3000, 0x4000, 1, 2, 0, true);
    _assert_vm_obj_list(&o.vm_area_node_ps, state_last, 1);


    //remove only remaining area
    ret = _map_obj_rmv_area(&o, &o_a_n[2]);
    ck_assert_int_eq(ret, 0);

    //assert state
    _assert_vm_obj(&o, "/foo/bar", "bar", 0x0, 0x0, 0, 2, 0, true);
    _assert_vm_obj_list(&o.vm_area_node_ps, NULL, 0);

    return;
    
} END_TEST



//_map_obj_rmv_last_area [stub object fixture]
START_TEST(test__map_obj_rmv_last_area) {

    int ret;

    uintptr_t state_first[1] = {0x6000};


    //remove first last area
    ret = _map_obj_rmv_last_area(&o, &o_a_l_n[0]);
    ck_assert_int_eq(ret, 0);

    //assert state
    _assert_vm_obj(&o, "/foo/bar", "bar", 0x1000, 0x5000, 4, 1, 0, true);
    _assert_vm_obj_list(&o.last_vm_area_node_ps, state_first, 1);


    //remove only remaining last area
    ret = _map_obj_rmv_last_area(&o, &o_a_l_n[1]);
    ck_assert_int_eq(ret, 0);

    //assert state
    _assert_vm_obj(&o, "/foo/bar", "bar", 0x1000, 0x5000, 4, 0, 0, true);
    _assert_vm_obj_list(&o.last_vm_area_node_ps, NULL, 0);
    
    return;
}



//_map_is_pathname_in_obj() [empty object fixture]
START_TEST(test__map_is_pathname_in_obj) {

    bool ret;

    //path is in the object
    ret = _map_is_pathname_in_obj("/foo/bar", &o);
    ck_assert(ret);    

    //path is not in the object
    ret = _map_is_pathname_in_obj("anonmap", &o);
    ck_assert(!ret);

    return;

} END_TEST



//_map_find_obj_for_area [empty map fixture]
START_TEST(test__map_find_obj_for_area) {

    int ret;

    mc_vm_obj objs[3];
    cm_lst_node obj_nodes[3];
    char * obj_paths[3] = {"/lib/libc", "/lib/libpthread", "anonmap"};

    struct vm_entry entry;
    _traverse_state state;    


    //construct objects
    for (int i = 0; i < 3; ++i) {
        _map_new_vm_obj(&objs[i], &m, obj_paths[i]);
        _create_lst_wrapper(&obj_nodes[i], &objs[i]);
    }


    //new object, state empty
    _init__traverse_state(&state, NULL, NULL);
    _init_vm_entry(&entry, 0x1000, 0x2000, 0x500, 
                   MC_ACCESS_READ, "/lib/libpthread");

    ret = _map_find_obj_for_area(&state, &entry);
    ck_assert_int_eq(ret, _MAP_OBJ_NEW);

    
    //new object, state full
    _init__traverse_state(&state, NULL, &obj_nodes[1]);
    _init_vm_entry(&entry, 0x1000, 0x2000, 0x500, 
                   MC_ACCESS_READ, "/lib/libpthread");

    ret = _map_find_obj_for_area(&state, &entry);
    ck_assert_int_eq(ret, _MAP_OBJ_NEW);


    //previous object 
    _init__traverse_state(&state, NULL, &obj_nodes[1]);
    _init_vm_entry(&entry, 0x1000, 0x2000, 0x500, 
                   MC_ACCESS_READ, "/lib/libc");

    ret = _map_find_obj_for_area(&state, &entry);
    ck_assert_int_eq(ret, _MAP_OBJ_PREV);


    //next object 
    _init__traverse_state(&state, NULL, &obj_nodes[1]);
    _init_vm_entry(&entry, 0x1000, 0x2000, 0x500, 
                   MC_ACCESS_READ, "anonmap");

    ret = _map_find_obj_for_area(&state, &entry);
    ck_assert_int_eq(ret, _MAP_OBJ_NEXT);


    //destruct objects
    for (int i = 0; i < 3; ++i) {
        _map_del_vm_obj(&objs[i]);
    }

    return;
    
} END_TEST



//_map_backtrack_unmapped_obj_last_vm_areas()
START_TEST(test__map_backtrack_unmapped_obj_last_vm_areas) {

    /*
     *  Stub map:
     *
     *  0) 0x1000 - 0x2000 /bin/cat  r--
     *  1) 0x2000 - 0x3000 /bin/cat  rw-
     *  2) 0x3000 - 0x4000 /bin/cat  r-x
     *  3) 0x4000 - 0x5000 [heap]    rw- <- gap
     *  4) 0x6000 - 0x7000           rw- <- gap
     *  5) 0x8000 - 0x9000 /lib/foo  rw-
     *  6) 0x9000 - 0xA000 /lib/foo  rw-
     *  7) 0xA000 - 0xB000 /lib/foo  r-x <- gap
     *  8) 0xC000 - 0xD000           rw- <- gap
     *  9) 0xE000 - 0xF000 [stack]   rw-
     */

    int ret;

    mc_vm_area * area;
    mc_vm_obj * obj;
    cm_lst_node * area_node, * zero_node;

    uintptr_t first_state[2] = {0x6000, 0xC000};
    uintptr_t second_state[2] = {0x6000, 0xC000};


    //backtrack `/lib/foo`'s last area (index: 8)
    ret = _map_backtrack_unmapped_obj_last_vm_areas(&m_o_n[2]);
    ck_assert_int_eq(ret, 0);

    //assert state

    //check `/lib/foo` no longer has any last areas associated with it
    obj = MC_GET_NODE_OBJ(((cm_lst_node *) &m_o_n[2]));
    _assert_vm_obj(obj, "/lib/foo", "/lib/foo", 0x8000, 0xA000, 3, 0, 3, true);

    //check `[heap]` now has two last areas associated with it
    obj_node = cm_lst_get_n(&m.vm_objs, 2);
    obj = MC_GET_NODE_OBJ(obj_node);
    _assert_vm_obj(obj, "[heap]", "[heap]", 0x4000, 0x5000, 1, 2, 2, true);
    _assert_vm_obj_list(&obj->last_vm_area_node_ps, first_state, 2);

    //check the transfered last area (index: 8) now points to `[heap]`
    area_node = cm_lst_get_n(&m.vm_areas, 8);
    area = MC_GET_NODE_AREA(area_node);
    _assert_vm_area(area, NULL, NULL, 0xC000, 0xD000,
                    MC_ACCESS_READ | MC_ACCESS_WRITE, NULL, obj_node, 8, true);


    //backtrack `[heap]` and then further backtrack `/bin/cat` (index: 8, 4)
    obj_node = cm_lst_get_n(&m.vm_objs, 2);
    ret = _map_backtrack_unmapped_obj_last_vm_areas(obj_node);
    ck_assert_int_eq(ret, 0);

    obj_node = cm_lst_get_n(&m.vm_objs, 1);
    ret = _map_backtrack_unmapped_obj_last_vm_areas(obj_node);
    ck_assert_int_eq(ret, 0);

    //assert state
    
    //check 
    obj_node = cm_lst_get_n(&m.vm_objs, 0);
    obj = MC_GET_NODE_OBJ(obj_node);
    _assert_vm_obj(obj, "0x0", "0x0", 0x0, 0x0, 0, 2, MC_ZERO_OBJ_ID, true);
    _assert_vm_obj_list(&obj->last_vm_area_node_ps, second_state, 2);
    
    area_node = cm_lst_get_n(&m.vm_areas, 8);
    area = MC_GET_NODE_AREA(area_node);
    _assert_vm_area(area, NULL, NULL, 0xC000, 0xD000,
                    MC_ACCESS_READ | MC_ACCESS_WRITE, NULL, obj_node, 8, true);

    area_node = cm_lst_get_n(&m.vm_areas, 4);
    area = MC_GET_NODE_AREA(area_node);
    _assert_vm_area(area, NULL, NULL, 0x6000, 0x7000,
                    MC_ACCESS_READ | MC_ACCESS_WRITE, NULL, obj_node, 4, true);
    

    return;
    
} END_TEST



//_map_forward_unmapped_obj_last_vm_areas()
START_TEST(test__map_forward_unmapped_obj_last_vm_areas) {


    return;
    
} END_TEST
