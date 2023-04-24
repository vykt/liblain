#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <fcntl.h>

#include <linux/limits.h>

#include "../libpwu/libpwu.h"

int main() {

	int ret;
	int match_count;
	void * match_addr;

	FILE * fd = fopen("/proc/22422/maps", "r");
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


	//set up pattern object
	//ret = new_pattern(&ptn, NULL, "hello!", 6);
	ret = new_pattern(&ptn, NULL, "\xe8\xcc\xfe\xff\xff\xbf\x01", 7);
	ret = vector_get(&m_data.obj_vector, 0, (char *) &m_obj);

	for (int i = 0; i < 5; ++i) {
		ret = vector_get(&m_obj.entry_vector, i, (char *) &m_entry);
		
		ptn.search_region = &m_entry;
		match_count = match_pattern(&ptn, fd_mem);
	}

	//print out matches
	for ( int i = 0; i < ptn.instances.length; ++i) {
		ret = vector_get(&ptn.instances, i, (char *) &match_addr);
		printf("match at: %lx\n", match_addr);
	}

	ret = del_maps_data(&m_data);
	ret = del_pattern(&ptn);

	fclose(fd);

	return 0;

}
