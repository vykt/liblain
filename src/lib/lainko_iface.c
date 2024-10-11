#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <fcntl.h>
#include <unistd.h>

#include <sys/ioctl.h>
#include <sys/mman.h>

#include <linux/limits.h>

#include <libcmore.h>

#include "lainko_iface.h"
#include "liblain.h"
#include "lainko.h"
#include "map.h"


// --- LAINKO INTERFACE INTERNALS

//read lainko's major number
static inline char _get_major() {

    int fd;
    
    size_t read_bytes;
    char read_buf[8];   

    char major;

    //open major attribute
    fd = open(LAINKO_MAJOR_PATH, O_RDONLY);
    if (fd == -1) {
        ln_errno = LN_ERR_LAINKO_MAJOR;
        return -1;
    }

    //read string representation into buffer
    read_bytes = read(fd, &read_buf, 8);
    if (read_bytes == -1) {
        ln_errno = LN_ERR_LAINKO_MAJOR;
        return -1;
    }

    //convert back to binary representation
    major = (char) strtol(read_buf, NULL, 10);

    close(fd);
    return major;
}


// --- CALLED BY VIRTUAL INTERFACE

//locate and open the lainmemu device, and then open the target
int _lainko_open(ln_session * session, pid_t pid) {

    int ret;
    char memu_path[PATH_MAX];
    uint32_t ioctl_call;
    struct ioctl_arg arg;

    //get page size
    session->page_size = sysconf(_SC_PAGESIZE);
    if (session->page_size < 0) {
        ln_errno = LN_ERR_PAGESIZE;
        return -1;
    }

    //get major of the module, -1 if unloaded
    session->major = _get_major();
    if (session->major == -1) return -1;

    //build lainmemu device path
    snprintf(memu_path, PATH_MAX, "/dev/char/%d:%d", 
             (unsigned char) session->major, LAINMEMU_MINOR);

    //open the lainmemu device
    session->fd_dev_memu = open(memu_path, O_RDWR);
    if (session->fd_dev_memu == -1) {
        ln_errno = LN_ERR_MEMU_OPEN;
        return -1;
    }

    //call ioctl to set target
    arg.target_pid = pid;
    ioctl_call = LAINKO_APPLY_TEMPLATE((char) session->major, 
                                       LAINMEMU_TEMPLATE_OPEN_TGT);
    ret = ioctl(session->fd_dev_memu, ioctl_call, &arg);
    if (ret) {
        close(session->fd_dev_memu);
        ln_errno = LN_ERR_MEMU_TARGET;
        return -1;
    }

    return 0;
}


//close the target
int _lainko_close(ln_session * session) {

    int ret;
    uint32_t ioctl_call;
    struct ioctl_arg arg; //not used

    //call ioctl to release target
    ioctl_call = LAINKO_APPLY_TEMPLATE((char) session->major, 
                                       LAINMEMU_TEMPLATE_RELEASE_TGT);
    ret = ioctl(session->fd_dev_memu, ioctl_call, &arg);

    //close device
    close(session->fd_dev_memu);

    return 0;
}


//update the map
int _lainko_update_map(ln_session * session, ln_vm_map * vm_map) {

    int ret, count;
    uint32_t ioctl_call;
    size_t u_buf_sz;
    
    struct ioctl_arg arg;
    _traverse_state state;


    //call ioctl to get the map size
    ioctl_call = LAINKO_APPLY_TEMPLATE((char) session->major, 
                                       LAINMEMU_TEMPLATE_GET_MAP_SZ);
    count = ioctl(session->fd_dev_memu, ioctl_call, &arg);
    if (count <= 0) {
        ln_errno = LN_ERR_MEMU_MAP_SZ;
        return -1;
    }

    //allocate buffer to hold the map
    u_buf_sz = count * sizeof(struct vm_entry);
    arg.u_buf = mmap(NULL, u_buf_sz, 
                     PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (arg.u_buf == MAP_FAILED) {
        ln_errno = LN_ERR_MEM;
        return -1;
    }

    //get the map
    ioctl_call = LAINKO_APPLY_TEMPLATE((char) session->major, 
                                       LAINMEMU_TEMPLATE_GET_MAP);

    count = ioctl(session->fd_dev_memu, ioctl_call, &arg);
    if (count <= 0) {
        munmap(arg.u_buf, u_buf_sz);
        ln_errno = LN_ERR_MEMU_MAP_GET;
        return -1;
    }


    //update the map with each received segment
    _map_init_traverse_state(vm_map, &state);

    for (int i = 0; i < count; ++i) {
        
        ret = _map_send_entry(vm_map, &state, 
                              (struct vm_entry *) 
                              (arg.u_buf + (i * sizeof(struct vm_entry))));
        if (ret) {
            munmap(arg.u_buf, u_buf_sz);
            return -1;
        }
    } //end for

    //unmap map buffer
    munmap(arg.u_buf, u_buf_sz);

    return 0;
}


//read memory
int _lainko_read(ln_session * session, uintptr_t addr, 
                cm_byte * buf, size_t buf_sz) {

	off_t off_ret;
	ssize_t read_bytes, read_done, read_left;
	
    read_done = read_left = 0;

	//seek to address
	off_ret = lseek(session->fd_dev_memu, (off_t) addr, SEEK_SET);
	if (off_ret == -1) {
        ln_errno = LN_ERR_SEEK_ADDR;
        return -1;
    }

	//read page_size bytes repeatedly until done
	do {

        //calc how many bytes left to read
        read_left = buf_sz - read_done;

		//read into buffer
		read_bytes = read(session->fd_dev_memu, buf + read_done, 
                          read_left > session->page_size 
                          ? session->page_size : read_left);
		//if error or EOF before reading len bytes
		if (read_bytes == -1 || (read_bytes == 0 && read_done < buf_sz)) {
            ln_errno = LN_ERR_READ_WRITE;
            return -1;
        }
		read_done += read_bytes;

	} while (read_done < buf_sz);

	return 0;
}


//write memory
int _lainko_write(ln_session * session, uintptr_t addr, 
                 cm_byte * buf, size_t buf_sz) {

	off_t off_ret;
	ssize_t write_bytes, write_done, write_left;
	
    write_done = write_left = 0;

	//seek to address
	off_ret = lseek(session->fd_dev_memu, (off_t) addr, SEEK_SET);
	if (off_ret == -1) {
        ln_errno = LN_ERR_SEEK_ADDR;
        return -1;
    }

	//write page_size bytes repeatedly until done
	do {

        //calc how many bytes left to write
        write_left = buf_sz - write_done;

		//write into buffer
		write_bytes = write(session->fd_dev_memu, buf + write_done, 
                            write_left > session->page_size 
                            ? session->page_size : write_left);
		//if error or EOF before writing len bytes
		if (write_bytes == -1 || (write_bytes == 0 && write_done < buf_sz)) {
            ln_errno = LN_ERR_READ_WRITE;
            return -1;
        }
		write_done += write_bytes;

	} while (write_done < buf_sz);

	return 0;
}
