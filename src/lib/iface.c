#include <stdbool.h>
#include <stdint.h>

#include <libcmore.h>

#include "iface.h"
#include "liblain.h"

#include "procfs_iface.h"
#include "lainko_iface.h"


static inline void set_procfs_session(ln_session * session) {

    session->iface.open       = _procfs_open;
    session->iface.close      = _procfs_close;
    session->iface.update_map = _procfs_update_map;
    session->iface.read       = _procfs_read;
    session->iface.write      = _procfs_write;

    return;
}


static inline void set_lainko_session(ln_session * session) {

    session->iface.open       = _lainko_open;
    session->iface.close      = _lainko_close;
    session->iface.update_map = _lainko_update_map;
    session->iface.read       = _lainko_read;
    session->iface.write      = _lainko_write;
    
    return;
}


//open session
int ln_open(ln_session * session, int iface, int pid) {

    int ret;

    //if requesting procfs interface
    if (iface == LN_IFACE_PROCFS) {
        set_procfs_session(session);        
    } else {
        set_lainko_session(session);
    }

    ret = session->iface.open(session, pid);
    if (ret) return -1;

    return 0;
}


//close session
int ln_close(ln_session * session) {

    int ret;

    ret = session->iface.close(session);
    if (ret) return -1;

    return 0;
}


//update a map
int ln_update_map(ln_session * session, ln_vm_map * vm_map) {

    int ret;

    ret = session->iface.update_map(session, vm_map);
    if (ret) return -1;

    return 0;
}


//read memory
int ln_read(ln_session * session, uintptr_t addr, cm_byte * buf, size_t buf_sz) {

    int ret;

    ret = session->iface.read(session, addr, buf, buf_sz);
    if (ret) return -1;

    return 0;
}


//write memory
int ln_write(ln_session * session, uintptr_t addr, cm_byte * buf, size_t buf_sz) {

    int ret;

    ret = session->iface.write(session, addr, buf, buf_sz);
    if (ret) return -1;

    return 0;
}
