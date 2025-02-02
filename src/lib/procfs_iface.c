//standard library
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <stdint.h>

//system headers
#include <unistd.h>

#include <linux/limits.h>

//external libraries
#include <cmore.h>

//local headers
#include "procfs_iface.h"
#include "memcry.h"
#include "map.h"
#include "krncry.h"



/*
 *  --- [INTERNAL] ---
 */
 
//build vm_entry from a line in procfs maps
DBG_STATIC DBG_INLINE
void _build_entry(struct vm_entry * entry, const char * line_buf) {

    int mode = 0;
    int done = 0;
    int column_count = 0;

    char * start_str;
    char * end_str;
    char * offset_str;
    char * file_path_str;

    char perm_chars[] = {'r', 'w', 'x', 's'};
    cm_byte perm_vals[] = {VM_READ, VM_WRITE, VM_EXEC, VM_SHARED};


    //save str for start addr
    start_str = (char *) line_buf;

    //for every character on the line
    for (int i = 0; i < LINE_LEN; ++i) {

        if (done) break;

        switch (mode) {

            case 0: //look for end addr
                if (line_buf[i] == '-' || line_buf[i] == 'p') {
                    ++i;
                    ++mode;
                    end_str = (char *) line_buf + i;
                } else {
                    continue;
                }
                break;

            case 1: //look for permissions & file offset
                if (line_buf[i] == ' ') {
                    
                    //get permissions
                    ++i;
                    ++mode;
                    //for every char in perms field
                    for (int j = 0; j < 4; ++j) {
                        if (*(line_buf + i + j) == perm_chars[j]) {
                            entry->prot += (krncry_pgprot_t) perm_vals[j];
                        }
                    } //end for

                    //get offset
                    i += 5;
                    offset_str = (char *) line_buf + i;

                } else {
                    continue;
                }
                break;
            
            case 2: //look for file path
                if (line_buf[i] == ' ' || line_buf[i] == '\t') {
                    ++column_count;
                    continue;
                }
                if (column_count <= 2) continue;
                file_path_str = (char *) line_buf + i;
                done = 1;

        } //end switch

    } //end for every char


    //fill entry (`prot` already filled inside switch statement)
    entry->vm_start = (unsigned long) strtol(start_str, NULL, 16);
    entry->vm_end = (unsigned long) strtol(end_str, NULL, 16);
    entry->file_off = (unsigned long) strtol(offset_str, NULL, 16);
    strncpy(entry->file_path, file_path_str, PATH_MAX);
    entry->file_path[strcspn(entry->file_path, "\n")] = '\0';

    return;
}



/*
 *  --- [INTERFACE] ---
 */
 
int procfs_open(mc_session * session,  const int pid) {

    int fd;
	char mem_buf[PATH_MAX] = {0};


    session->pid = pid;

    //get page size to determine maximum read/write size
    session->page_size = sysconf(_SC_PAGESIZE);
    if (session->page_size < 0) {
        mc_errno = MC_ERR_PAGESIZE;
        return -1;
    }

    //open procfs mem file
    snprintf(mem_buf, PATH_MAX, "/proc/%d/mem", pid);
    fd = open(mem_buf, O_RDWR);
    if (fd == -1) {
        mc_errno = MC_ERR_PROC_MEM;
        return -1;
    }
    session->fd_mem = fd;

    return 0;
}



int procfs_close(mc_session * session) {

    //close procfs mem file
    close(session->fd_mem);

    return 0;
}



int procfs_update_map(const mc_session * session, mc_vm_map * vm_map) {

    int ret;
    FILE * fs; 
	
    char map_buf[PATH_MAX] = {0};
    char line_buf[LINE_LEN];

    struct vm_entry new_entry;
    _traverse_state state;


    //open memory map
    snprintf(map_buf, PATH_MAX, "/proc/%d/maps", session->pid);
    fs = fopen(map_buf, "r");
    if (fs == NULL) {
        mc_errno = MC_ERR_PROC_MAP;
        return -1;
    }

    //init traverse state for this map
    map_init_traverse_state(&state, vm_map);

    //while there are entries left
    while (fgets(line_buf, LINE_LEN, fs) != NULL) {

        memset(&new_entry, 0, sizeof(new_entry));
        _build_entry(&new_entry, line_buf);        

        ret = map_send_entry(&new_entry, &state, vm_map);
        if (ret != 0) return -1;

    } //end while


    //close memory map
    fclose(fs);

    return 0;
}



int procfs_read(const mc_session * session, const uintptr_t addr, 
                cm_byte * buf, const size_t buf_sz) {

	off_t off_ret;
	ssize_t read_bytes, read_done, read_left;
	
    read_done = read_left = 0;

	//seek to address
	off_ret = lseek(session->fd_mem, (off_t) addr, SEEK_SET);
	if (off_ret == -1) {
        mc_errno = MC_ERR_SEEK_ADDR;
        return -1;
    }

	//read page_size bytes repeatedly until done
	do {

        //calc how many bytes left to read
        read_left = buf_sz - read_done;

		//read into buffer
		read_bytes = read(session->fd_mem, buf + read_done, 
                          read_left > session->page_size 
                          ? session->page_size : read_left);
		//if error or EOF before reading len bytes
		if (read_bytes == -1 || (read_bytes == 0 && read_done < buf_sz)) {
            mc_errno = MC_ERR_READ_WRITE;
            return -1;
        }
		read_done += read_bytes;

	} while (read_done < buf_sz);

	return 0;
}



int procfs_write(const mc_session * session, const uintptr_t addr, 
                 const cm_byte * buf, const size_t buf_sz) {

	off_t off_ret;
	ssize_t write_bytes, write_done, write_left;
	
    write_done = write_left = 0;

	//seek to address
	off_ret = lseek(session->fd_mem, (off_t) addr, SEEK_SET);
	if (off_ret == -1) {
        mc_errno = MC_ERR_SEEK_ADDR;
        return -1;
    }

	//write page_size bytes repeatedly until done
	do {

        //calc how many bytes left to write
        write_left = buf_sz - write_done;

		//write into buffer
		write_bytes = write(session->fd_mem, buf + write_done, 
                            write_left > session->page_size 
                            ? session->page_size : write_left);
		//if error or EOF before writing len bytes
		if (write_bytes == -1 || (write_bytes == 0 && write_done < buf_sz)) {
            mc_errno = MC_ERR_READ_WRITE;
            return -1;
        }
		write_done += write_bytes;

	} while (write_done < buf_sz);

	return 0;
}
