#ifndef MAP_UTIL_H
#define MAP_UTIL_H

//external libraries
#include <cmore.h>

//local headers
#include "memcry.h"


#ifdef DEBUG
//internal
bool _is_map_empty(const mc_vm_map * vm_map);
cm_lst_node * _get_starting_obj(const mc_vm_map * vm_map);
cm_lst_node * _get_obj_last_area(const mc_vm_obj * vm_obj);
cm_lst_node * _fast_addr_find(const mc_vm_map * vm_map, 
                               const uintptr_t addr, const int mode);
cm_lst_node * _obj_name_find(const mc_vm_map * vm_map, 
                              const char * name, const int mode);
#endif


//external
off_t mc_get_area_offset(const cm_lst_node * area_node,
                         const uintptr_t addr);
off_t mc_get_obj_offset(const cm_lst_node * obj_node,
                        const uintptr_t addr);
off_t mc_get_area_offset_bnd(const cm_lst_node * area_node, 
                             const uintptr_t addr);
off_t mc_get_obj_offset_bnd(const cm_lst_node * obj_node, 
                            const uintptr_t addr);

cm_lst_node * mc_get_area_node_by_addr(const mc_vm_map * vm_map, 
                                       const uintptr_t addr,
                                       off_t * offset);
cm_lst_node * mc_get_obj_node_by_addr(const mc_vm_map * vm_map, 
                                      const uintptr_t addr,
                                      off_t * offset);

cm_lst_node * mc_get_obj_node_by_pathname(const mc_vm_map * vm_map, 
                                          const char * pathname);
cm_lst_node * mc_get_obj_node_by_basename(const mc_vm_map * vm_map, 
                                          const char * basename);


#endif
