#ifndef _LIBPWU_H
#define _LIBPWU_H

#include <stdio.h>
#include <stdint.h>

#include <sys/user.h>
#include <sys/types.h>

#include <linux/limits.h>

//generic macros
#define APPEND_TRUE 1
#define APPEND_FALSE 0

#define PATTERN_LEN 1024
#define REL_JUMP_LEN 5

#define COPY_OLD 0
#define COPY_NEW 1

#define APPEND_TRUE 1
#define APPEND_FALSE 0

//payload size macros
#define PAYL_NEW_THREAD 41


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

        //get_caves()
        vector cave_vector; //cave

} maps_entry;

//regions grouped by backing file/type
typedef struct {

        char name[PATH_MAX];
        vector entry_vector; //*maps_entry

} maps_obj;

//entire memory map
typedef struct {

        vector obj_vector;   //maps_obj
        vector entry_vector; //maps_entry

} maps_data;


//pattern to search for
typedef struct {

    maps_entry * search_region;
    byte pattern_bytes[PATTERN_LEN];
    int pattern_len;
    vector offset_vector;

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
    uint32_t to_offset;

} rel_jump_hook;

//name to find matching PIDs for
typedef struct {

    char name[NAME_MAX];
    vector pid_vector; //pid_t

} name_pid;

//information about puppet process
typedef struct {

    pid_t pid;

    void * syscall_addr; //address of a syscall instruction

    struct user_regs_struct saved_state;
    struct user_fpregs_struct saved_float_state;

    struct user_regs_struct new_state;
    struct user_fpregs_struct new_float_state;

} puppet_info;

//payload mutations structure
typedef struct {

	unsigned int offset;
	byte[32] mod;
	int mod_len; //beware of bounds, i trust you!

} mutation;


// --- READING PROCESS MEMORY MAPS ---
//read /proc/<pid>/maps into allocated maps_data object
//returns: 0 - success, -1 - failed to read maps
extern int read_maps(maps_data * m_data, FILE * maps_stream);
//returns: 0 - success, -1 - failed to allocate space
extern int new_maps_data(maps_data * m_data);
//returns: 0 - success, -1 - failed to deallocate maps_data
extern int del_maps_data(maps_data * m_data);


// --- INJECTION ---
//returns: number of caves found on success, -1 - failed to search memory
extern int get_caves(maps_entry * m_entry, int fd_mem, int min_size, cave * first_cave);
//returns: 0 - success, -1 - failed to find big enough cave, -2 fail to search_region
extern int find_cave(maps_entry * m_entry, int fd_mem, int required_size,
	                 cave * matched_cave);
//returns: 0 - success, -1 - failed to inject
extern int raw_inject(raw_injection r_injection_dat, int fd_mem);
//returns: 0 - success, -1 - fail
extern int new_raw_injection(raw_injection * r_injection_dat, maps_entry * target_region,
                             unsigned int offset, char * payload_filename);
//returns: 0 - success, -1 - fail
extern void del_raw_injection(raw_injection * r_injection_dat);

// --- HOOKING ---
//returns: old relative jump offset on success, 0 on fail to hook
extern uint32_t hook_rj(rel_jump_hook rj_hook_dat, int fd_mem);


// --- SEARCHING FOR PATTERNS IN MEMORY ---
//returns: 0 - success, -1 - failed to allocate object
//search_region can be NULL and be set later
extern int new_pattern(pattern * ptn, maps_entry * search_region, byte * bytes_ptn,
                       int bytes_ptn_len);
//returns: 0 - success, -1 - failed to deallocate object
extern int del_pattern(pattern * ptn);
//returns: n - number of patterns, -1 - failed to search for patterns
extern int match_pattern(pattern * ptn, int fd_mem);


// --- ATTACHING TO PROCESS ---
//returns: 0 - success, -1 - failed to attach
extern int puppet_attach(puppet_info p_info);
//returns: 0 - success, -1 - failed to detach
extern int puppet_detach(puppet_info p_info);
//returns: 0 - success, -1 - no syscall found, -2 - failed to perform search
extern int puppet_find_syscall(puppet_info * p_info, maps_data * m_data, int fd_mem);
//returns: 0 - success, -1 - failed to save registers
extern int puppet_save_regs(puppet_info * p_info);
//returns: 0 - success, -1 - failed to write registers
extern int puppet_write_regs(puppet_info * p_info);
//returns: void
extern void puppet_copy_regs(puppet_info * p_info, int mode);

//returns: 0 - success, -1 - failed to execute syscall
extern int arbitrary_syscall(puppet_info * p_info, int fd_mem,
                             unsigned long long * syscall_ret);
//returns: 0 - success, -1 - failed to change region permissions
extern int change_region_perms(puppet_info * p_info, byte perms, int fd_mem,
                               maps_entry * target_region);
//returns: 0 - success, -1 - failed create new thread stack
extern int create_thread_stack(puppet_info * p_info, int fd_mem, void ** stack_addr,
                               unsigned int stack_size);
//returns: 0 - success, -1 - failed to start thread
extern int start_thread(puppet_info * p_info, void * exec_addr, int fd_mem,
                        void * stack_addr, unsigned int stack_size, int * tid);

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


// --- PAYLOAD MUTATIONS
//return 0 on success, -1 on fail
extern int apply_mutations(byte * payload_buffer, vector mutation_vector);


// --- VECTOR OPERATIONS ---
//returns: 0 - success, -1 - fail
int new_vector(vector * v, size_t data_size);
//returns: 0 - success, -1 - fail
int del_vector(vector * v);
//returns: 0 - success, -1 - fail
int vector_add(vector * v, unsigned long pos, byte * data, unsigned short append);
//returns: 0 - success, -1 - fail
int vector_rmv(vector * v, unsigned long pos);
//returns: 0 - success, -1 - fail
extern int vector_get(vector * v, unsigned long pos, byte * data);
//returns: 0 - success, -1 - fail
extern int vector_get_ref(vector * v, unsigned long pos, byte ** data);


// --- MEMORY OPERATIONS ---
//returns: 0 - success, -1 - fail
extern int read_mem(int fd_mem, void * addr, byte * read_buf, int len);
//returns: 0 - success, -1 - fail
extern int write_mem(int fd_mem, void * addr, byte * write_buf, int len);

#endif
