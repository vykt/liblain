//standard library
#include <stdlib.h>
#include <string.h>

//system headers
#include <unistd.h>

#include <linux/limits.h>

//external libraries
#include <cmore.h>
#include <check.h>

//local headers
#include "map_helper.h"
#include "suites.h"

//test target headers
#include "../lib/memcry.h"
#include "../lib/map.h"


//initialise a cm_lst_node stub wrapper
void create_lst_wrapper(cm_lst_node * node, void * data) {

    node->data = data;
    node->next = node->prev = NULL;    

    return;
}


//assert the length of a list, also works as an integration test for CMore
void assert_lst_len(cm_lst * list, int len) {

    ck_assert_int_eq(list->len, len);

    //if length is zero (0), ensure head is null
    if (len == 0) {
        ck_assert_ptr_null(list->head);
        return;
    }

    //if length is one (1), ensure head is not null
    ck_assert_ptr_nonnull(list->head);
    cm_lst_node * iter = list->head;

    if (len == 1) {
        ck_assert_ptr_null(iter->next);
        ck_assert_ptr_null(iter->prev);
        return;
    }

    //if length is greater than 1 (1), iterate over nodes to ensure length
    ck_assert_ptr_nonnull(iter->next);
    iter = iter->next;

    for (int i = 1; i < len; ++i) {
        ck_assert(iter != list->head);
        iter = iter->next;
    }
    
    return;
}


//properly assert a potentially NULL string pair
void assert_names(char * a, char * b) {

    //determine comparison type
    int cmp_type = (a == NULL ? 0 : 1) + (b == NULL ? 0 : 2);


    switch (cmp_type) {

        case 0:
            ck_assert_ptr_eq(a, b);
            break;
        case 1:
            ck_assert(*a == '\0' && b == NULL);
            break;
        case 2:
            ck_assert(a == NULL && *b == '\0');
            break;
        case 3:
            ck_assert_str_eq(a, b);
            break;
        
    } //end switch

    return;
}


//basic assertion of state for a mc_vm_map
void assert_vm_map(mc_vm_map * map, int vm_areas_len, int vm_objs_len,
                   int vm_areas_unmapped_len, int vm_objs_unmapped_len,
                   int next_id_area, int next_id_obj) {

    //check mapped lists
    assert_lst_len(&map->vm_areas, vm_areas_len);
    assert_lst_len(&map->vm_objs, vm_objs_len);

    //check unmapped lists
    assert_lst_len(&map->vm_areas_unmapped, vm_areas_unmapped_len);
    assert_lst_len(&map->vm_objs_unmapped, vm_objs_unmapped_len);

    //check next IDs
    ck_assert_int_eq(map->next_id_area, next_id_area);
    ck_assert_int_eq(map->next_id_obj, next_id_obj);

    return;
}


/*
 *  NOTE: The mapped area & object lists store areas and objects directly.
 *        Unmapped area & object lists store unmapped nodes instead, which 
 *        means an additional pointer dereference is required.
 */

//assert the state of all [unmapped] objects inside a mc_vm_map
void assert_vm_map_objs(cm_lst * lst, struct obj_check * obj_checks, 
                        int start_index, int len, bool mapped) {

    int ret;

    cm_lst_node * node;
    mc_vm_obj * obj;


    for (int i = 0; i < len; ++i) {

        if (!mapped) {
            ret = cm_lst_get(lst, start_index + i, &node);
            ck_assert_int_eq(ret, 0);
            obj = MC_GET_NODE_OBJ(node);

        } else {
            obj = cm_lst_get_p(lst, start_index + i);
            ck_assert_ptr_nonnull(obj);
        }

        assert_names(obj->basename, obj_checks[i].basename);
        ck_assert_int_eq(obj->start_addr, obj_checks[i].start_addr);
        ck_assert_int_eq(obj->end_addr, obj_checks[i].end_addr);
    }

    return;
}


//assert only pathnames, not mapped address ranges
void assert_vm_map_objs_aslr(cm_lst * lst, char * basenames[NAME_MAX],
                             int start_index, int len, bool mapped) {

    int ret;

    cm_lst_node * node;
    mc_vm_obj * obj;


    for (int i = 0; i < len; ++i) {

        if (!mapped) {
            ret = cm_lst_get(lst, start_index + i, &node);
            ck_assert_int_eq(ret, 0);
            obj = MC_GET_NODE_OBJ(node);

        } else {
            obj = cm_lst_get_p(lst, start_index + i);
            ck_assert_ptr_nonnull(obj);
        }

        assert_names(obj->basename, basenames[i]);
    }

    return;
}


//assert the state of all [unmapped] memory areas inside a mc_vm_map
void assert_vm_map_areas(cm_lst * lst, struct area_check * area_checks,
                         int start_index, int len, bool mapped) {

    int ret;

    cm_lst_node * node;
    mc_vm_area * area;

    for (int i = 0; i < len; ++i) {

        if (!mapped) {
            ret = cm_lst_get(lst, start_index + i, &node);
            ck_assert_int_eq(ret, 0);
            area = MC_GET_NODE_AREA(node);

        } else {
            area = cm_lst_get_p(lst, start_index + i);
            ck_assert_ptr_nonnull(area);
        }

        assert_names(area->basename, area_checks[i].basename);
        ck_assert_int_eq(area->start_addr, area_checks[i].start_addr);
        ck_assert_int_eq(area->end_addr, area_checks[i].end_addr);
    }

    return;
}


//assert only pathnames, not mapped address ranges
void assert_vm_map_areas_aslr(cm_lst * lst, char * basenames[NAME_MAX],
                              int start_index, int len, bool mapped) {

    int ret;

    cm_lst_node * node;
    mc_vm_area * area;

    for (int i = 0; i < len; ++i) {

        if (!mapped) {
            ret = cm_lst_get(lst, start_index + i, &node);
            ck_assert_int_eq(ret, 0);
            area = MC_GET_NODE_AREA(node);

        } else {
            area = cm_lst_get_p(lst, start_index + i);
            ck_assert_ptr_nonnull(area);
        }

        assert_names(area->basename, basenames[i]);
    }

    return;
}


void assert_vm_obj(mc_vm_obj * obj, char * pathname, char * basename,
                   uintptr_t start_addr, uintptr_t end_addr, int vm_areas_len,
                   int last_vm_areas_len, int id, bool mapped) {

    //check names
    assert_names(obj->pathname, pathname);
    assert_names(obj->basename, basename);

    //check addresses
    ck_assert_int_eq(obj->start_addr, start_addr);
    ck_assert_int_eq(obj->end_addr, end_addr);

    //check area node lists are initialised
    assert_lst_len(&obj->vm_area_node_ps, vm_areas_len);
    assert_lst_len(&obj->last_vm_area_node_ps, last_vm_areas_len);

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
 
void assert_vm_obj_list(cm_lst * outer_node_lst,
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


void assert_vm_area(mc_vm_area * area, char * pathname, char * basename,
                    uintptr_t start_addr, uintptr_t end_addr,
                    cm_byte access, cm_lst_node * obj_node_p,
                    cm_lst_node * last_obj_node_p, int id, bool mapped) {

    //check names
    assert_names(area->pathname, pathname);
    assert_names(area->basename, basename);

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
