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
	
	byte * vector;
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

//name to find matching PIDs for
typedef struct {

	char name[NAME_MAX];
	vector pids;

} name_pid;



// --- READING PROCESS MEMORY MAPS ---
//read /proc/<pid>/maps into allocated maps_data object
extern int read_maps(maps_data * m_data, FILE * maps_stream);

//returns: 0 - success, -1 - failed to allocate space
extern int new_maps_data(maps_data * m_data);

//returns: 0 - success, -1 - failed to deallocate maps_data
extern int del_maps_data(maps_data * m_data);


// --- SEARCHING FOR PATTERNS IN MEMORY ---
//returns: 0 - success, -1 - failed to allocate object
//search_region can be NULL and be set later
extern int new_pattern(pattern * ptn, maps_entry * search_region, byte * bytes_ptn, int bytes_ptn_len);

//returns: 0 - success, -1 - failed to deallocate object
extern int del_pattern(pattern * ptn);

//returns: n - number of patterns, -1 - failed to search for patterns
extern int match_pattern(pattern * ptn, int fd);


// --- MISCELLANEOUS UTILITIES ---
//convert bytes to hex string
extern void bytes_to_hex(byte * inp, int inp_len, char * out);


// --- VECTOR OPERATIONS ---
//get vector entry data
extern int vector_get(vector * v, unsigned long pos, byte * data);
//get vector entry reference
extern int vector_get_ref(vector * v, unsigned long pos, byte ** data);


// --- MEMORY OPERATIONS ---
//read memory into buffer
extern int read_mem(int fd, void * addr, byte * read_buf, int len);

//write memory at address from write_buf
extern int write_mem(int fd, void * addr, byte * write_buf, int len);


#endif
