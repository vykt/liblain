#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include <linux/limits.h>

#include <libcmore.h>

#include "liblain.h"
#include "map.h"
#include "util.h"



// --- INTERNAL CONSTRUCTORS & DESTRUCTORS

//ln_vm_area constructor
static void _new_vm_area(ln_vm_area * vm_area, ln_vm_map * vm_map, 
                         cm_list_node * obj_node, 
                         cm_list_node * last_obj_node,
                         struct vm_entry * entry) {

    ln_vm_obj * vm_obj;

    //set pathname for area if applicable
    if (obj_node != NULL) {

        vm_obj = LN_GET_NODE_OBJ(obj_node);

        vm_area->pathname = vm_obj->pathname;
        vm_area->basename = vm_obj->basename;
    
    } else {
        vm_area->pathname = NULL;
        vm_area->basename = NULL;
        
    } //end if

    vm_area->obj_node_ptr      = obj_node;
    vm_area->last_obj_node_ptr = last_obj_node;

    vm_area->start_addr = entry->vm_start;
    vm_area->end_addr   = entry->vm_end;

    vm_area->access = (cm_byte) (entry->prot & 0x0000000F);

    vm_area->mapped = true;

    //set id & increment map's next id
    vm_area->id = vm_map->next_id_area;
    ++vm_map->next_id_area;

    return;
}


//ln_vm_obj constructor
static void _new_vm_obj(ln_vm_obj * vm_obj, ln_vm_map * vm_map, char * pathname) {

    char * basename = ln_pathname_to_basename(pathname);

    strncpy(vm_obj->pathname, pathname, PATH_MAX);
    strncpy(vm_obj->basename, basename, NAME_MAX);

    vm_obj->start_addr = -1;
    vm_obj->end_addr = -1;

    //initialise area list
    cm_new_list(&vm_obj->vm_area_node_ptrs, sizeof(cm_list_node *));

    vm_obj->mapped = true;

    //set id & increment map's next id
    vm_obj->id = vm_map->next_id_obj;
    ++vm_map->next_id_obj;

    return;
}


//ln_vm_obj destructor
static void _del_vm_obj(ln_vm_obj * vm_obj) {

    cm_del_list(&vm_obj->vm_area_node_ptrs);
    
    return;
}



// --- MAP GENERATION INTERNALS

//add an area to an obj (is not called 
static inline int _obj_add_area(ln_vm_obj * obj, cm_list_node * area_node) {

    cm_list_node * ret_node;
    ln_vm_area * area = LN_GET_NODE_AREA(area_node);

    //if this object has no areas yet
    if (obj->start_addr == -1) {

        //set new addr bounds
        obj->start_addr = area->start_addr;
        obj->end_addr = area->end_addr;

    //if this object has areas, only update the start and end addr if necessary
    } else {

        //set new addr bounds if necessary
        if (area->start_addr < obj->start_addr) obj->start_addr = area->start_addr;
        if (area->end_addr > obj->end_addr) obj->end_addr = area->end_addr;        
    }
    
    //simply appending preserves chronological order; out of order vm_areas 
    //are guaranteed(?) to be removed
    ret_node = cm_list_append(&obj->vm_area_node_ptrs, (cm_byte *) &area_node);
    if (ret_node == NULL) {
        ln_errno = LN_ERR_LIBCMORE;
        return -1;
    }

    return 0;
}


//find if vm area's pathname belongs in object
static bool _is_pathname_in_obj(char * pathname, ln_vm_obj * obj) {

    if (obj == NULL) return false;

    bool ret = (strncmp(pathname, obj->pathname, PATH_MAX) ? false : true);
    
    return ret;
}


#define _MAP_OBJ_PREV 0
#define _MAP_OBJ_NEW  1
#define _MAP_OBJ_NEXT 2
static inline int _find_obj_for_area(_traverse_state * state, struct vm_entry * entry) {

    cm_list_node * prev_node, * next_node;
    ln_vm_obj * prev_obj, * next_obj;

    //check for null
    if (state->prev_obj == NULL) return _MAP_OBJ_NEW; 

    //get prev obj
    prev_node = state->prev_obj;
    prev_obj = LN_GET_NODE_OBJ(prev_node);

    if (prev_node->next == NULL) {
        next_obj = NULL;
    } else {
        next_node = prev_node->next;
        next_obj = LN_GET_NODE_OBJ(next_node);
    }

    //return appropriate index
    if (_is_pathname_in_obj(entry->file_path, prev_obj)) return _MAP_OBJ_PREV;
    if (_is_pathname_in_obj(entry->file_path, next_obj)) return _MAP_OBJ_NEXT;

    return _MAP_OBJ_NEW; 
}


//find index of vm area in its corresponding object
static inline int _get_area_index(ln_vm_area * area, ln_vm_obj * obj) {

    int ret;

    cm_list_node * temp_node;
    ln_vm_area * temp_area;

    //for all vm areas in this object
    for (int i = 0; i < obj->vm_area_node_ptrs.len; ++i) {

        ret = cm_list_get_val(&obj->vm_area_node_ptrs, i, (cm_byte *) &temp_node);
        if (ret) return -1;
        temp_area = LN_GET_NODE_AREA(temp_node);
        
        if (temp_area->id == area->id) return i;
        
    } //end for

    return -1;
}


//find the new start and end addresses for an object
static inline int _update_obj_addr_range(ln_vm_obj * obj) {

    int ret, end_index;

    cm_list_node * temp_node;
    ln_vm_area * temp_area;

    //get start addr
    ret = cm_list_get_val(&obj->vm_area_node_ptrs, 0, (cm_byte *) &temp_node);
    if (ret) {
        ln_errno = LN_ERR_LIBCMORE;
        return -1;
    }

    temp_area = LN_GET_NODE_AREA(temp_node);
    obj->start_addr = temp_area->start_addr;

    //get end addr
    if (obj->vm_area_node_ptrs.len == 1) {
        end_index = 0;
    } else {
        end_index = -1;
    }
    
    ret = cm_list_get_val(&obj->vm_area_node_ptrs, end_index, (cm_byte *) &temp_node); 
    if (ret) {
        ln_errno = LN_ERR_LIBCMORE;
        return -1;
    }

    temp_area = LN_GET_NODE_AREA(temp_node);
    obj->end_addr = temp_area->end_addr;

    return 0;
}


//correctly remove unmapped obj node
static inline int _unlink_unmapped_obj(cm_list_node * node, 
                                       _traverse_state * state, ln_vm_map * vm_map) {

    int ret;
    cm_list_node * ret_node;

    //unlink this node from the list of mapped vm areas
    ret = cm_list_unlink(&vm_map->vm_objs, state->prev_obj_index + 1);
    if (ret) {
        ln_errno = LN_ERR_LIBCMORE;
        return -1;
    }

    //set node's values to be unmapped
    (LN_GET_NODE_OBJ(node))->mapped = false;
    node->next = NULL;
    node->prev = NULL;

    //add a pointer to this node to the list containing nodes to dealloc later
    ret_node = cm_list_append(&vm_map->vm_objs_unmapped, (cm_byte *) &node);
    if (ret_node == NULL) {
        ln_errno = LN_ERR_LIBCMORE;
        return -1;
    }

    return 0;
}


//correctly remove unmapped area node
static inline int _unlink_unmapped_area(cm_list_node * node, _traverse_state * state, 
                                        ln_vm_map * vm_map) {

    int ret, index;
    
    cm_list_node * obj_node, * temp_node;
    
    ln_vm_area * temp_area;
    ln_vm_obj * temp_obj;


    //get vm area of node
    temp_area = LN_GET_NODE_AREA(node);

    
    //remove this area from its parent object if necessary
    if (temp_area->obj_node_ptr != NULL) {
        
        obj_node = temp_area->obj_node_ptr;
        temp_obj = LN_GET_NODE_OBJ(obj_node);

        //find the index for this area in the obj
        index = _get_area_index(temp_area, temp_obj);
        if (index != -1) {
            ret = cm_list_remove(&temp_obj->vm_area_node_ptrs, index);
            if (ret) {
                ln_errno = LN_ERR_LIBCMORE;
                return -1;
            }
        } else {
            ln_errno = LN_ERR_INTERNAL_INDEX;
            return -1;
        }

        //if the object now has no areas, set it to be unmapped
        if (temp_obj->vm_area_node_ptrs.len == 0) {
            
            ret = _unlink_unmapped_obj(obj_node, state, vm_map);
            if (ret) return -1;
        
        //else set the new start and end addr for this object
        } else {
    ret = _update_obj_addr_range(temp_obj);
            if (ret) return -1;
        }

    } //end if


    //unlink this node from the list of mapped vm areas
    ret = cm_list_unlink(&vm_map->vm_areas, state->next_area_index);
    if (ret) {
        ln_errno = LN_ERR_LIBCMORE;
        return -1;
    }

    //now safe to set node's values to be unmapped
    temp_area->mapped = false;
    node->next = NULL;
    node->prev = NULL;

    //add a pointer to this node to the list containing nodes to dealloc later
    temp_node = cm_list_append(&vm_map->vm_areas_unmapped, (cm_byte *) &node);
    if (temp_node == NULL) {
        ln_errno = LN_ERR_LIBCMORE;
        return -1;
    }

    return 0;
}


//check if the new vm_area is the same as the old vm_area
static inline int _check_area_eql(struct vm_entry * entry, cm_list_node * area_node) {

    ln_vm_area * area = LN_GET_NODE_AREA(area_node);

    //check misc
    if ((entry->vm_start != area->start_addr) 
        || (entry->vm_end != area->end_addr)) return -1;
    if ((entry->prot & VM_PROT_MASK) != area->access) return -1;
    
    //check pathname
    if (entry->file_path[0] == '\0' && area->pathname == NULL) return 0;
    if (((uintptr_t) entry->file_path[0] == '\0') 
        || (area->pathname == NULL)) return -1;
    if (strncmp(entry->file_path, area->pathname, PATH_MAX)) return -1;

    return 0;
}


//remove old entries and get in sync again
static inline int _resync_area(ln_vm_map * vm_map,
                              _traverse_state * state, struct vm_entry * entry) {

    int ret;

    ln_vm_area * iter_area;   
    cm_list_node * iter_node;

    iter_node = state->next_area;
    iter_area = LN_GET_NODE_AREA(iter_node);


    //while there are vm areas left to discard
    while (entry->vm_end > iter_area->start_addr) {

        //make state point to the next node, and NULL if there is no next node
        if (state->next_area->next == NULL) {
            state->next_area = NULL;
        } else {
            state->next_area = state->next_area->next;
        }

        //correctly handle removing this area node
        ret = _unlink_unmapped_area(iter_node, state, vm_map);
        if (ret) return -1;

        //update iterator nodes
        if (state->next_area == NULL) break;
        iter_node = state->next_area;
        iter_area = LN_GET_NODE_AREA(iter_node);

    } //end while

    return 0;
}


#define _STATE_AREA_NODE_KEEP     0
#define _STATE_AREA_NODE_ADVANCE  1
#define _STATE_AREA_NODE_REASSIGN 2
//move area state forward
static void _state_inc_area(ln_vm_map * vm_map, _traverse_state * state, 
                            cm_list_node * assign_node, int inc_type) {

    switch (inc_type) {

        case _STATE_AREA_NODE_KEEP:
            break;

        case _STATE_AREA_NODE_ADVANCE:    
            //advance next area if we haven't reached end & dont circle back to start
            if (state->next_area != NULL 
                && state->next_area_index < vm_map->vm_areas.len) {
                
                state->next_area = state->next_area->next;
            
            } else {
                state->next_area = NULL;
            }
            break;

        case _STATE_AREA_NODE_REASSIGN:
            state->next_area = assign_node;
            break;

    } //end switch

    //always increment state index
    ++state->next_area_index;

    return;
}


//move obj state forward
static void _state_inc_obj(ln_vm_map * vm_map, _traverse_state * state) {

    //if there is no prev obj, initialise it
    if (state->prev_obj == NULL) {

        state->prev_obj = vm_map->vm_objs.head;
        state->prev_obj_index = 0;

    //if there is a prev obj
    } else {

        //only advance next object if we won't circle back to start
        if (state->prev_obj_index < vm_map->vm_objs.len) {;

            state->prev_obj = state->prev_obj->next;
            ++state->prev_obj_index;
        }
    }

    return;
}


//add a new obj to the map
static cm_list_node * _map_add_obj(ln_vm_map * vm_map, 
                                  _traverse_state * state, struct vm_entry * entry) {

    int index;

    ln_vm_obj vm_obj;
    cm_list_node * obj_node;

    //create new object
    _new_vm_obj(&vm_obj, vm_map, entry->file_path);

    //figure out which insertion index to use
    index = (vm_map->vm_objs.len == 0) ? 0 : state->prev_obj_index + 1;

    //insert obj into map at state's index
    obj_node = cm_list_insert(&vm_map->vm_objs, 
                              index, (cm_byte *) &vm_obj);
    if (obj_node == NULL) {
        ln_errno = LN_ERR_LIBCMORE;
        return NULL;
    }

    //advance state
    _state_inc_obj(vm_map, state);

    return obj_node;
}


//add a new area to the map
static int _map_add_area(ln_vm_map * vm_map, _traverse_state * state, 
                        struct vm_entry * entry, int inc_type) {

    int ret;
    bool use_obj = false;

    ln_vm_area vm_area;
    ln_vm_obj * vm_obj;

    cm_list_node * area_node;
    cm_list_node * obj_node;

    //if no obj for this area
    if (entry->file_path[0] != '\0') use_obj = true;

    if (!use_obj) {

        /*
         *  It should never be possible for prev_obj to point at/ahead of this vm_area
         */
        _new_vm_area(&vm_area, vm_map, NULL, state->prev_obj, entry);

    //else there is an obj for this area
    } else {

        ret = _find_obj_for_area(state, entry);
        
        switch (ret) {

            //area belongs to previous obj
            case _MAP_OBJ_PREV:
                _new_vm_area(&vm_area, vm_map, state->prev_obj, NULL, entry);
                break;

            //area is part of a new object
            case _MAP_OBJ_NEW:
                obj_node = _map_add_obj(vm_map, state, entry);
                if (obj_node == NULL) return -1;
                _new_vm_area(&vm_area, vm_map, state->prev_obj, NULL, entry);
                break;

            //area belongs to the next obj
            case _MAP_OBJ_NEXT:
                _state_inc_obj(vm_map, state);
                _new_vm_area(&vm_area, vm_map, state->prev_obj, NULL, entry);
                break;

        } //end switch
    }

    //add area to the map list
    area_node = cm_list_insert(&vm_map->vm_areas, 
                               state->next_area_index, (cm_byte *) &vm_area);
    if (area_node == NULL) {
        ln_errno = LN_ERR_LIBCMORE;
        return -1;
    }

    //add area to the obj ptr list
    if (use_obj) {
        vm_obj = LN_GET_NODE_OBJ(state->prev_obj);
        ret = _obj_add_area(vm_obj, area_node);
        if (ret == -1) return -1;
    }

    //increment area state
    _state_inc_area(vm_map, state, area_node, inc_type);

    return 0;
}



// --- CALLED BY MEMORY INTERFACES

//send information about a vm area from an interface to a map
int _map_send_entry(ln_vm_map * vm_map, 
                    _traverse_state * state, struct vm_entry * entry) {

    int ret;


    //if at end of old map
    if (state->next_area == NULL) {

        ret = _map_add_area(vm_map, state, entry, false);
        if (ret == -1) return -1;

    //if not at end of old map
    } else {
        
        //if entry doesn't match next area (a change in the map)
        if (_check_area_eql(entry, state->next_area)) {

            ret = _resync_area(vm_map, state, entry);
            if (ret) return -1;

            ret = _map_add_area(vm_map, state, entry, false);
            if (ret == -1) return -1;

        // else entry matches next area
        } else {

            _state_inc_area(vm_map, state, NULL, _STATE_AREA_NODE_ADVANCE);

            //check if area belongs to the next obj
            if (_find_obj_for_area(state, entry) == _MAP_OBJ_NEXT) {
                _state_inc_obj(vm_map, state);
            }

        } //end if match

    } //end if map end

    return 0;
}


//initialise traverse state for a map
void _map_init_traverse_state(ln_vm_map * vm_map, _traverse_state * state) {

    state->next_area_index = 0;
    state->prev_obj_index = 0;

    //set up next area node
    state->next_area = vm_map->vm_areas.head;
    state->prev_obj = vm_map->vm_objs.head;

    return;
}



// --- USER INTERFACE

//ln_vm_map constructor
void ln_new_vm_map(ln_vm_map * vm_map) {

    cm_new_list(&vm_map->vm_areas, sizeof(ln_vm_area));
    cm_new_list(&vm_map->vm_objs, sizeof(ln_vm_obj));

    cm_new_list(&vm_map->vm_areas_unmapped, sizeof(cm_list_node *));
    cm_new_list(&vm_map->vm_objs_unmapped, sizeof(cm_list_node *));

    vm_map->next_id_area = vm_map->next_id_obj = 0;

    return;
}


//ln_vm_map destructor
int ln_del_vm_map(ln_vm_map * vm_map) {

    int ret;

    //unallocate all unmapped nodes
    ret = ln_map_clean_unmapped(vm_map);
    if (ret) return -1;


    cm_list_node * iter_node;
    ln_vm_obj * iter_obj;

    int len_obj = vm_map->vm_objs.len;

    
    //manually free all unmapped obj nodes
    for (int i = 0; i < len_obj; ++i) {

        iter_node = cm_list_get_node(&vm_map->vm_objs_unmapped, i);
        if (iter_node == NULL) {
            ln_errno = LN_ERR_LIBCMORE;
            return -1;
        }
        
        iter_obj = LN_GET_NODE_OBJ(iter_node);
        if (iter_obj == NULL) {
            ln_errno = LN_ERR_UNEXPECTED_NULL;
            return -1;
        }

        _del_vm_obj(iter_obj);

    } //end for
   
    cm_del_list(&vm_map->vm_areas);
    cm_del_list(&vm_map->vm_objs);

    return 0;
}


/*
 *  The nodes of 'vm_map-><type>_unmapped' lists hold pointers to other nodes. 
 *  This means to thoroughly 
 */
int ln_map_clean_unmapped(ln_vm_map * vm_map) {

    int ret;

    cm_list_node * iter_node;
    cm_list_node * del_node;

    int len_area = vm_map->vm_areas_unmapped.len;
    int len_obj = vm_map->vm_objs_unmapped.len;


    //manually free all unmapped area nodes
    for (int i = 0; i < len_area; ++i) {

        iter_node = cm_list_get_node(&vm_map->vm_areas_unmapped, i);
        if (iter_node == NULL) {
            ln_errno = LN_ERR_LIBCMORE;
            return -1;
        }

        del_node = *((cm_list_node **) iter_node->data);
        free(del_node->data);
        free(del_node);

    } //end for
    
    //manually free all unmapped obj nodes
    for (int i = 0; i < len_obj; ++i) {

        iter_node = cm_list_get_node(&vm_map->vm_objs_unmapped, i);
        if (iter_node == NULL) {
            ln_errno = LN_ERR_LIBCMORE;
            return -1;
        }

        del_node = *((cm_list_node **) iter_node->data);
        free(del_node->data);
        free(del_node);

    } //end for

    //empty out both unmapped lists
    ret = cm_list_empty(&vm_map->vm_areas_unmapped);
    if (ret) {
        ln_errno = LN_ERR_LIBCMORE;
        return -1;
    }

    ret = cm_list_empty(&vm_map->vm_objs_unmapped);
    if (ret) {
        ln_errno = LN_ERR_LIBCMORE;
        return -1;
    }

    return 0;
}
