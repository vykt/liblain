#include <unistd.h>
#include <stdint.h>


#include "libpwu.h"
#include "hook.h"

//hook a 32bit relative jump to jump elsewhere
//return overwritten relative jump
uint32_t hook_rj(rel_jump_hook rj_hook_dat, int fd_mem) {

	off_t ret;
	ssize_t rdwr_ret;
	
	uint32_t old_offset;
	uint32_t new_offset;
	
	void * ptr_math_buf;

	//get new offset
	ptr_math_buf = (rj_hook_dat.to_region->start_addr + rj_hook_dat.to_offset)
		            - (rj_hook_dat.from_region->start_addr + rj_hook_dat.from_offset
					   + REL_JUMP_LEN);

	new_offset = (uint32_t) ptr_math_buf;

	//TODO debug
	/*void * ptr_math_buf1;
	void * ptr_math_buf2;
	
	ptr_math_buf1 = rj_hook_dat.from_region->start_addr + rj_hook_dat.from_offset;
	ptr_math_buf2 = rj_hook_dat.to_region->start_addr + rj_hook_dat.to_offset;
	
	ptr_math_buf1 = ptr_math_buf2 - ptr_math_buf1;
	new_offset = (uint32_t) ptr_math_buf1;
	*/
	//TODO end debug

	//save old jump offset
	ret = lseek(fd_mem, (off_t) rj_hook_dat.from_region->start_addr + rj_hook_dat.from_offset 
			    + 1, SEEK_SET);
	if (ret == -1) return 0;
	rdwr_ret = read(fd_mem, &old_offset, 4);
	if (rdwr_ret == -1) return 0;

	//write new offset
	ret = lseek(fd_mem, (off_t) rj_hook_dat.from_region->start_addr + rj_hook_dat.from_offset
			    + 1, SEEK_SET);
	if (ret == -1) return 0;
	rdwr_ret = write(fd_mem, &new_offset, 4);
	if (rdwr_ret == -1) return 0;
	
	return old_offset;
}
