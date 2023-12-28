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
                       unsigned int * offset, maps_data * m_data) {

    int ret;

    maps_entry * temp_entry;

    //for every region
    for (int i = 0; i < m_data->entry_vector.length; ++i) {

        //get next entry
        ret = vector_get_ref(&m_data->entry_vector, i, (byte **) &temp_entry);
        if (ret == -1) return -2;

        //if address is in range of the region
        if (addr >= temp_entry->start_addr && addr < temp_entry->end_addr) {
            *matched_region = temp_entry;
            *offset = addr - temp_entry->start_addr;
            return 0;
        }
    }//end for every region

    return -1;
}


//get memory region that matches a pathname and index
int get_obj_by_pathname(char * pathname, maps_data * m_data,
                           maps_obj ** matched_obj) {

    int ret;
    maps_obj * temp_obj;

    //for every backing object
    for (int i = 0; i < m_data->obj_vector.length; ++i) {
        
        //get the object
        ret = vector_get_ref(&m_data->obj_vector, i, (byte **) &temp_obj);
        if (ret == -2) return -2;

        //if names match
        ret = strcmp(pathname, temp_obj->basename);
        if (ret == 0) {

            *matched_obj = temp_obj;
            return 0;

        } //end if names match
    } //end for every backing object

    return -1;
}


//get memory object that matches a pathname
int get_obj_by_basename(char * basename, maps_data * m_data, 
                           maps_obj ** matched_obj) {

    int ret;
    maps_obj * temp_obj;

    //for every backing object
    for (int i = 0; i < m_data->obj_vector.length; ++i) {
        
        //get the object
        ret = vector_get_ref(&m_data->obj_vector, i, (byte **) &temp_obj);
        if (ret == -2) return -2;

        //if names match
        ret = strcmp(basename, temp_obj->basename);
        if (ret == 0) {

            *matched_obj = temp_obj;
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
    maps_obj * host_matched_obj;
    maps_entry * target_entry;
    maps_obj * target_obj;

    //get address of symbol in own process
    temp_addr = get_symbol_addr(symbol, s_resolve);
    if (temp_addr == NULL) return -1;

    //using the address, get: region and offset into region
    ret = get_region_by_addr(temp_addr, &host_matched_region, &offset,
                             s_resolve.host_m_data);
    if (ret != 0) return ret;

    //get backing object for this region
    ret = vector_get_ref(&s_resolve.host_m_data->obj_vector, 
                         host_matched_region->obj_vector_index,
                         (byte **) &host_matched_obj);
    if (ret != 0) return -2; //convert from vector_get_ref fail to resolve_symbol fail

    //find the corresponding backing object in target process
    ret = get_obj_by_pathname(host_matched_obj->basename, s_resolve.target_m_data, 
                              &target_obj);
    if (ret != 0) return ret;

    //get the corresponding region in the target process
    ret = vector_get_ref(&s_resolve.target_m_data->obj_vector,
                         target_entry->obj_index,
                         (byte **) & target_entry);
    if (ret != 0) return -2; //convert from vector_get_ref fail to resolve_symbol fail

    //offset in our process and target should be identical
    *matched_region = target_entry;
    *matched_offset = offset;

    return 0;
}
