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
 *  [ADVANCED TEST]
 *
 *      The virtuam memory map management code is complicated and as such,
 *      almost every internal function has independent tests. For these tests 
 *      to run, the debug target must be built.
 *
 *      The following functions do not have unit tests:
 *
 *        _map_obj_add_area_insert():
 *
 *            > Tested through `_map_obj_add_area()` 
 *              and `_map_obj_add_last_area()`
 *
 *        _map_obj_find_area_outer_node():
 *
 *            > Tested through `_map_obj_rmv_area()` 
 *              and `_map_obj_rmv_last_area()`
 *
 *
 */

/*
 *  Functions are not tested in the same order as they appear in the map.c
 *  source file. This assists with bootstrapping many tests. The order of
 *  testing remains close.
 */
 


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



//assert the length of a list, also works as an integration test for CMore
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



//basic assertion of state for a mc_vm_map
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



//assert the state of all [unmapped] objects inside a mc_vm_map
static void _assert_vm_map_objs(cm_lst * obj_lst, 
                                struct obj_check * obj_checks, 
                                int start_index, int len) {

    mc_vm_obj * obj;

    for (int i = 0; i < len; ++i) {

        obj = cm_lst_get_p(obj_lst, start_index + i);
        ck_assert_ptr_nonnull(obj);

        ck_assert_str_eq(obj->basename, obj_checks[i].basename);
        ck_assert_int_eq(obj->start_addr, obj_checks[i].start_addr);
        ck_assert_int_eq(obj->end_addr, obj_checks[i].end_addr);
    }

    return;
}



//assert the state of all [unmapped] memory areas inside a mc_vm_map
static void _assert_vm_map_areas(cm_lst * area_lst, 
                                 struct area_check * area_checks,
                                 int start_index, int len) {

    mc_vm_area * area;

    for (int i = 0; i < len; ++i) {

        area = cm_lst_get_p(area_lst, start_index + i);
        ck_assert_ptr_nonnull(area);

        ck_assert_str_eq(area->basename, area_checks[i].basename);
        ck_assert_int_eq(area->start_addr, area_checks[i].start_addr);
        ck_assert_int_eq(area->end_addr, area_checks[i].end_addr);
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

    //check the object ID is correctly set
    ck_assert_int_eq(obj->id, id);

    //check the object is set as mapped
    ck_assert(obj->mapped == mapped);

    return;
}



/*
 *  Check state of the object by checking the starting addresses of each of
 *  its constituent areas.
 */
 
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


#ifdef DEBUG
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
    _map_init_vm_area(&m_a[0], &entry, &m_o_n[0], NULL, &m);

    _init_vm_entry(&entry, 0x2000, 0x3000, 0x200, 
                   MC_ACCESS_READ | MC_ACCESS_WRITE, "/bin/cat");
    _map_init_vm_area(&m_a[1], &entry, &m_o_n[0], NULL, &m);

    _init_vm_entry(&entry, 0x3000, 0x4000, 0x300, 
                   MC_ACCESS_READ | MC_ACCESS_EXEC, "/bin/cat");
    _map_init_vm_area(&m_a[2], &entry, &m_o_n[0], NULL, &m);

    _init_vm_entry(&entry, 0x4000, 0x5000, 0x0, 
                   MC_ACCESS_READ | MC_ACCESS_WRITE, "[heap]");
    _map_init_vm_area(&m_a[3], &entry, &m_o_n[1], NULL, &m);

    _init_vm_entry(&entry, 0x6000, 0x7000, 0x0, 
                   MC_ACCESS_READ | MC_ACCESS_WRITE, NULL);
    _map_init_vm_area(&m_a[4], &entry, NULL, &m_o_n[1], &m);

    _init_vm_entry(&entry, 0x8000, 0x9000, 0x100, 
                   MC_ACCESS_READ, "/lib/foo");
    _map_init_vm_area(&m_a[5], &entry, &m_o_n[2], NULL, &m);

    _init_vm_entry(&entry, 0x9000, 0xA000, 0x200, 
                   MC_ACCESS_READ | MC_ACCESS_WRITE, "/lib/foo");
    _map_init_vm_area(&m_a[6], &entry, &m_o_n[2], NULL, &m);

    _init_vm_entry(&entry, 0xA000, 0xB000, 0x300, 
                   MC_ACCESS_READ | MC_ACCESS_EXEC, "/lib/foo");
    _map_init_vm_area(&m_a[7], &entry, &m_o_n[2], NULL, &m);

    _init_vm_entry(&entry, 0xC000, 0xD000, 0x0, 
                   MC_ACCESS_READ | MC_ACCESS_WRITE, NULL);
    _map_init_vm_area(&m_a[8], &entry, NULL, &m_o_n[2], &m);

    _init_vm_entry(&entry, 0xE000, 0xF000, 0x0, 
                   MC_ACCESS_READ | MC_ACCESS_WRITE, "[stack]");
    _map_init_vm_area(&m_a[9], &entry, &m_o_n[3], NULL, &m);
    
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
#endif



static void _teardown_vm_map() {

    mc_del_vm_map(&m);

    return;
}


#ifdef DEBUG
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
#endif



#ifdef DEBUG
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
        _map_init_vm_area(&o_a[i], &entry, &o_n, NULL, &m);
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
        _map_init_vm_area(&o_a_l[i], &entry, NULL, &o_n, &m);
        _map_obj_add_last_area(&o, &o_a_l_n[i]);
    
        //advance iteration
        last_addr += 0x1000;
    }

    return;
}
#endif



#ifdef DEBUG
static void _teardown_vm_obj() {

    //destroy the object
    _map_del_vm_obj(&o);

    //super of _teardown_vm_map()
    _teardown_vm_map();

    return;
}
#endif



/*
 *  --- [UNIT TESTS] ---
 */

//mc_new_vm_map() & mc_del_vm_map() [no fixture]
START_TEST(test_mc_new_del_vm_map) {

    mc_vm_obj * zero_obj;


    //only test: construct the map
    mc_new_vm_map(&m);

    _assert_vm_map(&m, 0, 1, 0, 0, 0, 0);

    //check the pseudo object is present
    zero_obj = MC_GET_NODE_OBJ(m.vm_objs.head);
    _assert_vm_obj(zero_obj, "0x0", "0x0", 0x0, 0x0, 
                   0, 0, MC_ZERO_OBJ_ID, true);

    mc_del_vm_map(&m);

    return;
    
} END_TEST



#ifdef DEBUG
//_map_new_vm_obj() & _map_del_vm_obj() [empty map fixture]
START_TEST(test__map_new_del_vm_obj) {

    mc_vm_obj obj;

    //only test: construct the object
    _map_new_vm_obj(&obj, &m, "/foo/bar");

    _assert_vm_obj(&obj, "/foo/bar", "bar", 0x0, 0x0, 0, 0, 0, true);
    _assert_vm_map(&m, 0, 1, 0, 0, 0, 1);

    return;
}



//_map_make_zero_obj() [empty map fixture]
START_TEST(test__map_make_zero_obj) {

    mc_vm_obj zero_obj;


    //create new object
    _map_new_vm_obj(&zero_obj, &m, "0x0");

    //only test: convert new object to pseudo object 
    _map_make_zero_obj(&zero_obj);
    
    _assert_vm_obj(&zero_obj, 
                   "0x0", "0x0", 0x0, 0x0, 0, 0, MC_ZERO_OBJ_ID, true);
    _assert_vm_map(&m, 0, 1, 0, 0, 0, 0);

    //destroy pseudo object
    _map_del_vm_obj(&zero_obj);

    return;
}



//_map_init_vm_area() [empty object fixture]
START_TEST(test__map_init_vm_area) {

    mc_vm_area area;
    struct vm_entry entry;


    //first test: create a stub entry & initialise the new area
    _init_vm_entry(&entry, 0x1000, 0x2000, 0x800, 
                   MC_ACCESS_READ, "/foo/bar");
    _map_init_vm_area(&area, &entry, &o_n, NULL, &m);

    _assert_vm_area(&area, "/foo/bar", "bar", 0x1000, 0x2000, 
                    MC_ACCESS_READ, &o_n, NULL, 0, true);
    _assert_vm_map(&m, 0, 1, 0, 0, 1, 0);


    //second test: create a stub entry & initialise another new area
    _init_vm_entry(&entry, 0x2000, 0x4000, 0x800, 
                   MC_ACCESS_READ | MC_ACCESS_WRITE, "/purr/meow");
    _map_init_vm_area(&area, &entry, NULL, &o_n, &m);

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
    _map_init_vm_area(&area[0], &entry, &o_n, NULL, &m);
    _create_lst_wrapper(&area_node[0], &area[0]);


    //first test: add first area to the backing object
    _map_obj_add_area(&o, &area_node[0]);

    _assert_vm_obj(&o, "/foo/bar", "bar", 0x2000, 0x3000, 1, 0, 0, true);
    _assert_vm_obj_list(&o.vm_area_node_ps, state_first, 1);
    area_node_ptr = MC_GET_NODE_PTR(o.vm_area_node_ps.head);
    _assert_vm_area(MC_GET_NODE_AREA(area_node_ptr), "/foo/bar", "bar", 
                    0x2000, 0x3000, MC_ACCESS_READ, &o_n, NULL, 0, true);


    //initialise lower area
    _init_vm_entry(&entry, 0x1000, 0x2000, 0x600, MC_ACCESS_WRITE, "/foo/bar");
    _map_init_vm_area(&area[1], &entry, &o_n, NULL, &m);
    _create_lst_wrapper(&area_node[1], &area);

    //second test: add lower area to the backing object
    _map_obj_add_area(&o, &area_node[1]);

    _assert_vm_obj(&o, "/foo/bar", "bar", 0x1000, 0x3000, 2, 0, 0, true);
    _assert_vm_obj_list(&o.vm_area_node_ps, state_lower, 2);
    area_node_ptr = MC_GET_NODE_PTR(o.vm_area_node_ps.head);
    _assert_vm_area(MC_GET_NODE_AREA(area_node_ptr), "/foo/bar", "bar", 
                    0x1000, 0x2000, MC_ACCESS_READ, &o_n, NULL, 1, true);
    
    
    //initialise higher area
    _init_vm_entry(&entry, 0x4000, 0x5000, 0x900, MC_ACCESS_EXEC, "/foo/bar");
    _map_init_vm_area(&area[2], &entry, &o_n, NULL, &m);
    _create_lst_wrapper(&area_node[2], &area);

    //third test: add lower area to the backing object
    _map_obj_add_area(&o, &area_node[2]);

    _assert_vm_obj(&o, "/foo/bar", "bar", 0x1000, 0x5000, 3, 0, 0, true);
    _assert_vm_obj_list(&o.vm_area_node_ps, state_higher, 3);
    area_node_ptr = MC_GET_NODE_PTR(o.vm_area_node_ps.head->prev);
    _assert_vm_area(MC_GET_NODE_AREA(area_node_ptr), "/foo/bar", "bar", 
                    0x4000, 0x5000, MC_ACCESS_EXEC, &o_n, NULL, 2, true);


    //initialise middle area
    _init_vm_entry(&entry, 0x3000, 0x4000, 0x880, MC_ACCESS_READ, "/foo/bar");
    _map_init_vm_area(&area[3], &entry, &o_n, NULL, &m);
    _create_lst_wrapper(&area_node[3], &area);

    //fourth test: add middle area to the backing object
    _map_obj_add_area(&o, &area_node[3]);

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
    _map_init_vm_area(&last_area[0], &entry, &o_n, NULL, &m);
    _create_lst_wrapper(&last_area_node[0], &last_area[0]);

    //first test: add first area to the backing object
    _map_obj_add_area(&o, &last_area_node[0]);

    _assert_vm_obj(&o, "/foo/bar", "bar", 0x0, 0x0, 0, 1, 0, true);
    _assert_vm_obj_list(&o.last_vm_area_node_ps, state_first, 1);
    last_area_node_ptr = MC_GET_NODE_PTR(o.vm_area_node_ps.head);
    _assert_vm_area(MC_GET_NODE_AREA(last_area_node_ptr), "anonmap", "anonmap", 
                    0x2000, 0x3000, MC_ACCESS_READ, &o_n, NULL, 0, true);


    //initialise lower area
    _init_vm_entry(&entry, 0x1000, 0x2000, 0x600, MC_ACCESS_WRITE, "/bin/cat");
    _map_init_vm_area(&last_area[1], &entry, &o_n, NULL, &m);
    _create_lst_wrapper(&last_area_node[1], &last_area);

    //second test: add lower area to the backing object
    _map_obj_add_area(&o, &last_area_node[1]);

    _assert_vm_obj(&o, "/foo/bar", "bar", 0x0, 0x0, 0, 2, 0, true);
    _assert_vm_obj_list(&o.last_vm_area_node_ps, state_lower, 2);
    last_area_node_ptr = MC_GET_NODE_PTR(o.vm_area_node_ps.head);
    _assert_vm_area(MC_GET_NODE_AREA(last_area_node_ptr), 
                    "/bin/cat", "cat", 0x1000, 0x2000, MC_ACCESS_READ, 
                    &o_n, NULL, 1, true);
    
    
    //initialise higher area
    _init_vm_entry(&entry, 0x4000, 0x5000, 0x900, MC_ACCESS_EXEC, "/lib/std");
    _map_init_vm_area(&last_area[2], &entry, &o_n, NULL, &m);
    _create_lst_wrapper(&last_area_node[2], &last_area);

    //third test: add lower area to the backing object
    _map_obj_add_area(&o, &last_area_node[2]);

    _assert_vm_obj(&o, "/lib/std", "std", 0x0, 0x0, 0, 3, 0, true);
    _assert_vm_obj_list(&o.last_vm_area_node_ps, state_higher, 3);
    last_area_node_ptr = MC_GET_NODE_PTR(o.vm_area_node_ps.head->prev);
    _assert_vm_area(MC_GET_NODE_AREA(last_area_node_ptr), "/foo/bar", "bar", 
                    0x4000, 0x5000, MC_ACCESS_EXEC, &o_n, NULL, 2, true);


    //initialise middle area
    _init_vm_entry(&entry, 0x3000, 0x4000, 0x880, MC_ACCESS_READ, "io");
    _map_init_vm_area(&last_area[3], &entry, &o_n, NULL, &m);
    _create_lst_wrapper(&last_area_node[3], &last_area);

    //fourth test: add middle area to the backing object
    _map_obj_add_area(&o, &last_area_node[3]);

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


    //first test: remove middle area
    ret = _map_obj_rmv_area(&o, &o_a_n[1]);
    ck_assert_int_eq(ret, 0);

    _assert_vm_obj(&o, "/foo/bar", "bar", 0x1000, 0x5000, 3, 2, 0, true);
    _assert_vm_obj_list(&o.vm_area_node_ps, state_middle, 3);
    

    //second test: remove first area
    ret = _map_obj_rmv_area(&o, &o_a_n[0]);
    ck_assert_int_eq(ret, 0);

    _assert_vm_obj(&o, "/foo/bar", "bar", 0x3000, 0x5000, 2, 2, 0, true);
    _assert_vm_obj_list(&o.vm_area_node_ps, state_first, 2);


    //third test: remove last area
    ret = _map_obj_rmv_area(&o, &o_a_n[3]);
    ck_assert_int_eq(ret, 0);

    _assert_vm_obj(&o, "/foo/bar", "bar", 0x3000, 0x4000, 1, 2, 0, true);
    _assert_vm_obj_list(&o.vm_area_node_ps, state_last, 1);


    //fourth test: remove only remaining area
    ret = _map_obj_rmv_area(&o, &o_a_n[2]);
    ck_assert_int_eq(ret, 0);

    _assert_vm_obj(&o, "/foo/bar", "bar", 0x0, 0x0, 0, 2, 0, true);
    _assert_vm_obj_list(&o.vm_area_node_ps, NULL, 0);

    return;
    
} END_TEST



//_map_obj_rmv_last_area [stub object fixture]
START_TEST(test__map_obj_rmv_last_area) {

    int ret;

    uintptr_t state_first[1] = {0x6000};


    //first test: remove first last area
    ret = _map_obj_rmv_last_area(&o, &o_a_l_n[0]);
    ck_assert_int_eq(ret, 0);

    _assert_vm_obj(&o, "/foo/bar", "bar", 0x1000, 0x5000, 4, 1, 0, true);
    _assert_vm_obj_list(&o.last_vm_area_node_ps, state_first, 1);


    //second test: remove only remaining last area
    ret = _map_obj_rmv_last_area(&o, &o_a_l_n[1]);
    ck_assert_int_eq(ret, 0);

    _assert_vm_obj(&o, "/foo/bar", "bar", 0x1000, 0x5000, 4, 0, 0, true);
    _assert_vm_obj_list(&o.last_vm_area_node_ps, NULL, 0);
    
    return;
}



//_map_is_pathname_in_obj() [empty object fixture]
START_TEST(test__map_is_pathname_in_obj) {

    bool ret;


    //first test: path is in the object
    ret = _map_is_pathname_in_obj("/foo/bar", &o);
    ck_assert(ret);    

    //second test: path is not in the object
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


    //construct test objects
    for (int i = 0; i < 3; ++i) {
        _map_new_vm_obj(&objs[i], &m, obj_paths[i]);
        _create_lst_wrapper(&obj_nodes[i], &objs[i]);
    }


    //first test: new object, state empty
    _init__traverse_state(&state, NULL, NULL);
    _init_vm_entry(&entry, 0x1000, 0x2000, 0x500, 
                   MC_ACCESS_READ, "/lib/libpthread");

    ret = _map_find_obj_for_area(&entry, &state);
    ck_assert_int_eq(ret, _MAP_OBJ_NEW);

    
    //second test: new object, state full
    _init__traverse_state(&state, NULL, &obj_nodes[1]);
    _init_vm_entry(&entry, 0x1000, 0x2000, 0x500, 
                   MC_ACCESS_READ, "/lib/libpthread");

    ret = _map_find_obj_for_area(&entry, &state);
    ck_assert_int_eq(ret, _MAP_OBJ_NEW);


    //third test: previous object 
    _init__traverse_state(&state, NULL, &obj_nodes[1]);
    _init_vm_entry(&entry, 0x1000, 0x2000, 0x500, 
                   MC_ACCESS_READ, "/lib/libc");

    ret = _map_find_obj_for_area(&entry, &state);
    ck_assert_int_eq(ret, _MAP_OBJ_PREV);


    //fourth test: next object 
    _init__traverse_state(&state, NULL, &obj_nodes[1]);
    _init_vm_entry(&entry, 0x1000, 0x2000, 0x500, 
                   MC_ACCESS_READ, "anonmap");

    ret = _map_find_obj_for_area(&entry, &state);
    ck_assert_int_eq(ret, _MAP_OBJ_NEXT);


    //destruct objects
    for (int i = 0; i < 3; ++i) {
        _map_del_vm_obj(&objs[i]);
    }

    return;
    
} END_TEST



//_map_backtrack_unmapped_obj_last_vm_areas() [stub map fixture]
START_TEST(test__map_backtrack_unmapped_obj_last_vm_areas) {

    int ret;

    mc_vm_obj * zero_obj;
    cm_lst_node * zero_node;

    uintptr_t first_state[2]  = {0x6000, 0xC000};
    uintptr_t second_state[2] = {0x6000, 0xC000};
    uintptr_t third_state[2]  = {0x6000, 0xC000};
    

    //first test: backtrack `/lib/foo`'s last area (index: 8)
    ret = _map_backtrack_unmapped_obj_last_vm_areas(&m_o_n[2]);
    ck_assert_int_eq(ret, 0);
    
    //check `/lib/foo` no longer has any last areas associated with it
    _assert_vm_obj(&m_o[2], "/lib/foo", "foo", 0x8000, 0xB000, 3, 0, 2, true);

    //check `[heap]` now has two last areas associated with it
    _assert_vm_obj(&m_o[1], "[heap]", "[heap]", 0x4000, 0x5000, 1, 2, 1, true);
    _assert_vm_obj_list(&m_o[1].last_vm_area_node_ps, first_state, 2);

    //check the transfered last area (index: 8) now points to `[heap]`
    _assert_vm_area(&m_a[8], NULL, NULL, 0xC000, 0xD000,
                    MC_ACCESS_READ | MC_ACCESS_WRITE, 
                    NULL, &m_o_n[1], 8, true);


    //second test: backtrack `[heap]`'s two last areas (indeces: 4, 8)
    ret = _map_backtrack_unmapped_obj_last_vm_areas(&m_o_n[1]);
    ck_assert_int_eq(ret, 0);

    //check `[heap]` no longer has any last areas associated with it
    _assert_vm_obj(&m_o[1], "[heap]", "[heap]", 0x4000, 0x5000, 1, 0, 1, true);

    //check `/bin/cat` now has two last areas associated with it
    _assert_vm_obj(&m_o[0], "/bin/cat", "cat", 0x1000, 0x4000, 3, 2, 0, true);
    _assert_vm_obj_list(&m_o[0].last_vm_area_node_ps, first_state, 2);

    //check the transfered last areas (indeces: 4, 8) now point to `/bin/cat`
    _assert_vm_area(&m_a[4], NULL, NULL, 0xC000, 0xD000,
                    MC_ACCESS_READ | MC_ACCESS_WRITE, NULL, &m_o_n[0], 4, true);
    
    _assert_vm_area(&m_a[8], NULL, NULL, 0xC000, 0xD000,
                    MC_ACCESS_READ | MC_ACCESS_WRITE, NULL, &m_o_n[0], 8, true);

    
    //third test: backtrack `/bin/cat`'s two lat areas (indeces: 4, 8)
    ret = _map_backtrack_unmapped_obj_last_vm_areas(&m_o_n[0]);
    ck_assert_int_eq(ret, 0);

    //get the pseudo object
    zero_node = cm_lst_get_n(&m.vm_objs, 0);
    zero_obj = MC_GET_NODE_OBJ(zero_node);
    
    //check `/bin/cat` no longer has any last areas associated with it
    _assert_vm_obj(&m_o[0], "/bin/cat", "cat", 0x1000, 0x4000, 3, 0, 0, true);

    //check the pseudo object now has two last areas associated with it
    _assert_vm_obj(zero_obj, "0x0", "0x0", 0x0, 0x0, 
                   0, 2, MC_ZERO_OBJ_ID, true);
    _assert_vm_obj_list(&zero_obj->last_vm_area_node_ps, third_state, 2);

    //check the transfered last areas (indeces: 4, 8) now point to `/bin/cat`
    _assert_vm_area(&m_a[4], NULL, NULL, 0xC000, 0xD000,
                    MC_ACCESS_READ | MC_ACCESS_WRITE, NULL, zero_node, 4, true);
    
    _assert_vm_area(&m_a[8], NULL, NULL, 0xC000, 0xD000,
                    MC_ACCESS_READ | MC_ACCESS_WRITE, NULL, zero_node, 8, true);
    
    return;
    
} END_TEST



//_map_forward_unmapped_obj_last_vm_areas() [stub map fixture]
START_TEST(test__map_forward_unmapped_obj_last_vm_areas) {

    int ret;

    uintptr_t heap_state[2]  = {0x6000};
    uintptr_t lib_foo_state[2] = {0xC000};


    //setup the test by backtracking `/lib/foo`'s last area.
    ret = _map_backtrack_unmapped_obj_last_vm_areas(&m_o_n[2]);
    ck_assert_int_eq(ret, 0);


    //only test: pretend the `/lib/foo` object was just inserted
    ret = _map_forward_unmapped_obj_last_vm_areas(&m_o_n[2]);
    ck_assert_int_eq(ret, 0);
    
    //check `[heap]` has only one last area associated with it
    _assert_vm_obj(&m_o[1], "[heap]", "[heap]", 0x4000, 0x5000, 1, 1, 1, true);
    _assert_vm_obj_list(&m_o[1].last_vm_area_node_ps, heap_state, 1);

    //check `/lib/foo` now has one last area associated with it
    _assert_vm_obj(&m_o[2], "/lib/foo", "foo", 0x8000, 0xB000, 3, 2, 0, true);
    _assert_vm_obj_list(&m_o[2].last_vm_area_node_ps, lib_foo_state, 1);

    //check the transfered last areas (indeces: 4, 8) now point to `/bin/cat`
    _assert_vm_area(&m_a[4], NULL, NULL, 0xC000, 0xD000,
                    MC_ACCESS_READ | MC_ACCESS_WRITE, NULL, &m_o_n[1], 4, true);
    
    _assert_vm_area(&m_a[8], NULL, NULL, 0xC000, 0xD000,
                    MC_ACCESS_READ | MC_ACCESS_WRITE, NULL, &m_o_n[2], 8, true);

    return;
    
} END_TEST



//_map_unlink_unmapped_obj()
START_TEST(test__map_unlink_unmapped_obj) {

    int ret;
    uintptr_t heap_state[2]  = {0x6000, 0xC000};
    
    struct obj_check obj_state[4] = {          //start index: 0
        {"0x0",     0x0,    0x0},
        {"cat",     0x1000, 0x4000},
        {"[heap]",  0x4000, 0x5000},
        {"[stack]", 0xE000, 0xF000}
    };
    
    struct obj_check unmapped_obj_state[1] = { //start index: 0
        {"foo",     0x8000, 0xB000}
    };

                                
    //only test: unlink `/lib/foo`
    ret = _map_unlink_unmapped_obj(&m_o_n[2], &m);
    ck_assert_int_eq(ret, 0);

    //check `/lib/foo` has no last areas associated with it, and is unmapped
    _assert_vm_obj(&m_o[2], "/lib/foo", "foo", 0x8000, 0xB000, 3, 0, 2, false);
    _assert_vm_obj_list(&m_o[2].last_vm_area_node_ps, NULL, 0);

    //check `[heap]` has no last areas associated with it
    _assert_vm_obj(&m_o[1], "[heap]", "[heap]", 0x4000, 0x5000, 1, 2, 1, true);
    _assert_vm_obj_list(&m_o[1].last_vm_area_node_ps, heap_state, 2);


    //check state of mapped objects
    _assert_vm_map_objs(&m.vm_objs, obj_state, 0, 4);

    //check state of unmapped objects
    _assert_vm_map_objs(&m.vm_objs_unmapped, unmapped_obj_state, 0, 1);

    //check removed object has no links to other objects anymore
    ck_assert(m_o[2].mapped == false);
    ck_assert_int_eq(m_o[2].start_addr, 0x0);
    ck_assert_int_eq(m_o[2].end_addr, 0x0);
    ck_assert_ptr_null(m_o_n[2].next);
    ck_assert_ptr_null(m_o_n[2].prev);

    return;
    
} END_TEST



//_map_unlink_unmapped_area() [stub map fixture]
START_TEST(test__map_unlink_unmapped_area) {
    
    int ret;


    //remove /lib/foo:1: object state
    struct obj_check foo_obj_state[3] = {             //start index: 2
        {"[heap]",  0x4000, 0x5000},
        {"foo",     0x9000, 0xB000},
        {"[stack]", 0xE000, 0xF000}
    };

    //remove /lib/foo:1: area state
    struct area_check foo_area_state[3] = {           //start index: 4
        {NULL,      0x6000, 0x7000},
        {"foo",     0x9000, 0xA000},
        {"foo",     0xA000, 0xB000}
    };

    struct area_check foo_unmapped_area_state[1] = {  //start index: 0
        {"foo",     0x8000, 0x9000}
    };


    //remove [heap]: object state
    struct obj_check heap_obj_state[2] = {            //start index: 1 
        {"cat",     0x1000, 0x4000},
        {"foo",     0x9000, 0xB000}
    };
    
    struct obj_check heap_unmapped_obj_state[1] = {   //start index: 0
        {"[heap]",  0x0,    0x0}
    };

    //remove [heap]: area state:
    struct area_check heap_area_state[2] = {          //start index: 2
        {"cat",     0x3000, 0x4000},
        {NULL,      0x6000, 0x7000}
    };

    struct area_check heap_unmapped_area_state[2] = { //start index: 0
        {"foo",     0x8000, 0x9000},
        {"[heap]",  0x4000, 0x5000}
    };
    

    //first test: remove first area of `/lib/foo`
    ret = _map_unlink_unmapped_area(&m_a_n[6], &m);
    ck_assert_int_eq(ret, 0);

    _assert_vm_area(&m_a[5], "/lib/foo", "foo", 
                    0x8000, 0x9000, MC_ACCESS_READ | MC_ACCESS_WRITE, 
                    NULL, NULL, 5, false);
    
    _assert_vm_map_objs(&m.vm_objs, foo_obj_state, 2, 3);
    _assert_vm_map_objs(&m.vm_objs_unmapped, NULL, 0, 0);
    _assert_vm_map_areas(&m.vm_areas, foo_area_state, 4, 3);
    _assert_vm_map_areas(&m.vm_areas_unmapped, foo_unmapped_area_state, 0, 1);


    //second test: remove only area of '[heap]'
    ret = _map_unlink_unmapped_area(&m_a_n[3], &m);
    ck_assert_int_eq(ret, 0);

    _assert_vm_area(&m_a[3], "[heap]", "[heap]", 
                    0x4000, 0x5000, MC_ACCESS_READ | MC_ACCESS_WRITE, 
                    NULL, NULL, 5, false);
    
    _assert_vm_map_objs(&m.vm_objs, heap_obj_state, 1, 2);
    _assert_vm_map_objs(&m.vm_objs_unmapped, heap_unmapped_obj_state, 0, 1);
    _assert_vm_map_areas(&m.vm_areas, heap_area_state, 2, 2);
    _assert_vm_map_areas(&m.vm_areas_unmapped, heap_unmapped_area_state, 0, 2);

    return;
    
} END_TEST



//_map_check_area_eql() [empty map fixture]
START_TEST(test__map_check_area_eql) {
    
    int ret;
    
    struct vm_entry entry;
    mc_vm_area area;
    cm_lst_node area_node;

    mc_vm_obj obj;
    cm_lst_node obj_node;


    //construct a vm_obj
    _map_new_vm_obj(&obj, &m, "/bin/cat");
    _create_lst_wrapper(&obj_node, &obj);

    //create a vm_area
    _init_vm_entry(&entry, 0x1000, 0x2000, 0x0, MC_ACCESS_READ, "/bin/cat");
    _map_init_vm_area(&area, &entry, &obj_node, NULL, &m);
    _create_lst_wrapper(&area_node, &area);


    //first test: entry same as vm_area
    ret = _map_check_area_eql(&entry, &area_node);
    ck_assert_int_eq(ret, 0);


    //second test: entry start address is different
    entry.vm_start = 0x500;
    ret = _map_check_area_eql(&entry, &area_node);
    ck_assert_int_eq(ret, -1);
    entry.vm_start = area.start_addr;


    //third test: entry end address is different
    entry.vm_end = 0x2500;
    ret = _map_check_area_eql(&entry, &area_node);
    ck_assert_int_eq(ret, -1);
    entry.vm_end = area.end_addr;


    //fourth test: entry permissions are different
    entry.prot = MC_ACCESS_READ | MC_ACCESS_WRITE;
    ret = _map_check_area_eql(&entry, &area_node);
    ck_assert_int_eq(ret, -1);
    entry.prot = area.access;


    //fifth test: both entry and area don't have a path
    entry.file_path[0] = '\0';
    area.pathname = NULL;
    ret = _map_check_area_eql(&entry, &area_node);
    ck_assert_int_eq(ret, 0);
    entry.file_path[0] = '/';
    area.pathname = obj.pathname;


    //sixth test: entry has a path, area does not
    area.pathname = NULL;
    ret = _map_check_area_eql(&entry, &area_node);
    ck_assert_int_eq(ret, 0);
    area.pathname = obj.pathname;


    //seventh test: entry does not have a path, area does
    entry.file_path[0] = '\0';
    ret = _map_check_area_eql(&entry, &area_node);
    ck_assert_int_eq(ret, 0);
    entry.file_path[0] = '/';


    //destroy pseudo object
    _map_del_vm_obj(&obj);

    return;
    
} END_TEST



//_map_state_inc_area() [stub map fixture]
START_TEST(test__map_state_inc_area) {

    _traverse_state state;


    //first test: keep
    state.next_area_node = &m_a_n[0];
    _map_state_inc_area(&state, _STATE_AREA_NODE_KEEP, NULL, &m);
    ck_assert_int_eq(MC_GET_NODE_AREA(state.next_area_node)->id, 0);

    //second test: advance - success
    state.next_area_node = &m_a_n[0];
    _map_state_inc_area(&state, _STATE_AREA_NODE_ADVANCE, NULL, &m);
    ck_assert_int_eq(MC_GET_NODE_AREA(state.next_area_node)->id, 1);

    //third test: advance - refuse (reached the end)
    state.next_area_node = &m_a_n[9];
    _map_state_inc_area(&state, _STATE_AREA_NODE_ADVANCE, NULL, &m);
    ck_assert_int_eq(MC_GET_NODE_AREA(state.next_area_node)->id, 9);

    //fourth test: reassign
    state.next_area_node = &m_a_n[0];
    _map_state_inc_area(&state, _STATE_AREA_NODE_REASSIGN, &m_a_n[2], &m);
    ck_assert_int_eq(MC_GET_NODE_AREA(state.next_area_node)->id, 2);

    return;
    
} END_TEST



//_map_state_inc_obj() [stub map fixture]
START_TEST(test__map_state_inc_obj) {

    _traverse_state state;


    //first test: advance from pseudo object
    state.prev_obj_node = m.vm_objs.head;
    _map_state_inc_obj(&state, &m);
    ck_assert_int_eq(MC_GET_NODE_OBJ(state.prev_obj_node), 0);
    
    //second test: advance from regular object
    state.prev_obj_node = &m_a_n[0];
    _map_state_inc_obj(&state, &m);
    ck_assert_int_eq(MC_GET_NODE_OBJ(state.prev_obj_node), 1);

    return;
    
} END_TEST



//_map_resync_area() [stub map fixture]
START_TEST(test__map_resync_area) {

    //remove [heap]: object state
    struct obj_check heap_obj_state[2] = {            //start index: 1
        {"cat",     0x1000, 0x4000},
        {"foo",     0x9000, 0xB000}
    };

    struct obj_check heap_unmapped_obj_state[1] = {   //start index: 0
        {"[heap]",  0x4000, 0x5000}
    };
    
    //remove [heap]: area state
    struct area_check heap_area_state[2] = {          //start index: 2
        {"cat",     0x3000, 0x4000},
        {NULL,      0x6000, 0x7000}
    };

    struct area_check heap_unmapped_area_state[1] = { //start index 0
        {"[heap]",  0x4000, 0x5000}
    };



    //remove /lib/foo:1,2: object state
    struct obj_check foo_obj_state[3] = {             //start index: 1
        {"cat",     0x1000, 0x4000},
        {"foo",     0xA000, 0xB000},
        {"[stack]", 0xE000, 0xF000}
    };

    struct obj_check foo_unmapped_obj_state[1] = {    //start index 0
        {"[heap]",  0x4000, 0x5000}
    };
    
    //remove /lib/foo:1,2: area state
    struct area_check foo_area_state[3] = {           //start index 3
        {NULL,      0x6000, 0x7000},
        {"foo",     0xA000, 0xB000},
        {NULL,      0xC000, 0xD000}
    };

    struct area_check foo_unmapped_area_state[3] = {  //start index 0
        {"[heap]",  0x4000, 0x5000},
        {"foo",     0x8000, 0x9000},
        {"foo",     0x9000, 0xA000},
    };



    //remove /bin/cat:1,2,3: object state
    struct obj_check cat_obj_state[2] = {             //start index: 1
        {"foo", 0xA000, 0xB000},
        {"[stack]", 0xE000, 0xF000}
    };

    struct obj_check cat_unmapped_obj_state[2] = {    //start index: 0
        {"cat", 0x0, 0x0},
        {"[heap]", 0x0, 0x0}
    };
    
    //remove /bin/cat:1,2,3: area state
    struct area_check cat_area_state[2] = {           //start index: 0
        {NULL, 0x6000, 0x7000},
        {"foo", 0xA000, 0xB000}
    };

    struct area_check cat_unmapped_area_state[6] = {  //start index: 0
        {"[heap]", 0x4000, 0x5000},
        {"foo", 0x8000, 0x9000},
        {"foo", 0x9000, 0xA000},
        {"cat", 0x1000, 0x2000},
        {"cat", 0x2000, 0x3000},
        {"cat", 0x3000, 0x4000}
    };
    
    int ret;
    
    struct vm_entry entry;
    _traverse_state state;


    //correct by removing `[heap]`
    _init_vm_entry(&entry, 0x6000, 0x7000, 0x0, 
                   MC_ACCESS_READ | MC_ACCESS_WRITE, NULL);
    _init__traverse_state(&state, &m_a_n[6], &m_o_n[0]);

    ret = _map_resync_area(&entry, &state, &m);
    ck_assert_int_eq(ret, 0);

    //check state
    _assert_vm_area(MC_GET_NODE_AREA(state.next_area_node), NULL, NULL, 
                    0x6000, 0x7000, MC_ACCESS_READ | MC_ACCESS_WRITE, 
                    NULL, NULL, 5, true);
    _assert_vm_obj(MC_GET_NODE_OBJ(state.prev_obj_node), 
                   "/bin/cat", "cat", 0x1000, 0x3000, 3, 1, 0, true);
    
    _assert_vm_map_objs(&m.vm_objs, heap_obj_state, 1, 2);
    _assert_vm_map_objs(&m.vm_objs_unmapped, heap_unmapped_obj_state, 0, 1);
    _assert_vm_map_areas(&m.vm_areas, heap_area_state, 3, 3);
    _assert_vm_map_areas(&m.vm_areas_unmapped, heap_unmapped_area_state, 0, 3);
    


    //correct by removing first 2 areas of `/lib/foo`
    _init_vm_entry(&entry, 0xA000, 0xB000, 0x0, 
                   MC_ACCESS_READ | MC_ACCESS_EXEC, "/lib/foo");
    _init__traverse_state(&state, &m_a_n[5], &m_o_n[2]);

    ret = _map_resync_area(&entry, &state, &m);
    ck_assert_int_eq(ret, 0);

    //check state
    _assert_vm_area(MC_GET_NODE_AREA(state.next_area_node), NULL, NULL,
                    0xC000, 0xD000, MC_ACCESS_READ | MC_ACCESS_WRITE, 
                    NULL, &m_o_n[2], 8, true);

    _assert_vm_obj(MC_GET_NODE_OBJ(state.prev_obj_node), "/lib/foo", "foo",
                   0xA000, 0xB000, 1, 1, 2, true);

    _assert_vm_map_objs(&m.vm_objs, foo_obj_state, 1, 3);
    _assert_vm_map_objs(&m.vm_objs_unmapped, foo_unmapped_obj_state, 0, 1);
    _assert_vm_map_areas(&m.vm_areas, foo_area_state, 3, 3);
    _assert_vm_map_areas(&m.vm_areas_unmapped, foo_unmapped_area_state, 0, 3);



    //correct by removing entirety of `/bin/cat`
    _init_vm_entry(&entry, 0x6000, 0x7000, 0x0, 
                   MC_ACCESS_READ | MC_ACCESS_EXEC, NULL);
    _init__traverse_state(&state, &m_a_n[0], &m_o_n[0]);

    ret = _map_resync_area(&entry, &state, &m);
    ck_assert_int_eq(ret, 0);

    //check state
    _assert_vm_area(MC_GET_NODE_AREA(state.next_area_node), NULL, NULL, 
                    0x6000, 0x7000, MC_ACCESS_READ | MC_ACCESS_WRITE, 
                    NULL, m.vm_objs.head, 4, true);

    _assert_vm_obj(MC_GET_NODE_OBJ(m.vm_objs.head), "0x0", "0x0", 
                   0x0, 0x0, 0, 2, MC_ZERO_OBJ_ID, true);

    _assert_vm_map_objs(&m.vm_objs, cat_obj_state, 1, 2);
    _assert_vm_map_objs(&m.vm_objs_unmapped, cat_unmapped_obj_state, 0, 2);
    _assert_vm_map_areas(&m.vm_areas, cat_area_state, 0, 2);
    _assert_vm_map_areas(&m.vm_areas_unmapped, cat_unmapped_area_state, 0, 6);

    return;
    
} END_TEST



//_map_add_obj() [stub map fixture]
START_TEST(test__map_add_obj) {

    //test data
    struct obj_check first_objs[3] = {  //start index: 3
        {"foo",     0x8000, 0xB000},
        {"bar",     0xD000, 0xE000},
        {"[stack]", 0xE000, 0xF000}
    };

    struct obj_check second_objs[3] = { //start index: 2
        {"[heap]",  0x4000, 0x5000},
        {"dog",     0x7000, 0x8000},
        {"foo",     0x8000, 0xB000}
    };


    //test vars
    cm_lst_node * ret_node;

    struct vm_entry entry;
    _traverse_state state;


    //first test: no forwarding last memory areas
    _init_vm_entry(&entry, 0xD000, 0xE000, 0x0, MC_ACCESS_READ, "/lib/bar");
    _init__traverse_state(&state, NULL, &m_o_n[2]);

    ret_node = _map_add_obj(&entry, &state, &m);
    ck_assert_ptr_nonnull(ret_node);

    _assert_vm_map_objs(&m.vm_objs, first_objs, 3, 3);
    _assert_lst_len(&MC_GET_NODE_OBJ(ret_node)->last_vm_area_node_ps, 0);


    //second test: forwarding last memory areas
    _init_vm_entry(&entry, 0x5000, 0x6000, 0x0, MC_ACCESS_READ, "/bin/dog");
    _init__traverse_state(&state, NULL, &m_o_n[1]);

    ret_node = _map_add_obj(&entry, &state, &m);
    ck_assert_ptr_nonnull(ret_node);

    _assert_vm_map_objs(&m.vm_objs, second_objs, 3, 2);
    _assert_lst_len(&MC_GET_NODE_OBJ(ret_node)->last_vm_area_node_ps, 1);
    
    return;
    
} END_TEST



//_map_add_area() [stub map fixture]
START_TEST(test__map_add_area) {
    
    //test data
    struct area_check first_areas[3] = {  //start_index: 7
        {"foo",     0xA000, 0xB000},
        {"foo",     0xB000, 0xC000},
        {NULL,      0xC000, 0xD000}
    };
    
    struct obj_check first_objs[2] = {    //start index: 3
        {"foo",     0x8000, 0xC000},
        {"[stack]", 0xE000, 0xF000}
    };

    struct area_check second_areas[3] = { //start_index: 6
        {NULL,      0x6000, 0x7000},
        {NULL,      0x7000, 0x8000},
        {"foo",     0x8000, 0x9000}
    };

    struct obj_check second_objs[2] = {   //start index: 2
        {"[heap]",  0x4000, 0x5000},
        {"foo",     0x8000, 0xC000},
    };

    struct area_check third_areas[3] = {  //start_index: 3
        {"[heap]",  0x4000, 0x5000},
        {"bar",     0x5000, 0x6000},
        {NULL,      0x6000, 0x7000}
    };
    
    struct obj_check third_objs[3] = {    //start index: 2
        {"[heap]",  0x4000, 0x5000},
        {"bar",     0x5000, 0x6000},
        {"foo",     0x8000, 0xC000}
    };


    //test vars
    int ret;

    struct vm_entry entry;
    _traverse_state state;


    //first test: add additional area to the end of `/lib/foo`
    _init_vm_entry(&entry, 0xB000, 0xC000, 0x0, MC_ACCESS_READ, "/lib/foo");
    _init__traverse_state(&state, &m_a_n[8], &m_o_n[2]);

    ret = _map_add_area(&entry, &state, &m);
    ck_assert_int_eq(ret, 0);

    _assert_vm_map_areas(&m.vm_areas, first_areas, 7, 3);
    _assert_vm_map_objs(&m.vm_objs, first_objs, 3, 2);


    //second test: add area without a backing object before `/lib/foo`
    _init_vm_entry(&entry, 0x7000, 0x8000, 0x0, MC_ACCESS_READ, NULL);
    _init__traverse_state(&state, &m_a_n[5], &m_o_n[1]);

    ret = _map_add_area(&entry, &state, &m);
    ck_assert_int_eq(ret, 0);

    _assert_vm_map_areas(&m.vm_areas, first_areas, 6, 3);
    _assert_vm_map_objs(&m.vm_objs, first_objs, 2, 2);


    //third test: add an area that creates a new object `/lib/bar`
    _init_vm_entry(&entry, 0x5000, 0x6000, 0x0, MC_ACCESS_READ, "/lib/bar");
    _init__traverse_state(&state, &m_a_n[4], &m_o_n[1]);

    ret = _map_add_area(&entry, &state, &m);
    ck_assert_int_eq(ret, 0);

    _assert_vm_map_areas(&m.vm_areas, first_areas, 3, 3);
    _assert_vm_map_objs(&m.vm_objs, first_objs, 2, 3);
    
    return;
    
} END_TEST
#endif
