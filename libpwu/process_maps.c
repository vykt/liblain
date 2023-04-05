#include <stdlib.h>

#include "process_maps.h"


//get the address range values from a line in /proc/<pid>/maps
int get_addr_range(char line[LINE_LEN], void ** start_addr, void ** end_addr) {

	int next = 0;
	int start_count = 0;
	int end_count = 0;

	char buf_start[LINE_LEN / 2] = {0};
	char buf_end[LINE_LEN / 2] = {0};

	//for every character of line
	for (int i = 0; i < LINE_LEN; ++i) {

		if (line[i] == '-') {next = 1; continue;}
		if (line[i] == ' ') break;

		if(next) {
			buf_start[start_count] = line[i];
			++start_count;
		} else {
			buf_end[end_count] = line[i];
			++end_count;
		}

	} //end for every character of line
	
	*start_addr = (void *) strtol(buf_start, NULL, 16);
	*end_addr = (void *) strtol(buf_end, NULL, 16);
	if (*start_addr == 0 || *end_addr == 0) {
		return -1; //convertion failed
	} else {
		return 0;  //convertion successful
	}
}
