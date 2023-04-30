#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <dirent.h>
#include <errno.h>
#include <ctype.h>

#include <sys/types.h>
#include <linux/limits.h>

#include "libpwu.h"
#include "name_pid.h"
#include "vector.h"


//initialise name_pid object
int new_name_pid(name_pid * n_pid, char * name) {

	int ret;
	size_t size_ret;

	//copy name string into fixed size buffer, don't allow invalid names
	size_ret = strlen(name);
	if (size_ret >= NAME_MAX) return -1;
	strncpy(n_pid->name, name, size_ret);

	ret = new_vector(&n_pid->pid_vector, sizeof(pid_t));
	return ret; //0 on success, -1 on fail
}


//deallocate name_pid object
int del_name_pid(name_pid * n_pid) {

	int ret;

	ret = del_vector(&n_pid->pid_vector);
	return ret; //0 on success, -1 on fail
}


//fill a vector with every name match of pid
int pid_by_name(name_pid * n_pid, pid_t * first_pid) {

	int ret;
	size_t rd_wr;
	pid_t temp_pid;
	int first_recorded = 0;

	DIR * proc_dir;
	struct dirent * entry;
	int fd;

	char path_buf[PATH_MAX];
	char comm_buf[NAME_MAX];

	proc_dir = opendir("/proc");
	if (proc_dir == NULL) return -1;

	//while entries left
	while ((entry = readdir(proc_dir)) != NULL) {

		//if the entry is a process directory named after the pid
		if (entry->d_type == DT_DIR && isdigit(entry->d_name[0]) > 0) {

			//build path
			memset(path_buf, 0, PATH_MAX); //zero out to make strcat behave
			strcat(path_buf, "/proc/");
			strcat(path_buf, entry->d_name);
			strcat(path_buf, "/comm");

			//get name
			fd = open(path_buf, O_RDONLY);
			if (fd == -1) {
				closedir(proc_dir);
				return -1;
			}

			rd_wr = read(fd, comm_buf, NAME_MAX);
			if (rd_wr == -1) {
				closedir(proc_dir);
				close(fd);
				return -1;
			}

			ret = close(fd);
			if (ret == -1) return -1;

			//if found process with matching name
			comm_buf[strcspn(comm_buf, "\n")] = '\0'; //replace trailing newline
			ret = strcmp(comm_buf, n_pid->name);
			if (!ret) {

				//convert dir name to pid_t
				temp_pid = (pid_t) strtoul(entry->d_name, NULL, 10);
				if (errno == ERANGE) {
					closedir(proc_dir);
					return -1;
				}
				
				//add pid_t to list of potential PIDs
				ret = vector_add(&n_pid->pid_vector, 0, (byte *) &temp_pid, APPEND_TRUE);
				if (ret == -1) {
					closedir(proc_dir);
					return -1;
				}

				//write first found PID to shortcut
				if (first_pid != NULL && first_recorded == 0) {
					*first_pid = temp_pid;
					first_recorded = 1;
				}

			}//end if found process with matching name

			//set path_buf to 0 by setting path_buf[0] = '\0'
			memset(path_buf, 0, NAME_MAX);
		}

	}//end while entries left
	
	ret = closedir(proc_dir);
	if (ret == -1) return -1;

	return n_pid->pid_vector.length;
}
