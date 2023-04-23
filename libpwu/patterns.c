#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <linux/limits.h>

#include "libpwu.h"
#include "patterns.h"
#include "vector.h"


//initiate new pattern
int new_pattern(pattern * ptn, maps_entry * search_region, byte * ptn, int ptn_len) {

	int ret;

	memcpy(ptn->pattern_bytes, ptn, ptn_len);
	ptn->pattern_len = ptn_len;
	ptn->search_region = search_region;

	ret = new_vector(&ptn->instances, sizeof(void *));
	if (ret != 0) return -1;
	return 0;
}


//delete pattern
int del_pattern(pattern * ptn) {

	int ret;
	ret = del_vector(&ptn->instances);
	if (ret != 0) return -1;
	return 0;

}


//return number of patterns matched, -1 on error
int find_pattern(pattern ptn, int fd) {

	int ret;
	int scanning = 1;
	int ptn_count = 0;
	int page_rd_count = 0;
	size_t rd_size;
	ssize_t rd_bytes;

	void * start_map_addr;
	void * end_map_addr;
	void * cur_map_addr;

	long page_size;
	byte * mem_page;

	//get a buffer the size of a page
	page_size = sysconf(_SC_PAGESIZE);
	if (page_size < 0) return -1;
	mem_page = malloc(page_size);
	if (mem_page == NULL) return -1;


	//read process region a page at a time
	while (scanning) {

		//zero out page buffer TODO seems not necessary
		//memset(mem_page, 0, page_size);

		//figure out how much to read to not segfault
		curr_map_addr = start_map_addr + (page_size * page_rd_count);
		if (end_map_addr - curr_map_addr > page_size) {
			rd_size = page_size;
		} else {
			//if not reading a full page, this is the final page.
			//therefore dont iterate again
			rd_size = end_map_addr - curr_map_addr;
			scanning = 0;
		}

		//read new page
		rd_bytes = read(fd, mem_page, rd_size);

		//for every byte of page
		for (int byte_index = 0; byte_index < rd_size; ++byte_index) {

			//if the next byte is in the pattern
			if (ptn->pattern_bytes[ptn_count] == mem_page[byte_index]) {
				++ptn_count;

				//if the entire pattern has now been matched
				if (ptn_count == ptn->pattern_len) {

					//add address where pattern began to instances
					ret = vector_add(&ptn->instances, 0, (char *) cur_map_addr 
							         + byte_index - ptn->pattern_len);
					if (ret != 0) return -1;
					//reset pattern count
					ptn_count = 0;
				}
			}
		}//end for every byte of page
	}//end read process region

	//return number of patterns matched
	return ptn->instances.length;
}
