#ifndef MAP_UTIL_H
#define MAP_UTIL_H

#include <libcmore.h>

#include "liblain.h"


//external
off_t ln_get_area_offset(cm_list_node * area_node, uintptr_t addr);
off_t ln_get_obj_offset(cm_list_node * obj_node, uintptr_t addr);
cm_list_node * ln_get_vm_area_by_addr(ln_vm_map * vm_map, 
                                   uintptr_t addr, off_t * offset);
cm_list_node * ln_get_vm_obj_by_addr(ln_vm_map * vm_map, 
                                   uintptr_t addr, off_t * offset);
cm_list_node * ln_get_vm_obj_by_pathname(ln_vm_map * vm_map, char * pathname);
cm_list_node * ln_get_vm_obj_by_basename(ln_vm_map * vm_map, char * basename);


#endif
