//standard library
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

//system headers
#include <linux/limits.h>

//external libraries
#include <cmore.h>

//local headers
#include "memcry.h"
#include "map.h"
#include "util.h"
#include "debug.h"



/*
 *  --- [INTERNAL]
 */

/*
 *  Note that while the vm_area initialiser function does take a vm_obj node 
 *  as a parameter, the state of this vm_obj is not changed.
 */

DBG_STATIC
void _map_init_vm_area(mc_vm_area * vm_area, mc_vm_map * vm_map, 
                       const cm_lst_node * obj_node, 
                       const cm_lst_node * last_obj_node,
                       const struct vm_entry * entry) {

    mc_vm_obj * vm_obj;

    //set pathname for area if applicable
    if (obj_node != NULL) {

        vm_obj = MC_GET_NODE_OBJ(obj_node);

        vm_area->pathname = vm_obj->pathname;
        vm_area->basename = vm_obj->basename;
    
    } else {
        vm_area->pathname = NULL;
        vm_area->basename = NULL;
        
    } //end if

    vm_area->obj_node_p      = (cm_lst_node *) obj_node;
    vm_area->last_obj_node_p = (cm_lst_node *) last_obj_node;

    vm_area->start_addr = entry->vm_start;
    vm_area->end_addr   = entry->vm_end;

    vm_area->access = (cm_byte) (entry->prot & 0x0000000F);

    vm_area->mapped = true;

    //set id & increment map's next id
    vm_area->id = vm_map->next_id_area;
    ++vm_map->next_id_area;

    return;
}



/*
 *  Note that while the vm_obj constructor does take a vm_map pointer as a  
 *  parameter, the vm_obj is not added to the vm_map in the constructor. Only 
 *  the `next_id_obj` counter is incremented.
 */

DBG_STATIC
void _map_new_vm_obj(mc_vm_obj * vm_obj, 
                     mc_vm_map * vm_map, const char * pathname) {

    const char * basename = mc_pathname_to_basename(pathname);

    strncpy(vm_obj->pathname, pathname, PATH_MAX);
    strncpy(vm_obj->basename, basename, NAME_MAX);

    vm_obj->start_addr = 0x0;
    vm_obj->end_addr = 0x0;

    //initialise area list
    cm_new_lst(&vm_obj->vm_area_node_ps, sizeof(cm_lst_node *));
    cm_new_lst(&vm_obj->last_vm_area_node_ps, sizeof(cm_lst_node *));

    vm_obj->mapped = true;

    //set id & increment map's next id
    vm_obj->id = vm_map->next_id_obj;
    ++vm_map->next_id_obj;

    return;
}



DBG_STATIC
void _map_del_vm_obj(mc_vm_obj * vm_obj) {

    cm_del_lst(&vm_obj->vm_area_node_ps);
    cm_del_lst(&vm_obj->last_vm_area_node_ps);

    return;
}



DBG_STATIC DBG_INLINE
void _map_make_zero_obj(mc_vm_obj * vm_obj) {
    
    vm_obj->id = ZERO_OBJ_ID;

    return;
}



DBG_STATIC
int _map_obj_add_area_insert(cm_lst * obj_area_lst, 
                             const cm_lst_node * area_node) {

    cm_lst_node * ret_node, * iter_node;
    mc_vm_area * area = MC_GET_NODE_AREA(area_node);

    //insert the new area at an appropriate index
    iter_node = obj_area_lst->head;
    for (int i = 0; i < obj_area_lst->len; ++i) {

        //new area ends at a lower address than some existing area
        if (area->end_addr < MC_GET_NODE_AREA(iter_node)->end_addr) {
            
            ret_node = cm_lst_ins_nb(obj_area_lst, iter_node, &area_node);
            break;
        }

        //new area ends later than any existing area
        if ((iter_node->next == NULL) 
            || (iter_node->next == obj_area_lst->head)) {
            
            ret_node = cm_lst_ins_na(obj_area_lst, iter_node, &area_node);
            break;
        }
        
        //increment node iteration        
        iter_node = iter_node->next;
    }
    
    //check the new area was inserted successfully
    if (ret_node == NULL) {
        mc_errno = MC_ERR_LIBCMORE;
        return -1;
    }

    return 0;
}



DBG_STATIC DBG_INLINE
int _map_obj_add_area(mc_vm_obj * obj, 
                      const cm_lst_node * area_node) {

    int ret;
    mc_vm_area * area = MC_GET_NODE_AREA(area_node);

    //if this object has no areas yet
    if (obj->start_addr == -1 || obj->start_addr == 0x0) {

        //set new addr bounds
        obj->start_addr = area->start_addr;
        obj->end_addr = area->end_addr;
        
    //if this object has areas, only update the start and end addr if necessary
    } else {

        //set new addr bounds if necessary
        if (area->start_addr < obj->start_addr) 
            obj->start_addr = area->start_addr;
        if (area->end_addr > obj->end_addr) 
            obj->end_addr = area->end_addr;
    
        
    }

    //insert new area & ensure the area list remains sorted 
    ret = _map_obj_add_area_insert(&obj->vm_area_node_ps, area_node);
    if (ret) return -1; 

    return 0;
}



DBG_STATIC DBG_INLINE
int _map_obj_add_last_area(mc_vm_obj * obj, 
                           const cm_lst_node * last_area_node) {

    int ret;
    
    //insert new last area & ensure the last area list remains sorted
    ret = _map_obj_add_area_insert(&obj->last_vm_area_node_ps, last_area_node);
    if (ret) return -1; 

    return 0;
}



DBG_STATIC
bool _map_is_pathname_in_obj(const char * pathname, const mc_vm_obj * obj) {

    if (obj == NULL) return false;

    bool ret = (strncmp(pathname, obj->pathname, PATH_MAX) ? false : true);
    
    return ret;
}



DBG_STATIC DBG_INLINE
int _map_find_obj_for_area(const _traverse_state * state, 
                           const struct vm_entry * entry) {

    cm_lst_node * prev_node, * next_node;
    mc_vm_obj * prev_obj, * next_obj;

    //check for null
    if (state->prev_obj_node == NULL) return _MAP_OBJ_NEW; 

    //get prev obj
    prev_node = state->prev_obj_node;
    prev_obj = MC_GET_NODE_OBJ(prev_node);

    if (prev_node->next == NULL) {
        next_obj = NULL;
    } else {
        next_node = prev_node->next;
        next_obj = MC_GET_NODE_OBJ(next_node);
    }

    //return appropriate index
    if (_map_is_pathname_in_obj(entry->file_path, prev_obj)) 
        return _MAP_OBJ_PREV;
    if (_map_is_pathname_in_obj(entry->file_path, next_obj)) 
        return _MAP_OBJ_NEXT;

    return _MAP_OBJ_NEW; 
}



DBG_STATIC DBG_INLINE
int _map_update_obj_addr_range(mc_vm_obj * obj) {

    int ret, end_index;

    cm_lst_node * temp_node;
    mc_vm_area * temp_area;

    if (obj->vm_area_node_ps.len == 0) {
        
        obj->end_addr = obj->start_addr = 0x0;   
        return 0;
    }

    //get start addr
    ret = cm_lst_get(&obj->vm_area_node_ps, 0, &temp_node);
    if (ret) {
        mc_errno = MC_ERR_LIBCMORE;
        return -1;
    }

    temp_area = MC_GET_NODE_AREA(temp_node);
    obj->start_addr = temp_area->start_addr;

    //get end addr
    end_index = obj->vm_area_node_ps.len == 1 ? 0 : -1;
    
    ret = cm_lst_get(&obj->vm_area_node_ps, end_index, &temp_node); 
    if (ret) {
        mc_errno = MC_ERR_LIBCMORE;
        return -1;
    }

    temp_area = MC_GET_NODE_AREA(temp_node);
    obj->end_addr = temp_area->end_addr;

    return 0;
}



//called when deleting an object; moves `last_obj_node_p` back if needed
DBG_STATIC DBG_INLINE
int _map_backtrack_unmapped_obj_last_vm_areas(cm_lst_node * obj_node) {

    int ret;
    int iterations;

    mc_vm_obj * temp_obj;

    cm_lst_node * last_area_node;
    mc_vm_area * last_area;


    //setup iteration
    temp_obj = MC_GET_NODE_OBJ(node);
    last_area_node = temp_obj->last_vm_area_node_ps.head;
    if (last_area_node == NULL) return 0;

    last_area = MC_GET_NODE_AREA(last_area_node);


    //for every area, backtrack last object pointer
    iterations = temp_obj->last_vm_area_node_ps.len;
    for (int i = 0; i < iterations; ++i) {

        //update pointer
        last_area->last_obj_node_p = node->prev;
        
        //advance iteration (part 1)
        last_area_node = last_area_node->next;

        //remove this area from the object's last_vm_area_node_ps list
        ret = cm_lst_rem(&temp_obj->last_vm_area_node_ps, 0);
        if (ret == -1) {
            mc_errno = MC_ERR_LIBCMORE;
            return -1;
        }

        //advance iteration (part 2)
        last_area = MC_GET_NODE_AREA(last_area_node);
    }

    return 0;
}



//called when adding an object; moves `last_obj_node_p` forward if needed
DBG_STATIC DBG_INLINE
int _map_forward_unmapped_obj_last_vm_areas(cm_lst_node * obj_node) {

    int ret;

    //declarations
    cm_lst_node * prev_node;
    mc_vm_obj * temp_obj;
    mc_vm_obj * temp_prev_obj;

    cm_lst_node * last_area_node;
    mc_vm_area * last_area;


    /*
     *  This is not that complicated, C syntax for generics is just awful
     */

    //setup iteration
    prev_node = node->prev;
    
    temp_obj = MC_GET_NODE_OBJ(node);
    temp_prev_obj = MC_GET_NODE_OBJ(prev_node);

    last_area_node = temp_prev_obj->last_vm_area_node_ps.head;
    if (last_area_node == NULL) return 0;
    
    last_area = MC_GET_NODE_AREA(last_area_node);


    //for every area, move last object pointer forward if necessary
    for (int i = 0; i < temp_prev_obj->last_vm_area_node_ps.len; ++i) {

        //advance iteration (part 1)
        last_area_node = last_area_node->next;

        //if this area's address range comes completely after this object
        if (last_area->start_addr >= temp_obj->end_addr) {
            
            //set this object as the new last object pointer
            last_area->last_obj_node_p = node;

            //add this area to this object's last_vm_area_node_ps list
            cm_lst_apd(&temp_obj->last_vm_area_node_ps,
                           &last_area_node);

            //remove this area from the previous 
            //last object's last_vm_area_node_ps
            ret = cm_lst_rem(&temp_prev_obj->last_vm_area_node_ps, i);
            if (ret == -1) {
                mc_errno = MC_ERR_LIBCMORE;
                return -1;
            }

            i -= 1;

        } //end if

        //advance iteration (part 2)
        last_area = MC_GET_NODE_AREA(last_area_node);

    } //end for

    return 0;
}



DBG_STATIC DBG_INLINE
int _map_unlink_unmapped_obj(cm_lst_node * node, 
                             const _traverse_state * state, 
                             mc_vm_map * vm_map) {

    int ret;
    cm_lst_node * ret_node;
    mc_vm_obj * temp_obj;


    //if this is the pseudo object, just reset it
    temp_obj = MC_GET_NODE_OBJ(node);
    if (temp_obj->id == ZERO_OBJ_ID) {
        temp_obj->start_addr = 0x0;
        temp_obj->end_addr = 0x0;
        return 0;
    }

    //correct last_obj_node_p of every vm_area 
    //using this object as its last object
    ret = _backtrack_unmapped_obj_last_vm_areas(node);
    if (ret == -1) return -1;

    //unlink this node from the list of mapped vm areas
    ret = cm_lst_uln(&vm_map->vm_objs, state->prev_obj_index + 1);
    if (ret) {
        mc_errno = MC_ERR_LIBCMORE;
        return -1;
    }

    //set node's values to be unmapped
    (MC_GET_NODE_OBJ(node))->mapped = false;
    node->next = NULL;
    node->prev = NULL;

    //add a pointer to this node to the list containing nodes to dealloc later
    ret_node = cm_lst_apd(&vm_map->vm_objs_unmapped, &node);
    if (ret_node == NULL) {
        mc_errno = MC_ERR_LIBCMORE;
        return -1;
    }

    return 0;
}



DBG_STATIC DBG_INLINE
int _map_unlink_unmapped_area(cm_lst_node * node, 
                              const _traverse_state * state, 
                              mc_vm_map * vm_map) {

    int ret, index;
    
    cm_lst_node * obj_node, * temp_node;
    
    mc_vm_area * temp_area;
    mc_vm_obj * temp_obj;


    //get vm area of node
    temp_area = MC_GET_NODE_AREA(node);

    
    //remove this area from its parent object if necessary
    if (temp_area->obj_node_p != NULL) {
        
        obj_node = temp_area->obj_node_p;
        temp_obj = MC_GET_NODE_OBJ(obj_node);

        //find the index for this area in the obj
        index = _get_area_index(temp_area, temp_obj);
        if (index != -1) {
            ret = cm_lst_rem(&temp_obj->vm_area_node_ps, index);
            if (ret) {
                mc_errno = MC_ERR_LIBCMORE;
                return -1;
            }
        } else {
            mc_errno = MC_ERR_INTERNAL_INDEX;
            return -1;
        }

        //if the object now has no areas, set it to be unmapped
        if (temp_obj->vm_area_node_ps.len == 0) {
            
            ret = _unlink_unmapped_obj(obj_node, state, vm_map);
            if (ret) return -1;
        
        //else set the new start and end addr for this object
        } else {
            ret = _update_obj_addr_range(temp_obj);
            if (ret) return -1;
        }

    } //end if


    //unlink this node from the list of mapped vm areas
    ret = cm_lst_uln(&vm_map->vm_areas, state->next_area_index);
    if (ret) {
        mc_errno = MC_ERR_LIBCMORE;
        return -1;
    }

    //now safe to set node's values to be unmapped
    temp_area->mapped = false;
    node->next = NULL;
    node->prev = NULL;

    //add a pointer to this node to the list containing nodes to dealloc later
    temp_node = cm_lst_apd(&vm_map->vm_areas_unmapped, &node);
    if (temp_node == NULL) {
        mc_errno = MC_ERR_LIBCMORE;
        return -1;
    }

    return 0;
}



DBG_STATIC DBG_INLINE
int _map_check_area_eql(const struct vm_entry * entry, 
                        const cm_lst_node * area_node) {

    mc_vm_area * area = MC_GET_NODE_AREA(area_node);

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



DBG_STATIC DBG_INLINE
int _map_resync_area(mc_vm_map * vm_map, _traverse_state * state, 
                     const struct vm_entry * entry) {

    int ret;

    mc_vm_area * iter_area;   
    cm_lst_node * iter_node;

    iter_node = state->next_area;
    iter_area = MC_GET_NODE_AREA(iter_node);


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
        iter_area = MC_GET_NODE_AREA(iter_node);

    } //end while

    return 0;
}



#define _STATE_AREA_NODE_KEEP     0
#define _STATE_AREA_NODE_ADVANCE  1
#define _STATE_AREA_NODE_REASSIGN 2

DBG_STATIC
void _map_state_inc_area(mc_vm_map * vm_map, _traverse_state * state, 
                         const cm_lst_node * assign_node, const int inc_type) {

    switch (inc_type) {

        case _STATE_AREA_NODE_KEEP:
            break;

        case _STATE_AREA_NODE_ADVANCE:    
            //advance next area if we haven't reached the end 
            //& dont circle back to the start
            if (state->next_area != NULL 
                && state->next_area_index < vm_map->vm_areas.len) {
                
                state->next_area = state->next_area->next;
            
            } else {
                state->next_area = NULL;
            }
            break;

        case _STATE_AREA_NODE_REASSIGN:
            state->next_area = (cm_lst_node *) assign_node;
            break;

    } //end switch

    //always increment state index
    ++state->next_area_index;

    return;
}



DBG_STATIC
void _map_state_inc_obj(mc_vm_map * vm_map, _traverse_state * state) {

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



DBG_STATIC
cm_lst_node * _map_add_obj(mc_vm_map * vm_map, _traverse_state * state, 
                            const struct vm_entry * entry) {

    int index;

    mc_vm_obj vm_obj;
    cm_lst_node * obj_node;

    //create new object
    _new_vm_obj(&vm_obj, vm_map, entry->file_path);

    //figure out which insertion index to use
    index = (vm_map->vm_objs.len == 0) ? 0 : state->prev_obj_index + 1;

    //insert obj into map at state's index
    obj_node = cm_lst_ins(&vm_map->vm_objs, index, &vm_obj);
    if (obj_node == NULL) {
        mc_errno = MC_ERR_LIBCMORE;
        return NULL;
    }

    /*
     *  With the insertion of this object, vm_areas in the previous object's 
     *  last_vm_area_node_ps list may now incorrectly treat the previous object
     *  as the last object, when in fact this newly inserted object should be 
     *  their new last object. This needs to now be corrected.
     */

    _forward_unmapped_obj_last_vm_areas(obj_node);

    //advance state
    _state_inc_obj(vm_map, state);

    return obj_node;
}



DBG_STATIC
int _map_add_area(mc_vm_map * vm_map, _traverse_state * state, 
                  const struct vm_entry * entry, const int inc_type) {

    int ret;
    bool use_obj = false;

    mc_vm_area vm_area;
    mc_vm_obj * vm_obj;

    cm_lst_node * area_node;
    cm_lst_node * obj_node;

    //if no obj for this area
    if (entry->file_path[0] != '\0') use_obj = true;

    if (!use_obj) {

        /*
         *  It should never be possible for prev_obj 
         *  to point at/ahead of this vm_area.
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
    area_node = cm_lst_ins(&vm_map->vm_areas, state->next_area_index, &vm_area);
    if (area_node == NULL) {
        mc_errno = MC_ERR_LIBCMORE;
        return -1;
    }

    //add area to the object pointer list
    vm_obj = MC_GET_NODE_OBJ(state->prev_obj); 
    if (use_obj) {
        ret = _obj_add_area(vm_obj, area_node);
        if (ret == -1) return -1;
    } else {
        ret = _obj_add_last_area(vm_obj, area_node);
        if (ret == -1) return -1;
    }

    //increment area state
    _state_inc_area(vm_map, state, area_node, inc_type);

    return 0;
}



/*
 *  --- [INTERFACE] ---
 */

int map_send_entry(mc_vm_map * vm_map, 
                   _traverse_state * state, const struct vm_entry * entry) {

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



void map_init_traverse_state(const mc_vm_map * vm_map, _traverse_state * state) {

    //set up next area node
    state->next_area = vm_map->vm_areas.head;
    state->prev_obj = vm_map->vm_objs.head;

    return;
}



/*
 * --- [EXTERNAL] ---
 */

void mc_new_vm_map(mc_vm_map * vm_map) {

    //pseudo object, will adopt leading parentless vm_areas
    mc_vm_obj zero_obj;

    //initialise lists
    cm_new_lst(&vm_map->vm_areas, sizeof(mc_vm_area));
    cm_new_lst(&vm_map->vm_objs, sizeof(mc_vm_obj));

    cm_new_lst(&vm_map->vm_areas_unmapped, sizeof(cm_lst_node *));
    cm_new_lst(&vm_map->vm_objs_unmapped, sizeof(cm_lst_node *));

    //setup pseudo object at start of map
    _map_new_vm_obj(&zero_obj, vm_map, "0x0");
    _map_make_zero_obj(&zero_obj);

    cm_lst_apd(&vm_map->vm_objs, &zero_obj);

    //reset next object id back to zero
    vm_map->next_id_area = vm_map->next_id_obj = 0;

    return;
}



int mc_del_vm_map(mc_vm_map * vm_map) {

    int ret;

    //unallocate all unmapped nodes
    ret = mc_map_clean_unmapped(vm_map);
    if (ret) return -1;


    cm_lst_node * iter_node;
    mc_vm_obj * iter_obj;

    int len_obj = vm_map->vm_objs.len;

    
    //manually free all unmapped obj nodes
    for (int i = 0; i < len_obj; ++i) {

        iter_node = cm_lst_get_n(&vm_map->vm_objs_unmapped, i);
        if (iter_node == NULL) {
            mc_errno = MC_ERR_LIBCMORE;
            return -1;
        }
        
        iter_obj = MC_GET_NODE_OBJ(iter_node);
        if (iter_obj == NULL) {
            mc_errno = MC_ERR_UNEXPECTED_NULL;
            return -1;
        }

        _del_vm_obj(iter_obj);

    } //end for
   
    cm_del_lst(&vm_map->vm_areas);
    cm_del_lst(&vm_map->vm_objs);

    return 0;
}



/*
 *  The nodes of 'vm_map->vm_{areas,objs}_unmapped' lists hold pointers 
 *  to other nodes. These nodes must be freed as appropriate.
 */
 
int mc_map_clean_unmapped(mc_vm_map * vm_map) {

    int ret;

    cm_lst_node * iter_node;
    cm_lst_node * del_node;

    int len_area = vm_map->vm_areas_unmapped.len;
    int len_obj = vm_map->vm_objs_unmapped.len;


    //manually free all unmapped area nodes
    for (int i = 0; i < len_area; ++i) {

        iter_node = cm_lst_get_n(&vm_map->vm_areas_unmapped, i);
        if (iter_node == NULL) {
            mc_errno = MC_ERR_LIBCMORE;
            return -1;
        }

        del_node = *((cm_lst_node **) iter_node->data);
        free(del_node->data);
        free(del_node);

    } //end for
    
    //manually free all unmapped obj nodes
    for (int i = 0; i < len_obj; ++i) {

        iter_node = cm_lst_get_n(&vm_map->vm_objs_unmapped, i);
        if (iter_node == NULL) {
            mc_errno = MC_ERR_LIBCMORE;
            return -1;
        }

        del_node = *((cm_lst_node **) iter_node->data);
        free(del_node->data);
        free(del_node);

    } //end for

    //empty out both unmapped lists
    ret = cm_lst_emp(&vm_map->vm_areas_unmapped);
    if (ret) {
        mc_errno = MC_ERR_LIBCMORE;
        return -1;
    }

    ret = cm_lst_emp(&vm_map->vm_objs_unmapped);
    if (ret) {
        mc_errno = MC_ERR_LIBCMORE;
        return -1;
    }

    return 0;
}
