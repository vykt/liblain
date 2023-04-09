#ifndef _LIBPWU_H
#define _LIBPWU_H

#include "process_maps.h"

//params: line: zero filled buffer, addrs: integers of type void *
//return: 0 - success, -1 - failed to convert address from string
extern int get_addr_range(char line[LINE_LEN], void ** start_addr, void ** end_addr);

//return: 0 - success, -1 - name not found, name not set
extern int get_name(char line[LINE_LEN], char name[PATH_MAX]);

#endif
