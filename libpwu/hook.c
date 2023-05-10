#include <unistd.h>


#include "libpwu.h"
#include "hook.h"

//hook a 32bit relative jump to jump elsewhere
//return overwritten relative jump
void * hook_rj(rel_jump_hook rj_hook, int fd_mem) {

	off_t ret;
	ssize_t rdwr_ret;
	void * old_offset;
	void * new_offset;

	//calculate new offset
	new_offset = (void *) (rj_hook.from_region->start_addr + rj_hook.from_offset
		   - rj_hook.to_region->start_addr + rj_hook.to_offset + 4);

	//save old jump offset
	ret = lseek(fd_mem, (off_t) rj_hook.from_region->start_addr + rj_hook.from_offset 
			    + 1, SEEK_SET);
	if (ret == -1) return NULL;
	rdwr_ret = read(fd_mem, &old_offset, 4);
	if (rdwr_ret == -1) return NULL;

	//write new offset
	ret = lseek(fd_mem (off_t) rj_hook.from_region->start_addr + rj_hook.from_offset
			    + 1, SEEK_SET);
	if (ret == -1) return NULL;
	rdwr_ret = write(fd_mem, &new_offset, 4);
	if (rdwr_ret == -1) return NULL;
	
	return old_offset;
}
