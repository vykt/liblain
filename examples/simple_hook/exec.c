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

	char * payload_filename = "payload.o"; //name of our payload
	unsigned int target_offset = 0x17f;    //offset of the relative jump we're hooking
	int region_num = 1;                    //index of the segment where the jump is

	int ret;
	int fd_mem;               //fd for /proc/<pid>/mem
	FILE * fd_maps;           //stream for /proc/<pid>/maps
	uint32_t old_jump_offset; //buffer to store old relative jump offset

	//define uninitialised libpwu structs (see structs.md or man libpwu_structs)
	maps_data m_data;          //structure of target's /proc/<pid>/maps
	maps_entry * m_entry;      //pointer for a segment in m_data
	name_pid n_pid;            //struct to get pid using process' name
	puppet_info p_info;        //puppeting struct (see structs.md or man libpwu_structs)
	cave cav;                  //a found cave in the target process
	raw_injection r_injection; //struct for injecting a payload into the target
	rel_jump_hook hook;

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
	ret = get_caves(m_entry, fd_mem, 20, &cav); //get caves of size 20+
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
