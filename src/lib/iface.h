#ifndef IFACE_H
#define IFACE_H

//standard library
#include <stdbool.h>

//system headers
#include <unistd.h>

//local headers
#include "memcry.h"


#ifdef DEBUG
//internal
void _set_procfs_session(mc_session * session);
void _set_krncry_session(mc_session * session);
#endif


//external
int mc_open(mc_session * session,
            const enum mc_iface_type iface, const pid_t pid);
int mc_close(mc_session * session);
int mc_update_map(const mc_session * session, mc_vm_map * vm_map);
int mc_read(const mc_session * session, const uintptr_t addr, 
            cm_byte * buf, const size_t buf_sz);
int mc_write(const mc_session * session, const uintptr_t addr, 
             const cm_byte * buf, const size_t buf_sz);

#endif
