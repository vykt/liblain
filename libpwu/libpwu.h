#ifndef _LIBPWU_H
#define _LIBPWU_H

#include <stdio.h>
#include <stdint.h>

#include <sys/user.h>
#include <sys/types.h>

#include <linux/limits.h>

#define APPEND_TRUE 1
#define APPEND_FALSE 0

#define PATTERN_LEN 1024

#define REL_JUMP_LEN 5


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

		//read_maps()
        char pathname[PATH_MAX];
        byte perms; //4 (read) + 2 (write) + 1 (exec)
        void * start_addr;
        void * end_addr;
		
		//filled by get_caves() call on this specific region
		vector cave_vector; //cave

} maps_entry;

//regions grouped by backing file/type
typedef struct {

        char name[PATH_MAX];
        vector entry_vector; //*maps_entry

} maps_obj;

//entire memory map
typedef struct {

	/*	object vector holds entries sorted by type or backing disk object
	 *
	 *	entry_vector holds entries sorted by 
	 */
        vector obj_vector; //maps_obj
		vector entry_vector; //maps_entry

} maps_data;


//pattern to search for
typedef struct {

	maps_entry * search_region;
	byte pattern_bytes[PATTERN_LEN];
	int pattern_len;
	vector instance_vector;

} pattern;

//cave - unused region of memory, typically zero'ed out
typedef struct {

	unsigned int offset;
	int size;

} cave;

//injection metadata
typedef struct {

	maps_entry * target_region;
	unsigned int offset;

	byte * payload;
	unsigned int payload_size;

} raw_injection;

//relative 32bit jump hook
typedef struct {

	maps_entry * from_region;
	uint32_t from_offset; //address of jump instruction

	maps_entry * to_region;
	uint32_t to_offset; //address of jump instruction

} rel_jump_hook;

//name to find matching PIDs for
typedef struct {

	char name[NAME_MAX];
	vector pid_vector;

} name_pid;

//information about puppet process
typedef struct {

	pid_t pid;

	struct user_regs_struct saved_state;
	struct user_fpregs_struct saved_float_state;

	struct user_regs_struct new_state;
	struct user_fpregs_struct new_float_state;

} puppet_info;


// --- READING PROCESS MEMORY MAPS ---
//read /proc/<pid>/maps into allocated maps_data object
extern int read_maps(maps_data * m_data, FILE * maps_stream);
//returns: 0 - success, -1 - failed to allocate space
extern int new_maps_data(maps_data * m_data);
//returns: 0 - success, -1 - failed to deallocate maps_data
extern int del_maps_data(maps_data * m_data);


// --- INJECTION ---
//returns: number of caves found on success, -1 - failed to search memory
extern int get_caves(maps_entry * m_entry, int fd_mem, int min_size, cave * first_cave);
//returns: 0 - success, -1 - failed to inject
extern int raw_inject(raw_injection r_injection, int fd_mem);
//returns: 0 - success, -1 - fail
extern int new_raw_injection(raw_injection * r_injection, maps_entry * target_region,
                      unsigned int offset, char * payload_filename);
//returns: 0 - success, -1 - fail
extern void del_raw_injection(raw_injection * r_injection);

// --- HOOKING ---
//returns: old relative jump offset on success, 0 on fail to hook
extern uint32_t hook_rj(rel_jump_hook rj_hook, int fd_mem);


// --- SEARCHING FOR PATTERNS IN MEMORY ---
//returns: 0 - success, -1 - failed to allocate object
//search_region can be NULL and be set later
extern int new_pattern(pattern * ptn, maps_entry * search_region, byte * bytes_ptn, int bytes_ptn_len);
//returns: 0 - success, -1 - failed to deallocate object
extern int del_pattern(pattern * ptn);
//returns: n - number of patterns, -1 - failed to search for patterns
extern int match_pattern(pattern * ptn, int fd_mem);


// --- ATTACHING TO PROCESS ---
//returns: 0 - success, -1 - failed to attach
extern int puppet_attach(puppet_info p_info);
//returns: 0 - success, -1 - failed to detach
extern int puppet_detach(puppet_info p_info);
//returns: 0 - success, -1 - failed to save registers
extern int puppet_save_regs(puppet_info * p_info);
//returns: 0 - success, -1 - failed to write registers
extern int puppet_write_regs(puppet_info * p_info);
//returns: 0 - success, -1 - failed to perform operation, -2 - no syscall found
extern int change_region_perms(puppet_info * p_info, byte perms, int fd_mem,
		                       maps_data * m_data, maps_entry * target_region);

// --- FINDING PIDs BY NAME ---
//returns: 0 - success, -1 - failed to allocate object
extern int new_name_pid(name_pid * n_pid, char * name);
//returns: 0 - success, -1 - failed to deallocate object
extern int del_name_pid(name_pid * n_pid);
//returns: number of PIDs with matching name found, -1 on error
extern int pid_by_name(name_pid * n_pid, pid_t * first_pid);


// --- MISCELLANEOUS UTILITIES ---
//convert bytes to hex string
// inp's length = inp_len
// out's length = inp_len * 2
// 0x omitted from beginning of out's string
extern void bytes_to_hex(byte * inp, int inp_len, char * out);
//returns: 0 - success, -1 - couldn't open memory file(s)
extern int open_memory(pid_t pid, FILE ** fd_maps, int * fd_mem);
//returns: 0 - success, -1 - fail
extern int sig_stop(pid_t pid);
//returns: 0 - success, -1 - fail
extern int sig_cont(pid_t pid);


// --- VECTOR OPERATIONS ---
//returns: 0 - success, -1 - fail
extern int vector_get(vector * v, unsigned long pos, byte * data);
//returns: 0 - success, -1 - fail
extern int vector_get_ref(vector * v, unsigned long pos, byte ** data);


// --- MEMORY OPERATIONS ---
//returns: 0 - success, -1 - fail
extern int read_mem(int fd, void * addr, byte * read_buf, int len);
//returns: 0 - success, -1 - fail
extern int write_mem(int fd, void * addr, byte * write_buf, int len);


#endif
