#include <stdio.h>
#include <string.h>
#include <time.h>

#include <sys/types.h>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <sys/user.h>
#include <sys/mman.h>

#include <asm/unistd.h>

#include "libpwu.h"
#include "puppet.h"
#include "pattern.h"


//sends SIGSTOP itself, don't call util's sig_stop()
int puppet_attach(puppet_info p_info) {

	long ret;	
	pid_t ret_pid;
	time_t timeout;

	int wait_status;

	ret = ptrace(PTRACE_ATTACH, p_info.pid, NULL, NULL);
	if (ret == -1) return -1;

	ret_pid = waitpid(p_info.pid, &wait_status, 0);
	if (ret_pid == -1) return -1;

	//wait for process to stop
	timeout = time(NULL);
	while (1) {
		if (WIFSTOPPED(wait_status)) return 0;
		if (timeout + STOP_TIMEOUT >= timeout) {
			return -1;
		}
	}//end wait for process to stop
}


//detach and send SIGCONT
int puppet_detach(puppet_info p_info) {

	long ret;

	//send detach call
	ret = ptrace(PTRACE_DETACH, p_info.pid, NULL, NULL);
	if (ret == -1) return -1;

	return 0;
}


//save registers of puppeted process
int puppet_save_regs(puppet_info * p_info) {

	long ret;
	
	//get process registers
	ret = ptrace(PTRACE_GETREGS, p_info->pid, NULL, &p_info->saved_state);
	if (ret == -1) return -1;

	ret = ptrace(PTRACE_GETFPREGS, p_info->pid, NULL, &p_info->saved_float_state);
	if (ret == -1) return -1;

	return 0;
}


//write new register values to puppeted process
int puppet_write_regs(puppet_info * p_info) {

	long ret;

	ret = ptrace(PTRACE_SETREGS, p_info->pid, NULL, &p_info->new_state);
	if (ret == -1) return -1;

	ret = ptrace(PTRACE_SETFPREGS, p_info->pid, NULL, &p_info->saved_float_state);
	if (ret == -1) return -1;

	return 0;
}


//change permissions of a region
//perms = standard unix permission bits - 7 = rwx, 4 = r--, 3 = -wx
//target_region HAS TO BE A REFERENCE to a region inside target_maps
int change_region_perms(puppet_info * p_info, byte perms, int fd, 
		                maps_data * m_data, maps_entry * target_region) {

	int ret;
	long ptrace_ret;
	void * syscall_addr;
	int success = 0;

	maps_entry * m_entry_ref;
	pattern ptn;
	
	//create new pattern object to search for a syscall
	ret = new_pattern(&ptn, NULL, "\x0f\x05", 2);
	if (ret == -1) return -1;

	//for every region
	for (int i = 0; i < m_data->entry_vector.length; ++i) {
		
		//get region
		ret = vector_get_ref(&m_data->entry_vector, i, (byte **) &m_entry_ref);
		if (ret == -1) return -1;

		//continue if this region is not executable
		if (m_entry_ref->perms < PROT_EXEC ) continue;
		
		//search the region
		ptn.search_region = m_entry_ref;
		ret = match_pattern(&ptn, fd);
		if (ret == -1) return -1;
		if (ret == 0) continue;

		//if a match was found, carry out mprotect syscall
		ret = vector_get(&ptn.instance_vector, 0, (byte *) &syscall_addr);
		if (ret == -1) return -1;
		ret = puppet_save_regs(p_info);
		if (ret == -1) return -1;

		//copy saved registers to new registers
		memcpy(&p_info->new_state, &p_info->saved_state, 
			   sizeof(struct user_regs_struct));
		memcpy(&p_info->new_float_state, &p_info->saved_float_state,
			   sizeof(struct user_fpregs_struct));

		//setup registers for the call to mprotect
		p_info->new_state.rax = __NR_mprotect; //mprotect syscall number
		p_info->new_state.rdi = (long long int) target_region->start_addr;
		p_info->new_state.rsi = target_region->end_addr - target_region->start_addr;
		p_info->new_state.rdx = perms;
		p_info->new_state.rip = (long long int) syscall_addr;

		ret = puppet_write_regs(p_info);
		if (ret == -1) return -1;

		//catch entry and exit of syscall
		ptrace_ret = ptrace(PTRACE_SYSCALL, p_info->pid, NULL, NULL);
		if (ptrace_ret == -1) return -1;
		waitpid(p_info->pid, 0, 0);

		ptrace_ret = ptrace(PTRACE_SYSCALL, p_info->pid, NULL, NULL);
		if (ptrace_ret == -1) return -1;
		waitpid(p_info->pid, 0, 0);

		//update own memory map to new value
		target_region->perms = perms;

		//now restore registers
		memcpy(&p_info->new_state, &p_info->saved_state, 
			   sizeof(struct user_regs_struct));
		memcpy(&p_info->new_float_state, &p_info->saved_float_state,
			   sizeof(struct user_fpregs_struct));
		ret = puppet_write_regs(p_info);
		if (ret == -1) return -1;

		//done, exit
		success = 1;
		break;
	}

	ret = del_pattern(&ptn);
	if (ret == -1) return -1;

	if (success) return 0;
	return -2;
}
