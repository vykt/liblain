#ifndef PROC_IFACE_H
#define PROC_IFACE_H

#include <libcmore.h>

#include "liblain.h"


//internal
int _procfs_open(ln_session * session,  const int pid);
int _procfs_close(ln_session * session);
int _procfs_update_map(const ln_session * session, ln_vm_map * vm_map);
int _procfs_read(const ln_session * session, const uintptr_t addr, 
                 cm_byte * buf, const size_t buf_sz);
int _procfs_write(const ln_session * session, const uintptr_t addr, 
                  const cm_byte * buf, const size_t buf_sz);


#endif
