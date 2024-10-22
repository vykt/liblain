#ifndef IFACE_H
#define IFACE_H

#include <stdbool.h>

#include "liblain.h"


//external
int ln_open(ln_session * session, const int iface, const pid_t pid);
int ln_close(ln_session * session);
int ln_update_map(const ln_session * session, ln_vm_map * vm_map);
int ln_read(const ln_session * session, const uintptr_t addr, 
            cm_byte * buf, const size_t buf_sz);
int ln_write(const ln_session * session, const uintptr_t addr, 
             const cm_byte * buf, const size_t buf_sz);


#endif
