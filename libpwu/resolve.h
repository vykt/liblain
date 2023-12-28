#ifndef RESOLVE_H
#define RESOLVE_H

#include "libpwu.h"

//external
int open_lib(char * lib_path, sym_resolve * s_resolve);
void close_lib(sym_resolve * s_resolve);
void * get_symbol_addr(char * symbol, sym_resolve s_resolve);
int get_region_by_addr(void * addr, maps_entry ** matched_region,
                       unsigned int * offset, int * obj_index, maps_data * m_data);
int get_region_by_path(char * pathname, int index, maps_data * m_data,
                       maps_entry ** matched_region, maps_obj ** matched_obj);
int get_region_by_basename(char * basename, int index, maps_data * m_data,
                           maps_entry ** matched_region, maps_obj ** matched_obj);
int resolve_symbol(char * symbol, sym_resolve s_resolve,
                   maps_entry ** matched_region, unsigned int * matched_offset);

#endif
