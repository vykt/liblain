#ifndef _LIBPWU_H
#define _LIBPWU_H

#include <stdio.h>
#include <linux/limits.h>

#define APPEND_TRUE 1
#define APPEND_FALSE 0

#define PERMS_LEN 4
#define PATTERN_LEN 1024


//byte
typedef char byte;


//vector
typedef struct {
	
	char * vector;
	size_t data_size;
	unsigned long length;

} vector;

//single region in /proc/<pid>/maps
typedef struct {

        char pathname[PATH_MAX];
        char perms[PERMS_LEN];
        void * start_addr;
        void * end_addr;

} maps_entry;

//regions grouped by backing file/type
typedef struct {

        char name[PATH_MAX];
        vector entry_vector; //maps_entry

} maps_obj;

//entire memory map
typedef struct {

        vector obj_vector; //maps_obj

} maps_data;

//pattern to search for
typedef struct {

	maps_entry * search_region;
	byte pattern_bytes[PATTERN_LEN];
	int pattern_len;
	vector instances;


} pattern;


//read /proc/<pid>/maps into allocated maps_data object
extern int read_maps(maps_data * m_data, FILE * maps_stream);

//returns: 0 - success, -1 - failed to allocate space
extern int new_maps_data(maps_data * m_data);

//returns: 0 - success, -1 - failed to deallocate maps_data
extern int del_maps_data(maps_data * m_data);

#endif
