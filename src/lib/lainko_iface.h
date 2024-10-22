#ifndef LAINKO_IFACE_H
#define LAINKO_IFACE_H

#include <libcmore.h>

#include "liblain.h"


#define LAINKO_MAJOR_PATH "/sys/class/lainko/lainmemu_major"
#define LAINMEMU_MINOR 0

//internal
int _lainko_open(ln_session * session, const pid_t pid);
int _lainko_close(ln_session * session);
int _lainko_update_map(const ln_session * session, ln_vm_map * vm_map);
int _lainko_read(const ln_session * session, const uintptr_t addr, 
                 cm_byte * buf, const size_t buf_sz);
int _lainko_write(const ln_session * session, const uintptr_t addr, 
                  const cm_byte * buf, const size_t buf_sz);

#endif
