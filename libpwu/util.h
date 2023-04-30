#ifndef UTIL_H
#define UTIL_H

#include <stdio.h>

#include <sys/types.h>

#include "libpwu.h"

void bytes_to_hex(byte * inp, int inp_len, char * out);
int open_memory(pid_t pid, FILE ** fd_maps, int * fd_mem);
int sig_stop(pid_t pid);
int sig_cont(pid_t pid);

#endif
