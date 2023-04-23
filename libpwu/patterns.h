#ifndef PATTERNS_H
#define PATTERNS_H

#include "libpwu.h"

//return address at start of pattern on success, otherwise return fail
int pattern_match(pattern ptn, int fd);



#endif
