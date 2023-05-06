#ifndef PATTERNS_H
#define PATTERNS_H

#include "libpwu.h"

//external
int new_pattern(pattern * ptn, maps_entry * search_region, byte * bytes_ptn, int bytes_ptn_len);
int del_pattern(pattern * ptn);
int match_pattern(pattern * ptn, int fd_mem);

#endif
