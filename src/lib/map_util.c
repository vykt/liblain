#include <string.h>

#include <libcmore.h>

#include "liblain.h"
#include "map_util.h"


#define MAP_UTIL_GET_AREA 0
#define MAP_UTIL_GET_OBJ 1

#define MAP_UTIL_USE_PATHNAME 0
#define MAP_UTIL_USE_BASENAME 1


// --- INTERNAL

//check map is initialised
static inline bool _is_map_empty(const ln_vm_map * vm_map) {

    if (vm_map->vm_areas.len == 0) return true;
    if (vm_map->vm_objs.len == 0) return true;

    return false;
}


//get object's last area
static inline cm_list_node * _get_obj_last_area(const ln_vm_obj * vm_obj) {

    cm_list_node * last_node;

    //if this object has multiple areas
    if (vm_obj->vm_area_node_ptrs.len > 1) {
        last_node = LN_GET_NODE_PTR(vm_obj->vm_area_node_ptrs.head->prev);
    
    //else this object only has one area
    } else { 
        last_node = LN_GET_NODE_PTR(vm_obj->vm_area_node_ptrs.head);
    }

    return last_node;
}


//iterate through objects, then areas for a fast search
static cm_list_node * _fast_addr_find(const ln_vm_map * vm_map, 
                                      const uintptr_t addr, const int mode) {

    cm_list_node * iter_obj_node;
    cm_list_node * iter_area_node;

    ln_vm_obj * iter_vm_obj;
    ln_vm_area * iter_vm_area;

    ln_vm_area * prev_area;


    //check map is not empty
    if (_is_map_empty(vm_map)) return NULL;

    //init object iteration
    iter_obj_node = vm_map->vm_objs.head;
    iter_vm_obj = LN_GET_NODE_OBJ(iter_obj_node);

    iter_area_node = LN_GET_NODE_PTR(iter_vm_obj->vm_area_node_ptrs.head);
    iter_vm_area = LN_GET_NODE_AREA(iter_area_node);


    //if the address is in the very first object
    if (!(iter_vm_obj->end_addr > addr)) {

        //while can still iterate through objects
        for (int i = 1; i < vm_map->vm_objs.len; ++i) {

            //check if addr may be in this object
            if (iter_vm_obj->end_addr > addr) break;

            iter_area_node = _get_obj_last_area(iter_vm_obj);
            iter_vm_area = LN_GET_NODE_AREA(iter_area_node);

            iter_obj_node = iter_obj_node->next;
            iter_vm_obj = LN_GET_NODE_OBJ(iter_obj_node);

        } //end object search   
    
    }//end if

    //if an obj was requested
    if (mode == MAP_UTIL_GET_OBJ) {
        
        //if the address is in range of the obj
        if (iter_vm_obj->start_addr <= addr && iter_vm_obj->end_addr > addr) {
            return iter_obj_node;
        } else {
            return NULL;
        }
    } //end if an obj was requested


    //switch to area search and continue the search from last object's final area
    while (1) {

        //if found a matching area
        if (iter_vm_area->start_addr <= addr && iter_vm_area->end_addr > addr) {
            return iter_area_node;
        }

        //move to next object
        prev_area = iter_vm_area;
        iter_area_node = iter_area_node->next;
        iter_vm_area = LN_GET_NODE_AREA(iter_area_node);

        //check if back to start of dl-list, and backtrack if true
        if (prev_area->start_addr > iter_vm_area->end_addr) {
            break;
        }

    } //end area search


    //exhausted all areas and address was not found
    return NULL;
}


//find object with a given basename/pathname
static cm_list_node * _obj_name_find(const ln_vm_map * vm_map, 
                                     const char * name, const int mode) {

    int ret;

    cm_list_node * iter_obj_node;
    ln_vm_obj * iter_vm_obj;

    //check map is not empty
    if (_is_map_empty(vm_map)) return NULL;


    //init search
    iter_obj_node = vm_map->vm_objs.head;
    iter_vm_obj = LN_GET_NODE_OBJ(iter_obj_node);

    //while can still iterate through objects
    for (int i = 0; i < vm_map->vm_objs.len; ++i) {

        //carry out comparison
        if (mode == MAP_UTIL_USE_PATHNAME) {
            ret = strncmp(name, iter_vm_obj->pathname, PATH_MAX);
        } else {
            ret = strncmp(name, iter_vm_obj->basename, NAME_MAX);
        }

        //if found a match
        if (ret == 0) {
            return iter_obj_node;
        }

        iter_obj_node = iter_obj_node->next;
        iter_vm_obj = LN_GET_NODE_OBJ(iter_obj_node);

    } //end object search

    return NULL;
}



// --- EXTERNAL

//get area offset
off_t ln_get_area_offset(const cm_list_node * area_node, const uintptr_t addr) {

    ln_vm_area * vm_area = LN_GET_NODE_AREA(area_node);

    return (addr - vm_area->start_addr);
}


//get obj offset
off_t ln_get_obj_offset(const cm_list_node * obj_node, const uintptr_t addr) {

    ln_vm_obj * vm_obj = LN_GET_NODE_OBJ(obj_node);

    return (addr - vm_obj->start_addr);
}


//get area offset
off_t ln_get_area_offset_bnd(const cm_list_node * area_node, const uintptr_t addr) {

    ln_vm_area * vm_area = LN_GET_NODE_AREA(area_node);

    if ((addr >= vm_area->end_addr) || (addr < vm_area->start_addr)) return -1;
    return (addr - vm_area->start_addr);
}


//get obj offset
off_t ln_get_obj_offset_bnd(const cm_list_node * obj_node, const uintptr_t addr) {

    ln_vm_obj * vm_obj = LN_GET_NODE_OBJ(obj_node);

    if ((addr >= vm_obj->end_addr) || (addr < vm_obj->start_addr)) return -1;
    return (addr - vm_obj->start_addr);
}



//return the area node at a given address, optionally include offset into the area
cm_list_node * ln_get_vm_area_by_addr(const ln_vm_map * vm_map, 
                                      const uintptr_t addr, off_t * offset) {

    cm_list_node * area_node;

    area_node = _fast_addr_find(vm_map, addr, MAP_UTIL_GET_AREA);
    if (!area_node) return NULL;

    if (offset != NULL) *offset = ln_get_area_offset(area_node, addr);

    return area_node;
}


//return the obj node at a given address, optionally include offset into the obj
cm_list_node * ln_get_vm_obj_by_addr(const ln_vm_map * vm_map, 
                                     const uintptr_t addr, off_t * offset) {

    cm_list_node * obj_node;

    obj_node = _fast_addr_find(vm_map, addr, MAP_UTIL_GET_OBJ);
    if (!obj_node) return NULL;

    if (offset != NULL) *offset = ln_get_obj_offset(obj_node, addr);

    return obj_node;
}


//return object matching pathname
cm_list_node * ln_get_vm_obj_by_pathname(const ln_vm_map * vm_map, 
                                         const char * pathname) {

    cm_list_node * obj_node;

    obj_node = _obj_name_find(vm_map, pathname, MAP_UTIL_USE_PATHNAME);
    if (!obj_node) return NULL;

    return obj_node;
}


//return object matching basename
cm_list_node * ln_get_vm_obj_by_basename(const ln_vm_map * vm_map, 
                                         const char * basename) {

    cm_list_node * obj_node;

    obj_node = _obj_name_find(vm_map, basename, MAP_UTIL_USE_BASENAME);
    if (!obj_node) return NULL;

    return obj_node;
}
