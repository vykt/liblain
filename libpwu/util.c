#include <stdio.h>
#include <signal.h>

#include "libpwu.h"
#include "util.h"


//give byte string, return char string of double size
void bytes_to_hex(byte * inp, int inp_len, char * out) {

	for (int byte_index = 0; byte_index < inp_len; ++byte_index) {
		sprintf(&out[byte_index * 2], "%02x", inp[byte_index]);
	}
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
