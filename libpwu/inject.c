#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <sys/stat.h>

#include "libpwu.h"
#include "inject.h"
#include "rdwr_mem.h"
#include "vector.h"


//find caves in memory region and fill vector with them
//return number of caves found, -1 on error
int get_caves(maps_entry * m_entry, int fd_mem, int min_size, cave * first_cave) {

	int ret;
	int cave_count = 0;

	cave cur_cave;
	cur_cave.offset = 0;
	cur_cave.size = 0;

	//int cur_size = 0;
	//void * cur_addr = NULL;

	long page_size;
	byte * mem_page;


	//get buffer the size of a page
	page_size = sysconf(_SC_PAGESIZE);
	if (page_size < 0) return -1;
	mem_page = malloc(page_size);
	if (mem_page == NULL) return -1;

	//for every page in region
	for (int i = 0; i < (m_entry->end_addr - m_entry->start_addr) / page_size; ++i) {

		//read page
		ret = read_mem(fd_mem, m_entry->start_addr + (page_size * i),
				       mem_page, page_size);
		if (ret == -1) { free(mem_page); return -1; }

		//for every byte in page:
		for (int j = 0; j < page_size; ++j) {

			//if zero
			if (mem_page[j] == 0 || mem_page[j] == 255) { //0x00 or 0xFF
				//if new cave
				if (cur_cave.size == 0) {
					cur_cave.offset = (page_size*i) + j;
				}
				cur_cave.size += 1;

			//else not zero
			} else {
				//if cave above min_size
				if (cur_cave.size >= min_size) {

					ret = vector_add(&m_entry->cave_vector, 0, (byte *) &cur_cave,
							         APPEND_TRUE);
					if (ret == -1) { free(mem_page); return -1; }
					//if this is the first cave, set first_cave
					if (cave_count == 0) *first_cave = cur_cave;
					cave_count++;
				}

				cur_cave.size = 0;
			}

		} //end for every byte in page
	} //end for every page in region

	//if there is an ongoing pattern when there are no more bytes left in region
	if (cur_cave.size > min_size) {

		ret = vector_add(&m_entry->cave_vector, 0, (byte *) &cur_cave,
						 APPEND_TRUE);
		if (ret == -1) { free(mem_page); return -1; }
		//if this is the first cave, set first_cave
		if (cave_count == 0) *first_cave = cur_cave;
		++cave_count;
	}

	free(mem_page);
	return cave_count;
}


//find cave that meets size requirement
int find_cave(maps_entry * m_entry, int fd_mem, int required_size,
	          cave * matched_cave) {

	int ret;
	cave * cave_buf;

	//for every cave in region
	for (int i = 0; i < m_entry->cave_vector.length; ++i) {

		ret = vector_get_ref(&m_entry->cave_vector, i, (byte **) &cave_buf);
		if (ret == -1) return -2;

		//if cave size exceeds requirement
		if (cave_buf->size >= required_size) {
			matched_cave = cave_buf;
			return 0;
		}
	} //end for every cave
	return -1;
}


//inject contents of r_injection_dat.payload into target_region at offset
//region needs write permissions. get them from puppet.c
int raw_inject(raw_injection r_injection_dat, int fd_mem) {

	int ret;
	ret = write_mem(fd_mem, r_injection_dat.target_region->start_addr + r_injection_dat.offset,
			        r_injection_dat.payload, r_injection_dat.payload_size);
	return ret;
}


//initialise new raw injection
int new_raw_injection(raw_injection * r_injection_dat, maps_entry * target_region,
		              unsigned int offset, char * payload_filename) {

	int ret;

	int read_size;
	long page_size;
	size_t rdwr;
	size_t rdwr_total = 0;

	FILE * fp;
	struct stat fp_stat;

	//setup half the struct
	r_injection_dat->target_region = target_region;
	r_injection_dat->offset = offset;

	//get page_size;
	page_size = sysconf(_SC_PAGESIZE);
	if (page_size == -1) return -1;

	//open payload file
	fp = fopen(payload_filename, "r");
	if (fp == NULL) return -1;

	//get file size for malloc
	ret = fstat(fileno(fp), &fp_stat);
	if (ret == -1) { fclose(fp); return -1; }
	//fp_stat.st_size

	//allocate payload size
	r_injection_dat->payload = malloc(fp_stat.st_size);
	if (r_injection_dat->payload == NULL) { fclose(fp); return -1; }

	//read whole payload file
	while (rdwr_total < fp_stat.st_size) {

		//how many bytes to ask to read?
		if ((fp_stat.st_size - rdwr_total) > page_size) { read_size = page_size; }
		else { read_size = fp_stat.st_size - rdwr_total; }

		//read bytes to next appropriate place in payload buffer
		rdwr = fread(r_injection_dat->payload+rdwr_total, 1, read_size, fp);
		if (rdwr == -1) {
			free(r_injection_dat->payload);
			fclose(fp);
			return -1;
		} else {
			rdwr_total += rdwr;
		}
	}//end read whole payload file

	r_injection_dat->payload_size = rdwr_total;

	return 0;
}


//delete raw injection
void del_raw_injection(raw_injection * r_injection_dat) {

	free(r_injection_dat->payload);
}
