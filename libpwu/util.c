#include <stdio.h>
#include <signal.h>

#include "libpwu.h"
#include "util.h"

void bytes_to_hex(byte * inp, int inp_len, char * out) {

	for (int byte_index = 0; byte_index < inp_len; ++byte_index) {
		sprintf(&out[byte_index * 2], "%02x", inp[byte_index]);
	}
}


int sig
