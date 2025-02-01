#ifndef PROC_IFACE_H
#define PROC_IFACE_H

//external libraries
#include <cmore.h>

//local headers
#include "memcry.h"
#include "krncry.h"
#include "debug.h"


#define LINE_LEN PATH_MAX + 128


#ifdef DEBUG
//internal
void _build_entry(struct vm_entry * entry, const char * line_buf);
#endif


//interface
int procfs_open(mc_session * session,  const int pid);
int procfs_close(mc_session * session);
int procfs_update_map(const mc_session * session, mc_vm_map * vm_map);
ssize_t procfs_read(const mc_session * session, const uintptr_t addr, 
                    cm_byte * buf, const size_t buf_sz);
ssize_t procfs_write(const mc_session * session, const uintptr_t addr, 
                     const cm_byte * buf, const size_t buf_sz);

#endif
