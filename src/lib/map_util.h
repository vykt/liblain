#ifndef MAP_UTIL_H
#define MAP_UTIL_H

#include <libcmore.h>

#include "liblain.h"


//external
off_t ln_get_area_offset(const cm_list_node * area_node, const uintptr_t addr);
off_t ln_get_obj_offset(const cm_list_node * obj_node, const uintptr_t addr);
off_t ln_get_area_offset_bnd(const cm_list_node * area_node, const uintptr_t addr);
off_t ln_get_obj_offset_bnd(const cm_list_node * obj_node, const uintptr_t addr);
cm_list_node * ln_get_vm_area_by_addr(const ln_vm_map * vm_map, 
                                      const uintptr_t addr, off_t * offset);
cm_list_node * ln_get_vm_obj_by_addr(const ln_vm_map * vm_map, 
                                     const uintptr_t addr, off_t * offset);
cm_list_node * ln_get_vm_obj_by_pathname(const ln_vm_map * vm_map, 
                                         const char * pathname);
cm_list_node * ln_get_vm_obj_by_basename(const ln_vm_map * vm_map, 
                                         const char * basename);


#endif
