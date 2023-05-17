#ifndef INJECT_H
#define INJECT_H

#include "libpwu.h"

//external
int get_caves(maps_entry * m_entry, int fd_mem, int min_size, cave * first_cave);

int raw_inject(raw_injection r_injection, int fd_mem);
int new_raw_injection(raw_injection * r_injection, maps_entry * target_region,
		              unsigned int offset, char * payload_filename);
void del_raw_injection(raw_injection * r_injection);

#endif
