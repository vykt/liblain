#ifndef PUPPET_H
#define PUPPET_H

#include <sys/types.h>
#include <sys/user.h>

#include "libpwu.h"

#define STOP_TIMEOUT 3

//external
int puppet_attach(puppet_info p_info);
int puppet_detach(puppet_info p_info);
int puppet_save_regs(puppet_info * p_info);
int puppet_write_regs(puppet_info * p_info);


#endif
