#include <stdio.h>
#include <unistd.h>

#include "libpwu.h"
#include "rdwr_mem.h"

//read memory at address into read_buf
int read_mem(int fd, void * addr, char * read_buf, int len) {

	ssize_t rdwr_ret;
	off_t off_ret;

	//seek to address
	off_ret = lseek(fd, (off_t) addr, SEEK_SET);
	if (off_ret == -1) return -1;

	//read into buffer
	rdwr_ret = read(fd, read_buf, len);
	if (rdwr_ret == -1) return -1;

	return 0;
}

//write memory at address from write_buf
int write_mem(int fd, void * addr, char * write_buf, int len) {

	ssize_t rdwr_ret;
	off_t off_ret;

	//seek to address
	off_ret = lseek(fd, (off_t) addr, SEEK_SET);
	if (off_ret == -1) return -1;

	//write from buffer
	rdwr_ret = write(fd, write_buf, len);
	if (rdwr_ret == -1) return -1;

	return 0;
}
