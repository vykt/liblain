#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>

#include <sys/syscall.h>
#include <sys/mman.h>


int main() {

	//define some goods
	long sys_ret;

	while (1) {
		sys_ret = syscall(1, 1, "hello!", 6);
		sleep(2);
	}

	return 0;
}
