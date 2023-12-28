#include <string.h>
#include <dlfcn.h>

#include <linux/limits.h>

#include "libpwu.h"
#include "resolve.h"


//open a library in own process
int open_lib(char * lib_path, sym_resolve * s_resolve) {

    void * lib_handle;
    lib_handle = dlopen(lib_path, RTLD_NOW);
    if (lib_handle == NULL) return -1;
    s_resolve->lib_handle = lib_handle;
    return 0;
}

//close a library in own process
void close_lib(sym_resolve * s_resolve) {

    dlclose(s_resolve->lib_handle);
}

//get function symbol address
void * get_symbol_addr(char * symbol, sym_resolve s_resolve) {

    void * symbol_addr;
    //symbol_addr = dlsym(s_resolve->lib_handle, symbol);
    symbol_addr = dlsym(s_resolve.lib_handle, symbol);
    return symbol_addr; //NULL on fail
}


//find which memory region a given address resides in
int get_region_by_addr(void * addr, maps_entry ** matched_region, 
                       unsigned int * offset, int * obj_index, maps_data * m_data) {

    int ret;
    int temp_obj_index = 0;
    char pathname_buf[PATH_MAX] = {0};

    maps_entry * temp_entry;

    //for every region
    for (int i = 0; i < m_data->entry_vector.length; ++i) {

        ret = vector_get_ref(&m_data->entry_vector, i, (byte **) &temp_entry);
        if (ret == -1) return -2;

        //increment or reset obj_index
        ret = strcmp(temp_entry->pathname, pathname_buf);
        if (ret != 0) {
            memset(pathname_buf, 0, PATH_MAX);
            strcpy(pathname_buf, temp_entry->pathname);
            temp_obj_index = 0;
        } else {
            temp_obj_index++;
        }

        //if address is in range of the region
        if (addr >= temp_entry->start_addr && addr < temp_entry->end_addr) {
            *matched_region = temp_entry;
            *offset = addr - temp_entry->start_addr;
            if (obj_index != NULL) *obj_index = temp_obj_index;
            return 0;
        }
    }//end for every region

    return -1;
}


//get memory region that matches a pathname and index
int get_region_by_path(char * pathname, int index, maps_data * m_data,
                       maps_entry ** matched_region, maps_obj ** matched_obj) {

    int ret;
    maps_obj * temp_obj;
    maps_entry * temp_entry;

    //for every backing object
    for (int i = 0; i < m_data->obj_vector.length; ++i) {
        
        //get the object
        ret = vector_get_ref(&m_data->obj_vector, i, (byte **) &temp_obj);
        if (ret == -2) return -1;

        //if names match
        ret = strcmp(pathname, temp_obj->name);
        if (ret == 0) {

            ret = vector_get(&temp_obj->entry_vector, index, (byte *) &temp_entry);
            if (ret == -2) return -1;

            *matched_obj = temp_obj;
            *matched_region = temp_entry;
            return 0;
        } //end if names match
    } //end for every backing object

    return -1;
}


//get memory region that matches a pathname and index
int get_region_by_basename(char * basename, int index, maps_data * m_data,
                           maps_entry ** matched_region, maps_obj ** matched_obj) {

    int ret;
    maps_obj * temp_obj;
    maps_entry * temp_entry;

    //for every backing object
    for (int i = 0; i < m_data->obj_vector.length; ++i) {
        
        //get the object
        ret = vector_get_ref(&m_data->obj_vector, i, (byte **) &temp_obj);
        if (ret == -2) return -1;

        //if names match
        ret = strcmp(basename, temp_obj->basename);
        if (ret == 0) {

            ret = vector_get(&temp_obj->entry_vector, index, (byte *) &temp_entry);
            if (ret == -2) return -1;

            *matched_obj = temp_obj;
            *matched_region = temp_entry;
            return 0;
        } //end if names match
    } //end for every backing object

    return -1;
}


int resolve_symbol(char * symbol, sym_resolve s_resolve, 
                   maps_entry ** matched_region, unsigned int * matched_offset) {

    int ret;
    void * temp_addr;
    unsigned int offset;
    int obj_index;
    maps_entry * host_matched_region;
    maps_entry * temp_entry;
    maps_obj * temp_obj;

    //get address of symbol in own process
    temp_addr = get_symbol_addr(symbol, s_resolve);
    if (temp_addr == NULL) return -1;
    
    //using the address, get: region, offset into region, backing file region index
    ret = get_region_by_addr(temp_addr, &host_matched_region, &offset, &obj_index,
                             s_resolve.host_m_data);
    if (ret != 0) return ret;

    //find the corresponding backing file in target process
    ret = get_region_by_path(host_matched_region->pathname, obj_index,
                             s_resolve.target_m_data, &temp_entry, &temp_obj);
    if (ret != 0) return ret;

    *matched_region = temp_entry;
    *matched_offset = offset;

    return 0;
}
