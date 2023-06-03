#ifndef PUPPET_H
#define PUPPET_H

#include <sys/types.h>
#include <sys/user.h>

#include "libpwu.h"

#define STOP_TIMEOUT 3

//external
int puppet_attach(puppet_info p_info);
int puppet_detach(puppet_info p_info);
int puppet_find_syscall(puppet_info * p_info, maps_data * m_data, int fd_mem);
int puppet_save_regs(puppet_info * p_info);
int puppet_write_regs(puppet_info * p_info);
void puppet_copy_regs(puppet_info * p_info, int mode);

int arbitrary_syscall(puppet_info * p_info, int fd_mem, 
                      unsigned long long * syscall_ret);
int change_region_perms(puppet_info * p_info, byte perms, int fd_mem,
		                maps_entry * target_region);
int create_thread_stack(puppet_info * p_info, int fd_mem, void ** stack_addr,
                        unsigned int stack_size);
int start_thread(puppet_info * p_info, void * exec_addr, int fd_mem,
                 void * stack_addr, unsigned int stack_size, int * tid);

#endif
