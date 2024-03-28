#ifndef NAME_PID_H
#define NAME_PID_H

#include <sys/types.h>

#include "libpwu.h"

int new_name_pid(name_pid * n_pid, char * name);
int del_name_pid(name_pid * n_pud);
int pid_by_name(name_pid * n_pid, pid_t * first_pid);
int name_by_pid(pid_t pid, char * name);

#endif
