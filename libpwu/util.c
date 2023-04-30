#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>

#include <sys/types.h>

#include <linux/limits.h>

#include "libpwu.h"
#include "util.h"


//give byte string, return char string of double size
void bytes_to_hex(byte * inp, int inp_len, char * out) {

	for (int byte_index = 0; byte_index < inp_len; ++byte_index) {
		sprintf(&out[byte_index * 2], "%02x", inp[byte_index]);
	}
}


//get a file descriptor for maps using pid
int open_memory(pid_t pid, FILE ** fd_maps, int * fd_mem) {

	char maps_buf[PATH_MAX] = {0};
	char mem_buf[PATH_MAX] = {0};
	char pid_buf[NAME_MAX] = {0};

	char * proc = "/proc/";
	char * maps = "/maps";
	char * mem = "/mem";

	snprintf(pid_buf, NAME_MAX, "%d", pid);

	//concatinate alllll the strings
	strcat(maps_buf, proc);
	strcat(mem_buf, proc);

	strcat(maps_buf, pid_buf);
	strcat(mem_buf, pid_buf);

	strcat(maps_buf, maps);
	strcat(mem_buf, mem);

	*fd_maps = fopen(maps_buf, "r");
	*fd_mem = open(mem_buf, O_RDWR);
	if (*fd_maps == NULL || *fd_mem == -1) return -1;
	return 0;
}


//send SIGSTOP to process, wrapper for clib's <signal.h>
int sig_stop(pid_t pid) {

	int ret;
	ret = kill(pid, SIGSTOP);
	return ret; //-1 on fail, 0 on success;
}


//send SIGCONT to process, wrapper for clib's <signal.h>
int sig_cont(pid_t pid) {

	int ret;
	ret = kill(pid, SIGCONT);
	return ret; //-1 on fail, 0 on success;

}
