#ifndef PROC_IFACE_H
#define PROC_IFACE_H

#include <libcmore.h>

#include "liblain.h"


//internal
int _procfs_open(ln_session * session,  int pid);
int _procfs_close(ln_session * session);
int _procfs_update_map(ln_session * session, ln_vm_map * vm_map);
int _procfs_read(ln_session * session, uintptr_t addr, cm_byte * buf, size_t buf_sz);
int _procfs_write(ln_session * session, uintptr_t addr, cm_byte * buf, size_t buf_sz);


#endif
