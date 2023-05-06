#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <linux/limits.h>

#include "libpwu.h"
#include "pattern.h"
#include "vector.h"


//initiate new pattern, search_region can be NULL and set by user later
int new_pattern(pattern * ptn, maps_entry * search_region, byte * bytes_ptn, int bytes_ptn_len) {

	int ret;
	memset(ptn->pattern_bytes, 0, PATTERN_LEN);

	memcpy(ptn->pattern_bytes, bytes_ptn, bytes_ptn_len);
	ptn->pattern_len = bytes_ptn_len;
	
	if (search_region != NULL) ptn->search_region = search_region;

	ret = new_vector(&ptn->instance_vector, sizeof(void *));
	return ret; //0 on success, -1 on fail
}


//delete pattern
int del_pattern(pattern * ptn) {

	int ret;
	ret = del_vector(&ptn->instance_vector);
	return ret; //0 on success, -1 on fail
}


//return number of patterns matched, -1 on error
int match_pattern(pattern * ptn, int fd_mem) {

	int ret;
	int ptn_count = 0;

	void * match_addr;

	long page_size;
	byte * mem_page;

	//get a buffer the size of a page
	page_size = sysconf(_SC_PAGESIZE);
	if (page_size < 0) return -1;
	mem_page = malloc(page_size);
	if (mem_page == NULL) return -1;

	//read process region page
	for (int i = 0; i < (ptn->search_region->end_addr 
		                 - ptn->search_region->start_addr) / page_size; ++i) {

		//read page
		ret = read_mem(fd_mem, 
				       ptn->search_region->start_addr + (page_size * i), 
					   mem_page, page_size);
		if (ret == -1) { free(mem_page); return -1; }

		//for every byte of page
		for (int byte_index = 0; byte_index < page_size; ++byte_index) {

			//if the next byte is in the pattern
			if (ptn->pattern_bytes[ptn_count] == mem_page[byte_index]) {
				++ptn_count;

				//if the entire pattern has now been matched
				if (ptn_count == ptn->pattern_len) {

					//add address where pattern began to instances
					match_addr = ptn->search_region->start_addr 
						         + (page_size * i) + byte_index 
								 - (ptn->pattern_len - 1);
					ret = vector_add(&ptn->instance_vector, 0, (byte *) &match_addr, 
							         APPEND_TRUE);
					if (ret == -1) return -1;
					//reset pattern count
					ptn_count = 0;
				}

			//next byte not in pattern
			} else {
				ptn_count = 0;
			}

		}//end for every byte of page
	}//end read process region page

	//return number of patterns matched
	return ptn->instance_vector.length;
}
