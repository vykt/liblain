#include <stdio.h>
#include <unistd.h>

#include "libpwu.h"
#include "rdwr_mem.h"


//read memory at address into read_buf
int read_mem(int fd, void * addr, byte * read_buf, int len) {

	long max_read;
	ssize_t rdwr_ret;
	off_t off_ret;

	ssize_t rd_done = 0;

	//get read capacity
	max_read = fcntl(fd, F_GETPIPE_SZ);
	if (max_read == -1) return -1;

	//seek to address
	off_ret = lseek(fd, (off_t) addr, SEEK_SET);
	if (off_ret == -1) return -1;

	//read max_read bytes repeatedly until done
	do {

		//read into buffer
		rdwr_ret = read(fd, read_buf, max_read);
		//if error or EOF before reading len bytes
		if (rdwr_ret == -1 || (rdwr_ret == 0 && rd_done < len)) return -1;
		rd_done += rdwr_ret;

	} while (rd_done < len);

	return 0;
}

//write memory at address from write_buf
int write_mem(int fd, void * addr, byte * write_buf, int len) {

	long max_write
	ssize_t rdwr_ret;
	off_t off_ret;

	ssize_t wr_done = 0;

	//get write capacity
	max_write = fcntl(fd, F_GETPIPE_SZ);
	if (max_read == -1) return -1;

	//seek to address
	off_ret = lseek(fd, (off_t) addr, SEEK_SET);
	if (off_ret == -1) return -1;

	//write max_write bytes repeatedly until done
	do {

		//write from buffer
		rdwr_ret = write(fd, write_buf, max_write);
		//if error or EOF before writing len bytes
		if (rdwr_ret == -1 || (rdwr_ret == 0 && wr_done < len)) return -1;
		wr_done += rdwr_ret;

	} while (wr_done < len);
	
	return 0;
}
