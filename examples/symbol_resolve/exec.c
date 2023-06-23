#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>

#include <linux/limits.h>
#include <sys/mman.h>

#include <libpwu.h>


int main() {

	int ret;
	int fd_mem;
	FILE * fd_maps;
    
    int own_fd_mem;
    FILE * own_fd_maps;
    unsigned int matched_offset;
    pid_t own_pid;

	//define uninitialised libpwu structs (see header for details)
	maps_data m_data;
	maps_entry * m_entry;
	name_pid n_pid;

    sym_resolve s_resolve;
    maps_data own_maps;
    maps_entry * matched_region;

	//-----INIT
	//initialise the maps_data struct on the heap
	ret = new_maps_data(&m_data);
	if (ret == -1) return -1;

	//initialise the name_pid struct on the heap
	ret = new_name_pid(&n_pid, "target");
	if (ret == -1) return -1;


	//-----SETUP
	//get pid for the process by its name
	ret = pid_by_name(&n_pid, &p_info.pid);
	if (ret == -1) return -1;

	//open the process's memory and memory maps
	ret = open_memory(p_info.pid, &fd_maps, &fd_mem);
	if (ret == -1) return -1;

	//read the maps
	ret = read_maps(&m_data, fd_maps);
    if (ret == -1) return -1;


    //-----RESOLVE puts() ADDRESS IN TARGET
    //open a handle on libc
    ret = open_lib("libc.so.6", &s_resolve);
    if (ret == -1) return -1;

    //initialise own process maps
    ret = new_maps_data(&own_maps);

    //get own pid
    own_pid = getpid();

	//open own memory and memory maps
	ret = open_memory(own_pid, &own_fd_maps, &own_fd_mem);
	if (ret == -1) return -1;

	//read own memory maps
	ret = read_maps(&own_maps, own_fd_maps);
    if (ret == -1) return -1;

    //put together the symbol resolving struct
    s_resolve.host_m_data = &own_maps;
    s_resolve.target_m_data = &m_data;

    //resolve puts() symbol
    ret = resolve_symbol("puts", s_resolve, &matched_region, &matched_offset);

    printf("The address of puts() in target is: %p\n", 
           matched_region->start_addr + matched_offset);

    //close the handle on libc
    close_lib(&s_resolve);


	//-----CLEANUP
	//delete data structures
	ret = del_name_pid(&n_pid);
	ret = del_maps_data(&m_data);
    ret = del_maps_data(&own_m_data);
}
