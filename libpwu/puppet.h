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
int change_region_perms(puppet_info * p_info, byte perms, int fd,
		                maps_data * m_data, maps_entry * target_region);

#endif
