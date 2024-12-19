#ifndef KRNCRY_IFACE_H
#define KRNCRY_IFACE_H

//external libraries
#include <cmore.h>

//local headers
#include "memcry.h"
#include "debug.h"


#define KRNCRY_MAJOR_PATH "/sys/class/krncry/krncry_major"
#define KRNCRY_MINOR 0


#ifdef DEBUG
//internal
char _krncry_iface_get_major();
#endif


//interface
int krncry_open(mc_session * session, const pid_t pid);
int krncry_close(mc_session * session);
int krncry_update_map(const mc_session * session, mc_vm_map * vm_map);
int krncry_read(const mc_session * session, const uintptr_t addr, 
                cm_byte * buf, const size_t buf_sz);
int krncry_write(const mc_session * session, const uintptr_t addr, 
                 const cm_byte * buf, const size_t buf_sz);

#endif
