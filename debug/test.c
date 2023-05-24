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

	char debug_char;
	int ret;
	int cave_num;
	int fd_mem;
	FILE * fd_maps;

	uint32_t old_jump_offset;

	char * payload_filename = "payload.o";
	unsigned int target_offset = 0x17f;

	maps_data m_data;
	maps_entry * m_entry;

	name_pid n_pid;
	puppet_info p_info;

	cave cav;
	raw_injection r_injection_dat;
	rel_jump_hook hook_dat;

	//-----INIT
	ret = new_maps_data(&m_data);
	if (ret == -1) return -1;

	ret = new_name_pid(&n_pid, "target");
	if (ret == -1) return -1;


	//-----SETUP & PERMISSIONS
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
	ret = change_region_perms(&p_info, 7, fd_mem, &m_data, m_entry);
	if (ret == -1) return -1;

	//TODO debug
	//printf("debug -> write permissions granted (press enter)");
	//scanf("%c", &debug_char);

	//-----INJECTING
	//get caves and make sure there's at least one available
	ret = get_caves(m_entry, fd_mem, 20, &cav); //get caves of size 20+
	if (ret <= 0) return -1;

	//inject payload
	ret = new_raw_injection(&r_injection_dat, m_entry, cav.offset, payload_filename);
	if (ret == -1) return -1;

	ret = raw_inject(r_injection_dat, fd_mem);
	if (ret == -1) return -1;

	//hook call from target to payload
	hook_dat.from_region = m_entry;
	hook_dat.from_offset = target_offset;
	hook_dat.to_region = m_entry;
	hook_dat.to_offset = cav.offset;

	old_jump_offset = hook_rj(hook_dat, fd_mem);
	if (old_jump_offset == 0) return -1;


	//-----CLEANUP
	//delete injection data
	del_raw_injection(&r_injection_dat);

	//change restore r-x permissions for .text segment
	ret = change_region_perms(&p_info, 5, fd_mem, &m_data, m_entry);
	if (ret == -1) return -1;

	//detach from the target process
	ret = puppet_detach(p_info);
	if (ret == -1) return -1;

	//delete data structures
	ret = del_name_pid(&n_pid);
	ret = del_maps_data(&m_data);
}
