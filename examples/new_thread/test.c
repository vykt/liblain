#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>

#include <linux/limits.h>
#include <sys/mman.h>

#include "../libpwu/libpwu.h"


int main() {

	//everything that is required is to inject & hook is here
	char * payload_filename = "payload.o";
	int payload_size = 18;
	unsigned int target_offset = 0x6c7;      //static
    unsigned int thread_work_offset = 0x843; //static
	int region_num = 1;

	int ret;
	int fd_mem;
	int tid;
	FILE * fd_maps;
	uint32_t old_jump_offset;
    void * stack_addr;
    unsigned int stack_size = 0x800000;

	//define uninitialised libpwu structs (see header for details)
	maps_data m_data;
	maps_entry * m_entry;
	name_pid n_pid;
	puppet_info p_info;
	cave cav;
	raw_injection r_injection;
	rel_jump_hook hook;
	new_thread_setup n_t_setup;

	//-----INIT
	//initialise the maps_data struct on the heap
	ret = new_maps_data(&m_data);
	if (ret == -1) return -1;

	//initialise the name_pid struct on the heap
	ret = new_name_pid(&n_pid, "target");
	if (ret == -1) return -1;


	//-----SETUP & PERMISSIONS
	//get pid for the process by its name
	ret = pid_by_name(&n_pid, &p_info.pid);
	if (ret == -1) return -1;

	//open the process's memory and memory maps
	ret = open_memory(p_info.pid, &fd_maps, &fd_mem);
	if (ret == -1) return -1;

	//read the maps
	ret = read_maps(&m_data, fd_maps);
    if (ret == -1) return -1;

    //find syscall instruction for puppeting
    ret = puppet_find_syscall(&p_info, &m_data, fd_mem);
    if (ret == -1) return -1;

	//get the second memory region (check /proc/pid/maps to see which region you need)
	ret = vector_get_ref(&m_data.entry_vector, region_num, (byte **) &m_entry);
	if (ret == -1) return -1;

	//attach to the target process
	ret = puppet_attach(p_info);
	if (ret == -1) return -1;

	//change enable write permissions for the .text region
	ret = change_region_perms(&p_info, 7, fd_mem, m_entry);
	if (ret == -1) return -1;

	//-----INJECTING
	//get caves and make sure there's at least one available
	ret = get_caves(m_entry, fd_mem, 66, &cav); //get caves of size 20+
	if (ret <= 0) return -1;

	//initialise a new raw injection struct and read the payload into memory
	ret = new_raw_injection(&r_injection, m_entry, cav.offset, payload_filename);
	if (ret == -1) return -1;

	//inject the payload into a cave
	ret = raw_inject(r_injection, fd_mem);
	if (ret == -1) return -1;

	//hook call from target to payload
	hook.from_region = m_entry;
	hook.from_offset = target_offset;
	hook.to_region = m_entry;
	hook.to_offset = cav.offset;

	old_jump_offset = hook_rj(hook, fd_mem);
	if (old_jump_offset == 0) return -1;

    //-----THREAD SHENANIGANS
    //allocate some memory for a thread stack
    ret = create_thread_stack(&p_info, fd_mem, &stack_addr, stack_size);
    if (ret == -1) return -1;

	//assign info for thread bootstrapping
	n_t_setup.thread_func_region = m_entry;
	n_t_setup.thread_func_offset = thread_work_offset;
	n_t_setup.setup_region = m_entry;
	//put bootstrap payload immediately after the previously injected payload
	n_t_setup.setup_offset = cav.offset + payload_size;
	n_t_setup.stack_addr = stack_addr;
	n_t_setup.stack_size = stack_size;

    //run the new thread
	ret = start_thread(&p_info, fd_mem, n_t_setup, &tid);
    if (ret == -1) return -1;

	//-----CLEANUP
	//delete injection data
	del_raw_injection(&r_injection);

	//change restore r-x permissions for .text segment
	ret = change_region_perms(&p_info, 5, fd_mem, m_entry);
	if (ret == -1) return -1;

	//detach from the target process
	ret = puppet_detach(p_info);
	if (ret == -1) return -1;

	//delete data structures
	ret = del_name_pid(&n_pid);
	ret = del_maps_data(&m_data);
}
