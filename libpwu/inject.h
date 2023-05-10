#ifndef INJECT_H
#define INJECT_H

#include "libpwu.h"

//external
int get_caves(maps_entry * m_entry, int fd_mem, int min_size);
int raw_inject(raw_injection r_injection, int fd_mem);

#endif
