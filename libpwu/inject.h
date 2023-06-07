#ifndef INJECT_H
#define INJECT_H

#include "libpwu.h"

//external
int get_caves(maps_entry * m_entry, int fd_mem, int min_size, cave * first_cave);
int find_cave(maps_entry * m_entry, int fd_mem, int required_size,
	          cave * matched_cave);

int raw_inject(raw_injection r_injection_dat, int fd_mem);
int new_raw_injection(raw_injection * r_injection_dat, maps_entry * target_region,
		              unsigned int offset, char * payload_filename);
void del_raw_injection(raw_injection * r_injection_dat);

#endif
