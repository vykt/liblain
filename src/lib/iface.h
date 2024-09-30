#ifndef IFACE_H
#define IFACE_H

#include <stdbool.h>

#include "liblain.h"


//external
int ln_open(ln_session * session, int iface, int pid);
int ln_close(ln_session * session);
int ln_update_map(ln_session * session, ln_vm_map * vm_map);
int ln_read(ln_session * session, uintptr_t addr, cm_byte * buf, size_t buf_sz);
int ln_write(ln_session * session, uintptr_t addr, cm_byte * buf, size_t buf_sz);


#endif
