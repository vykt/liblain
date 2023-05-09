#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>

#include <linux/limits.h>

//DEBUG TODO
#include <sys/mman.h>

#include "../libpwu/libpwu.h"

int main() {

	printf("temp test\n");

	/*
	int ret;
	int cave_num;
	int fd_mem;
	FILE * fd_maps;

	maps_data m_data;
	maps_entry * m_entry;

	name_pid n_pid;
	puppet_info p_info;
	cave cav;

	//INIT
	ret = new_maps_data(&m_data);
	if (ret == -1) return -1;

	ret = new_name_pid(&n_pid, "target");
	if (ret == -1) return -1;


	//BODY
	//get pid for the process
	ret = pid_by_name(&n_pid, &p_info.pid);
	if (ret == -1) return -1;

	//open the process's memory and memory maps
	ret = open_memory(p_info.pid, &fd_maps, &fd_mem);
	if (ret == -1) return -1;

	//read the maps
	ret = read_maps(&m_data, fd_maps);

	//get the second memory region (it's known here that it is the right one)
	ret = vector_get_ref(&m_data.entry_vector, 1, (byte **) &m_entry);
	if (ret == -1) return -1;

	//attach to the target process
	ret = puppet_attach(p_info);
	if (ret == -1) return -1;

	//change enable write permissions for .text segment
	//ret = change_region_perms(&p_info, 7, fd_mem, &m_data, m_entry);
	//if (ret == -1) return -1;

	//TODO new test

	cave_num = get_caves(m_entry, fd_mem, 50);
	if (cave_num == -1) return -1;

	//for every acquired cave, print address
	for (int i = 0; i < cave_num; ++i) {

		ret = vector_get(&m_entry->cave_vector, i, (byte *) &cav);
		if (ret == -1) return -1;

		printf("cave at: %p\tsize: %lx\n", cav.addr, cav.size);

	}
	
	//TODO end new test

	//CLEANUP
	//detach from the target process
	ret = puppet_detach(p_info);
	if (ret == -1) return -1;

	//delete data structures
	ret = del_name_pid(&n_pid);
	ret = del_maps_data(&m_data); */
}
