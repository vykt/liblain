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


	//get target's PID
	char * name = "target";
	name_pid n_pid;

	ret = new_name_pid(&n_pid, name);
	printf("new name: %d\n", ret);

	ret = pid_by_name(&n_pid);
	printf("number of matches: %d\n", ret);

	ret = vector_get(&n_pid.pid_vector, 0, (byte *) &process_pid);
	printf("pid of process: %d\n", process_pid);

	ret = del_name_pid(&n_pid);
	printf("del name: %d\n", ret);


	//use PID to attach as debugger
	/*ret = puppet_attach(process_pid);
	printf("puppet_attach: %d\n", ret);

	ret = puppet_detach(process_pid);
	printf("puppet_detach: %d\n", ret);
	*/
	return 0;

}
