#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>

#include <linux/limits.h>

#include "../libpwu/libpwu.h"

int main() {

	int ret;
	int match_count;
	pid_t process_pid;
	void * match_addr;

	/*FILE * fd = fopen("/proc/22422/maps", "r");
	int fd_mem = open("/proc/22422/mem", O_RDONLY);
	if (fd_mem == -1) {
		perror("fd_mem");
		exit(-1);
	}

	maps_data m_data;
	maps_obj m_obj;
	maps_entry m_entry;
	pattern ptn;

	//set up maps object
	ret = new_maps_data(&m_data);
	ret = read_maps(&m_data, fd);

	

	ret = del_maps_data(&m_data);
	fclose(fd);*/

	char * name = "target";
	name_pid n_pid;

	ret = new_name_pid(&n_pid, name);
	printf("new name: %d\n", ret);

	ret = pid_by_name(&n_pid);
	printf("number of matches: %d\n", ret);

	ret = vector_get(&n_pid.pid_vector, 0, (byte *) &process_pid);
	printf("pid of process: %d\n", process_pid);

	ret = sig_stop(process_pid);
	printf("sent stop: %d\n", ret);
	sleep(8);
	ret = sig_cont(process_pid);
	printf("sent cont: %d\n", ret);

	ret = del_name_pid(&n_pid);
	printf("del name: %d\n", ret);

	return 0;

}
