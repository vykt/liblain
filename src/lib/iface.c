//standard library
#include <stdbool.h>
#include <stdint.h>

//external libraries
#include <cmore.h>

//local headers
#include "iface.h"
#include "memcry.h"
#include "procfs_iface.h"
#include "krncry_iface.h"
#include "debug.h"



/*
 *  --- [INTERNAL] ---
 */

DBG_STATIC DBG_INLINE
void _set_procfs_session(mc_session * session) {

    session->iface.open       = procfs_open;
    session->iface.close      = procfs_close;
    session->iface.update_map = procfs_update_map;
    session->iface.read       = procfs_read;
    session->iface.write      = procfs_write;

    return;
}



DBG_STATIC DBG_INLINE
void _set_krncry_session(mc_session * session) {

    session->iface.open       = krncry_open;
    session->iface.close      = krncry_close;
    session->iface.update_map = krncry_update_map;
    session->iface.read       = krncry_read;
    session->iface.write      = krncry_write;
    
    return;
}



/*
 *  --- [EXTERNAL] ---
 */

int mc_open(mc_session * session,
            const enum mc_iface_type iface, const pid_t pid) {
    

    int ret;

    //if requesting procfs interface
    if (iface == PROCFS) {
        _set_procfs_session(session);        
    } else {
        _set_krncry_session(session);
    }

    ret = session->iface.open(session, pid);
    if (ret) return -1;

    return 0;
}



int mc_close(mc_session * session) {

    int ret;

    ret = session->iface.close(session);
    if (ret) return -1;

    return 0;
}



int mc_update_map(const mc_session * session, mc_vm_map * vm_map) {

    int ret;

    ret = session->iface.update_map(session, vm_map);
    if (ret) return -1;

    return 0;
}



int mc_read(const mc_session * session, const uintptr_t addr, 
            cm_byte * buf, const size_t buf_sz) {

    int ret;

    ret = session->iface.read(session, addr, buf, buf_sz);
    if (ret == -1) return -1;

    return 0;
}



int mc_write(const mc_session * session, const uintptr_t addr, 
             const cm_byte * buf, const size_t buf_sz) {

    int ret;

    ret = session->iface.write(session, addr, buf, buf_sz);
    if (ret == -1) return -1;

    return 0;
}
