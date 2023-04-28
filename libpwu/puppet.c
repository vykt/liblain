#include <stdio.h>
#include <time.h>

#include <sys/types.h>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <sys/user.h>

#include "libpwu.h"
#include "puppet.h"


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
int change_region_perms(puppet_info * p_info, byte perms, 
		                maps_data * target_maps, maps_entry * target_region) {

	//TODO:
	//
	//1) search through every maps object looking for the syscall instruction (0x0F05)
	//
	//2) when found:
	//		-record the instruction address
	//		-save registers
	//		-change instruction pointer to the syscall address
	//		-change RAX to mprotect number
	//		-change other registers to hold correct parameters for mprotect
	//		-let execution continue until return from syscall
	//		-trap to this function again
	//		-restore registers
	//		-done!

}















