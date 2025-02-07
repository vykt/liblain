//standard library
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

//system headers
#include <linux/limits.h>

//external libraries
#include <cmore.h>
#include <time.h>

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
void _map_init_vm_area(mc_vm_area * area, const struct vm_entry * entry,
                       const cm_lst_node * obj_node, 
                       const cm_lst_node * last_obj_node, mc_vm_map * map) {

    mc_vm_obj * obj;

    //set pathname for area if applicable
    if (obj_node != NULL) {

        obj = MC_GET_NODE_OBJ(obj_node);

        area->pathname = obj->pathname;
        area->basename = obj->basename;
    
    } else {
        area->pathname = NULL;
        area->basename = NULL;
        
    } //end if

    area->obj_node_p      = (cm_lst_node *) obj_node;
    area->last_obj_node_p = (cm_lst_node *) last_obj_node;

    area->start_addr = entry->vm_start;
    area->end_addr   = entry->vm_end;

    area->access = (cm_byte) (entry->prot & 0x0000000F);

    area->mapped = true;

    //set id & increment map's next id
    area->id = map->next_id_area;
    ++map->next_id_area;

    return;
}



/*
 *  Note that while the vm_obj constructor does take a vm_map pointer as a  
 *  parameter, the vm_obj is not added to the vm_map in the constructor. Only 
 *  the `next_id_obj` counter is incremented.
 */

DBG_STATIC
void _map_new_vm_obj(mc_vm_obj * obj, 
                     mc_vm_map * map, const char * pathname) {

    const char * basename = mc_pathname_to_basename(pathname);

    strncpy(obj->pathname, pathname, PATH_MAX);
    strncpy(obj->basename, basename, NAME_MAX);

    obj->start_addr = 0x0;
    obj->end_addr = 0x0;

    //initialise area list
    cm_new_lst(&obj->vm_area_node_ps, sizeof(cm_lst_node *));
    cm_new_lst(&obj->last_vm_area_node_ps, sizeof(cm_lst_node *));

    obj->mapped = true;

    //set id & increment map's next id
    obj->id = map->next_id_obj;
    ++map->next_id_obj;

    return;
}



DBG_STATIC
void _map_del_vm_obj(mc_vm_obj * obj) {

    cm_del_lst(&obj->vm_area_node_ps);
    cm_del_lst(&obj->last_vm_area_node_ps);

    return;
}



DBG_STATIC DBG_INLINE
void _map_make_zero_obj(mc_vm_obj * obj) {
    
    obj->id = MC_ZERO_OBJ_ID;

    return;
}



DBG_STATIC
int _map_obj_add_area_insert(cm_lst * obj_area_lst, 
                             const cm_lst_node * area_node) {

    cm_lst_node * ret_node, * iter_node;
    mc_vm_area * area = MC_GET_NODE_AREA(area_node);

    //if list is empty, append
    if (obj_area_lst->len == 0) {
    
        cm_lst_apd(obj_area_lst, &area_node);
    
    //list is not empty, find appropriate insert point
    } else {

        //setup iteration
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
    }

    return 0;
}



DBG_STATIC
cm_lst_node * _map_obj_find_area_outer_node(cm_lst * obj_area_lst, 
                                            cm_lst_node * area_node) {

    cm_lst_node * iter_node;


    //list should never be empty
    if (obj_area_lst->len == 0) {
        
        mc_errno = MC_ERR_AREA_IN_OBJ;
        return NULL;
    }

    //setup iteration
    iter_node = obj_area_lst->head;
    for (int i = 0; i < obj_area_lst->len; ++i) {

        if (area_node == MC_GET_NODE_PTR(iter_node)) return iter_node;
        iter_node = iter_node->next;
    }
    
    mc_errno = MC_ERR_AREA_IN_OBJ;
    return NULL;
}



DBG_STATIC DBG_INLINE
int _map_obj_add_area(mc_vm_obj * obj, 
                      const cm_lst_node * area_node) {

    int ret;
    mc_vm_area * area = MC_GET_NODE_AREA(area_node);


    //if this object has no areas yet
    if (obj->start_addr == MC_UNDEF_ADDR || obj->start_addr == 0x0) {

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



DBG_STATIC DBG_INLINE
int _map_obj_rmv_area(mc_vm_obj * obj, cm_lst_node * area_node) {

    int ret, index;
    cm_lst_node * area_outer_node, * temp_node;


    //remove area node from the object

    //find index of area in object
    area_outer_node = _map_obj_find_area_outer_node(&obj->vm_area_node_ps, 
                                                    area_node);
    if (area_outer_node == NULL) return -1;

    //remove area node from object
    ret = cm_lst_rmv_n(&obj->vm_area_node_ps, area_outer_node);
    if (ret != 0) {
        mc_errno = MC_ERR_LIBCMORE;
        return -1;
    }


    //update object's address range

    //if no area nodes left, address range is now zero (unmapped)
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

    obj->start_addr = MC_GET_NODE_AREA(temp_node)->start_addr;


    //get end addr
    index = obj->vm_area_node_ps.len == 1 ? 0 : -1;

    if (index != 0) {
        ret = cm_lst_get(&obj->vm_area_node_ps, index, &temp_node); 
        if (ret) {
            mc_errno = MC_ERR_LIBCMORE;
            return -1;
        }
    }

    obj->end_addr = MC_GET_NODE_AREA(temp_node)->end_addr;


    return 0;
}



DBG_STATIC DBG_INLINE
int _map_obj_rmv_last_area(mc_vm_obj * obj, cm_lst_node * last_area_node) {

    int ret;
    cm_lst_node * last_area_outer_node;


    //remove area node from the object

    //find index of area in object
    last_area_outer_node = _map_obj_find_area_outer_node(&obj->
                                                         last_vm_area_node_ps, 
                                                         last_area_node);
    if (last_area_outer_node == NULL) return -1;

    //remove area node from object
    ret = cm_lst_rmv_n(&obj->last_vm_area_node_ps, last_area_outer_node);
    if (ret != 0) {
        mc_errno = MC_ERR_LIBCMORE;
        return -1;
    }

    return 0;
}



DBG_STATIC
bool _map_is_pathname_in_obj(const char * pathname, const mc_vm_obj * obj) {

    if (obj == NULL) return false;

    bool ret = (strncmp(pathname, obj->pathname, PATH_MAX) ? false : true);
    
    return ret;
}



DBG_STATIC DBG_INLINE
int _map_find_obj_for_area(const struct vm_entry * entry,
                           const _traverse_state * state) { 

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



//called when deleting an object; moves `last_obj_node_p` back if needed
DBG_STATIC DBG_INLINE
int _map_backtrack_unmapped_obj_last_vm_areas(cm_lst_node * obj_node) {

    int ret;
    int iterations;

    mc_vm_area * last_area;
    cm_lst_node * last_area_node;

    mc_vm_obj * temp_obj;


    //setup iteration
    temp_obj = MC_GET_NODE_OBJ(obj_node);
    last_area_node = temp_obj->last_vm_area_node_ps.head;
    if (last_area_node == NULL) return 0;

    last_area = MC_GET_NODE_AREA(last_area_node);


    //for every area, backtrack last object pointer
    iterations = temp_obj->last_vm_area_node_ps.len;
    for (int i = 0; i < iterations; ++i) {

        //update pointer
        last_area->last_obj_node_p = obj_node->prev;
        
        //advance iteration (part 1)
        last_area_node = last_area_node->next;

        //remove this area from the object's last_vm_area_node_ps list
        ret = cm_lst_rmv(&temp_obj->last_vm_area_node_ps, 0);
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
    mc_vm_obj * obj;
    
    cm_lst_node * prev_obj_node;
    mc_vm_obj * prev_obj;

    cm_lst_node * last_area_node;
    mc_vm_area * last_area;


    //setup iteration
    obj = MC_GET_NODE_OBJ(obj_node);
    
    prev_obj_node = obj_node->prev;
    prev_obj = MC_GET_NODE_OBJ(prev_obj_node);

    last_area_node = prev_obj->last_vm_area_node_ps.head;
    if (last_area_node == NULL) return 0;
    
    last_area = MC_GET_NODE_AREA(last_area_node);


    //for every area, move last object pointer forward if necessary
    for (int i = 0; i < prev_obj->last_vm_area_node_ps.len; ++i) {

        //if this area's address range comes completely after this object
        if (last_area->start_addr >= obj->end_addr) {
            
            //set this object as the new last object pointer
            last_area->last_obj_node_p = obj_node;

            //add this area to this object's last_vm_area_node_ps list
            ret = _map_obj_add_last_area(obj, last_area_node);
            if (ret == -1) return -1;

            //remove this area from the previous 
            //last object's last_vm_area_node_ps
            ret =_map_obj_rmv_last_area(prev_obj, last_area_node);
            if (ret == -1) return -1;

            //correct iteration index
            i -= 1;

        } //end if area's address range comes completely after this object

        //advance iteration
        last_area_node = last_area_node->next;
        last_area = MC_GET_NODE_AREA(last_area_node);

    } //end for

    return 0;
}



DBG_STATIC DBG_INLINE
int _map_unlink_unmapped_obj(cm_lst_node * obj_node, mc_vm_map * map) {

    int ret;
    cm_lst_node * ret_node;
    
    mc_vm_obj * obj;


    //fetch object
    obj = MC_GET_NODE_OBJ(obj_node);
    
    //if this is the pseydo object, just reset it
    if (obj->id == MC_ZERO_OBJ_ID) {
        obj->start_addr = 0x0;
        obj->end_addr = 0x0;
        return 0;
    }

    //correct last_obj_node_p of every vm_area 
    //with this object as its last object
    ret = _map_backtrack_unmapped_obj_last_vm_areas(obj_node);
    if (ret == -1) return -1;

    //unlink this node from the list of mapped vm areas
    ret_node = cm_lst_uln_n(&map->vm_objs, obj_node);
    if (ret_node != obj_node) {
        mc_errno = MC_ERR_LIBCMORE;
        return -1;
    }

    //disconnect object node's attributes
    obj->mapped     = false;
    obj->start_addr = 0x0;
    obj->end_addr   = 0x0;
    obj_node->next = NULL;
    obj_node->prev = NULL;

    //move this object node to the unmapped list
    ret_node = cm_lst_apd(&map->vm_objs_unmapped, &obj_node);
    if (ret_node == NULL) {
        mc_errno = MC_ERR_LIBCMORE;
        return -1;
    }

    return 0;
}



DBG_STATIC DBG_INLINE
int _map_unlink_unmapped_area(cm_lst_node * area_node, mc_vm_map * map) {

    int ret;
    cm_lst_node * ret_node;

    mc_vm_area * area;

    mc_vm_obj * obj;
    cm_lst_node * obj_node;


    //get vm area of node
    area = MC_GET_NODE_AREA(area_node);

    //remove this area from its parent object if necessary
    if (area->obj_node_p != NULL) {
        
        obj_node = area->obj_node_p;
        obj = MC_GET_NODE_OBJ(obj_node);

        //remove this area from the object
        ret = _map_obj_rmv_area(obj, area_node);
        if (ret == -1) return -1;

        //if this area's object now has no areas, unmap it
        if (obj->vm_area_node_ps.len == 0) {
            
            ret = _map_unlink_unmapped_obj(obj_node, map);
            if (ret) return -1;
        }
    }


    //remove this area from its last parent object if necessary
    if (area->last_obj_node_p != NULL) {

        obj_node = area->last_obj_node_p;
        obj = MC_GET_NODE_OBJ(obj_node);

        //remove this area from the last object
        ret = _map_obj_rmv_last_area(obj, area_node);
        if (ret == -1) return -1;
    }
    

    //unlink this node from the list of mapped vm areas
    ret_node = cm_lst_uln_n(&map->vm_areas, area_node);
    if (ret_node != area_node) {
        mc_errno = MC_ERR_LIBCMORE;
        return -1;
    }

    //disconnect area node's attributes
    area->mapped          = false;
    area->obj_node_p      = NULL;
    area->last_obj_node_p = NULL;
    area_node->next = NULL;
    area_node->prev = NULL;

    //move this area node to the unmapped list
    ret_node = cm_lst_apd(&map->vm_areas_unmapped, &area_node);
    if (ret_node == NULL) {
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



DBG_STATIC
void _map_state_inc_area(_traverse_state * state, const int inc_type,
                         const cm_lst_node * assign_node, mc_vm_map * map) {

    switch (inc_type) {

        case _STATE_AREA_NODE_KEEP:
            
            break;

        case _STATE_AREA_NODE_ADVANCE:    
            
            //advance next area if we haven't reached the end 
            //& dont circle back to the start
            if (state->next_area_node != NULL
                && state->next_area_node != map->vm_areas.head) {                
                state->next_area_node = state->next_area_node->next;
            
            } else {
                state->next_area_node = NULL;
            
            }            
            break;

        case _STATE_AREA_NODE_REASSIGN:
            
            state->next_area_node = (cm_lst_node *) assign_node;
            break;

    } //end switch

    return;
}



DBG_STATIC
void _map_state_inc_obj(_traverse_state * state, mc_vm_map * map) {

    //if there is no prev obj, initialise it
    if (state->prev_obj_node == NULL) {

        state->prev_obj_node = map->vm_objs.head;

    //if there is a prev obj
    } else {

        //only advance next object if we won't circle back to start
        if (state->prev_obj_node->next != map->vm_objs.head) {

            state->prev_obj_node = state->prev_obj_node->next;
        }
    }

    return;
}



DBG_STATIC DBG_INLINE
int _map_resync_area(const struct vm_entry * entry, 
                     _traverse_state * state, mc_vm_map * map) {

    int ret;

    mc_vm_area * area;   
    cm_lst_node * area_node;


    //setup iteration
    area_node = state->next_area_node;
    area = MC_GET_NODE_AREA(area_node);

    //while there are vm areas left to discard
    while (entry->vm_end > area->start_addr) {

        //advance state
        _map_state_inc_area(state, _STATE_AREA_NODE_ADVANCE, NULL, map);

        //remove this area node
        ret = _map_unlink_unmapped_area(area_node, map);
        if (ret) return -1;

        //update iterator nodes
        if (state->next_area_node == NULL) break;
        area_node = state->next_area_node;
        area = MC_GET_NODE_AREA(area_node);

    } //end while

    return 0;
}



DBG_STATIC
cm_lst_node * _map_add_obj(const struct vm_entry * entry,
                           _traverse_state * state, mc_vm_map * map) {

    mc_vm_obj vm_obj;
    cm_lst_node * obj_node;


    //create new object
    _map_new_vm_obj(&vm_obj, map, entry->file_path);

    //insert obj into map
    obj_node = cm_lst_ins_na(&map->vm_objs, state->prev_obj_node, &vm_obj);
    if (obj_node == NULL) {
        mc_errno = MC_ERR_LIBCMORE;
        return NULL;
    }

    /*
     *  With the insertion of this object, it may now be closer to some 
     *  memory areas without a backing object. For such memory areas, their 
     *  `last_obj_node_p` pointer must be updated.
     *
     */

    _map_forward_unmapped_obj_last_vm_areas(obj_node);

    //advance state
    _map_state_inc_obj(state, map);

    return obj_node;
}



DBG_STATIC
int _map_add_area(const struct vm_entry * entry,
                  _traverse_state * state, mc_vm_map * map) {

    int ret;
    bool use_obj;

    mc_vm_area area;
    cm_lst_node * area_node;
    
    mc_vm_obj * obj;
    cm_lst_node * obj_node;


    //determine if this area belongs to a backing object
    use_obj = (entry->file_path[0] == '\0') ? false : true;


    //if this area does not belong to a backing object, create a new area
    if (!use_obj) {

        /*
         *  It should never be possible for prev_obj 
         *  to point at/ahead of this vm_area.
         */
        _map_init_vm_area(&area, entry, NULL, state->prev_obj_node, map);


    //else there is a backing object for this area
    } else {

        //determine which of the adjascent backing objects this area belongs to
        ret = _map_find_obj_for_area(entry, state);

        //dispatch case
        switch (ret) {

            //area belongs to the previous object
            case _MAP_OBJ_PREV:
                break;

            //area is the start of a new object
            case _MAP_OBJ_NEW:
                obj_node = _map_add_obj(entry, state, map);
                if (obj_node == NULL) return -1;
                break;

            //area belongs to the next object
            case _MAP_OBJ_NEXT:
                _map_state_inc_obj(state, map);
                break;

        } //end switch

        //initialise the area
        _map_init_vm_area(&area, entry, state->prev_obj_node, NULL, map);
    
    } //end if-else


    //add area to the map list
    area_node = cm_lst_ins_nb(&map->vm_areas, state->next_area_node, &area);
    if (area_node == NULL) {
        mc_errno = MC_ERR_LIBCMORE;
        return -1;
    }

    //add area to the object pointer list
    obj = MC_GET_NODE_OBJ(state->prev_obj_node); 
    if (use_obj) {
        ret = _map_obj_add_area(obj, area_node);
        if (ret == -1) return -1;
        
    } else {
        ret = _map_obj_add_last_area(obj, area_node);
        if (ret == -1) return -1;
    }

    //increment area state
    _map_state_inc_area(state, _STATE_AREA_NODE_REASSIGN, area_node->next, map);

    return 0;
}



/*
 *  --- [INTERFACE] ---
 */

int map_send_entry(const struct vm_entry * entry,
                   _traverse_state * state, mc_vm_map * map) {

    int ret;


    //if reached the end of the old map
    if (state->next_area_node == NULL) {

        ret = _map_add_area(entry, state, map);
        if (ret == -1) return -1;

    //if not reached the end end of the old map
    } else {
        
        //if entry doesn't match next area (a change in the map)
        if (_map_check_area_eql(entry, state->next_area_node)) {

            ret = _map_resync_area(entry, state, map);
            if (ret) return -1;

            ret = _map_add_area(entry, state, map);
            if (ret == -1) return -1;

        // else entry matches next area
        } else {

            _map_state_inc_area(state, 
                                _STATE_AREA_NODE_ADVANCE, NULL, map);

            //check if area belongs to the next obj
            if (_map_find_obj_for_area(entry, state) == _MAP_OBJ_NEXT) {
                _map_state_inc_obj(state, map);
            }

        } //end if match

    } //end if map end

    return 0;
}



void map_init_traverse_state(_traverse_state * state, const mc_vm_map * map) {

    state->next_area_node = map->vm_areas.head;
    state->prev_obj_node = map->vm_objs.head;

    return;
}



/*
 * --- [EXTERNAL] ---
 */

void mc_new_vm_map(mc_vm_map * map) {

    //pseudo object, will adopt leading parentless vm_areas
    mc_vm_obj zero_obj;

    //initialise lists
    cm_new_lst(&map->vm_areas, sizeof(mc_vm_area));
    cm_new_lst(&map->vm_objs, sizeof(mc_vm_obj));

    cm_new_lst(&map->vm_areas_unmapped, sizeof(cm_lst_node *));
    cm_new_lst(&map->vm_objs_unmapped, sizeof(cm_lst_node *));

    //setup pseudo object at start of map
    _map_new_vm_obj(&zero_obj, map, "0x0");
    _map_make_zero_obj(&zero_obj);

    cm_lst_apd(&map->vm_objs, &zero_obj);

    //set next IDs to 0
    map->next_id_area = map->next_id_obj = 0;

    return;
}



int mc_del_vm_map(mc_vm_map * map) {

    int ret, len_obj;

    cm_lst_node * obj_node;
    mc_vm_obj * obj;


    //unallocate all unmapped nodes
    ret = mc_map_clean_unmapped(map);
    if (ret) return -1;


    //setup iteration
    len_obj = map->vm_objs.len;
    obj_node = map->vm_objs.head;
    
    //manually free all unmapped obj nodes
    for (int i = 0; i < len_obj; ++i) {

        //fetch & destroy the object
        obj = MC_GET_NODE_OBJ(obj_node);
        _map_del_vm_obj(obj);

        //advance iteration
        obj_node = obj_node->next;

    } //end for


    //destroy all lists   
    cm_del_lst(&map->vm_areas);
    cm_del_lst(&map->vm_objs);
    cm_del_lst(&map->vm_areas_unmapped);
    cm_del_lst(&map->vm_objs_unmapped);

    return 0;
}


 
int mc_map_clean_unmapped(mc_vm_map * map) {

    int ret, len;

    mc_vm_obj * obj;
    cm_lst_node * node, * del_node;


    //setup unmapped area iteration
    len  = map->vm_areas_unmapped.len;
    node = map->vm_areas_unmapped.head;


    //manually free all unmapped area nodes
    for (int i = 0; i < len; ++i) {

        //delete the unmapped area node
        del_node = MC_GET_NODE_PTR(node);
        cm_del_lst_node(del_node);

        //advance iteration
        node = node->next;

    } //end for


    //setup unmapped object iteration
    len = map->vm_objs_unmapped.len;
    node = map->vm_objs_unmapped.head;
    
    //manually free all unmapped obj nodes
    for (int i = 0; i < len; ++i) {

        //delete the unmapped object and its node
        del_node = MC_GET_NODE_PTR(node);
        obj = MC_GET_NODE_OBJ(del_node);
        
        _map_del_vm_obj(obj);
        cm_del_lst_node(del_node);

        //advance iteration
        node = node->next;

    } //end for


    //empty out both unmapped lists
    ret = cm_lst_emp(&map->vm_areas_unmapped);
    if (ret) {
        mc_errno = MC_ERR_LIBCMORE;
        return -1;
    }

    ret = cm_lst_emp(&map->vm_objs_unmapped);
    if (ret) {
        mc_errno = MC_ERR_LIBCMORE;
        return -1;
    }

    return 0;
}
