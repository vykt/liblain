#ifndef _LIBPWU_H
#define _LIBPWU_H

#include "process_maps.h"

extern int pwu_owo();
extern int get_addr_range(char line[LINE_LEN], void ** start_addr, void ** end_addr);

#endif
