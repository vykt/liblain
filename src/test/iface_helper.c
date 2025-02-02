//standard library
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

//system headers
#include <unistd.h>

//external libraries
#include <cmore.h>
#include <check.h>

//local headers
#include "info.h"
#include "iface_helper.h"
#include "target_helper.h"

//test target headers
#include "../lib/memcry.h"
#include "../lib/procfs_iface.h"



/*
 *  NOTE: Interface helper is a generic interface tester. It requires the
 *        test suite of an interface to implement a custom `assert_session()`
 *        function that verifies the validity of an open session.
 */



//get address for testing a read / write in a PIE process
static inline uintptr_t _get_addr(mc_vm_map * m,
                                  int obj_index, int area_index, off_t off) {

    int ret;

    mc_vm_area * a;
    cm_lst_node * a_p;
    mc_vm_obj * o;
    cm_lst_node * o_p;


    //get relevant object
    o_p = cm_lst_get_n(&m->vm_objs, obj_index); 
    ck_assert_ptr_nonnull(o_p);
    o = MC_GET_NODE_OBJ(o_p);

    //get relevant area
    ret = cm_lst_get(&o->vm_area_node_ps, area_index, &a_p);
    ck_assert_int_eq(ret, 0);
    a = MC_GET_NODE_AREA(a_p);

    return (a->start_addr + off);
}



//assert open() and close() methods of an interface
void assert_iface_open_close(enum mc_iface_type iface,
                             void (* assert_session)(mc_session *, pid_t)) {

    int ret;

    pid_t pid;
    mc_session s;


    //setup tests
    pid = start_target();
    ck_assert_int_ne(pid, -1);


    //first test: open the target & close the target
    ret = mc_open(&s, iface, pid);
    ck_assert_int_eq(ret, 0);

    assert_session(&s, pid);

    ret = mc_close(&s);
    ck_assert_int_eq(ret, 0);


    //second test: re-attach to existing target and
    //             target exits before session is closed
    ret = mc_open(&s, iface, pid);
    ck_assert_int_eq(ret, 0);

    assert_session(&s, pid);
    end_target(pid);
    
    ret = mc_close(&s);
    ck_assert_int_eq(ret, 0);


    return;    
}



//procfs_update_map() [no fixture]
void assert_iface_update_map(enum mc_iface_type iface) {

    int ret;

    pid_t pid;
    mc_session s;
    mc_vm_map m;


    //setup tests
    pid = start_target();
    ck_assert_int_ne(pid, -1);

    ret = mc_open(&s, iface, pid);
    ck_assert_int_eq(ret, 0);

    mc_new_vm_map(&m);


    //first test: update empty map
    ret = mc_update_map(&s, &m);
    ck_assert_int_eq(ret, 0);

    assert_target_map(pid, &m);
    

    //second test: update filled map (map new areas)
    change_target_map(pid);

    ret = mc_update_map(&s, &m);
    ck_assert_int_eq(ret, 0);

    assert_target_map(pid, &m);


    //third test: update filled map (unmap old areas)
    change_target_map(pid);

    ret = mc_update_map(&s, &m);
    ck_assert_int_eq(ret, 0);

    assert_target_map(pid, &m);


    //fourth test: process exited
    end_target(pid);

    ret = mc_update_map(&s, &m);
    ck_assert_int_eq(ret, -1);


    //cleanup
    ret = mc_close(&s);
    ck_assert_int_eq(ret, 0);    

    ret = mc_del_vm_map(&m);
    ck_assert_int_eq(ret, 0);

    end_target(pid);

    return;   
}



//procfs_read() & procfs_write() [no fixture]
void assert_iface_read_write(enum mc_iface_type iface) {

    /*
     * NOTE: These tests work with hardcoded offsets extracted from
     *       a compiled `unit_target` binary with rizin. If you find
     *       that these tests are failing for you, verify these
     *       offsets are correct with gdb & `/proc/<pid>/maps`
     */

    int ret;

    cm_byte rw_buf[16];
    uintptr_t tgt_buf_addr;

    pid_t pid;
    mc_session s;
    mc_vm_map m;

    const char * w_buf = "buffer written  ";


    //setup tests
    pid = start_target();
    ck_assert_int_ne(pid, -1);

    ret = mc_open(&s, iface, pid);
    ck_assert_int_eq(ret, 0);

    mc_new_vm_map(&m);
    ret = mc_update_map(&s, &m);
    ck_assert_int_eq(ret, 0);


    //first test: read & write predefined rw- segment, seek & no seek
    tgt_buf_addr = _get_addr(&m, IFACE_RW_BUF_OBJ_INDEX,
                            IFACE_RW_BUF_AREA_INDEX, IFACE_RW_BUF_OFF);
    
    //read foreign buffer
    ret = mc_read(&s, tgt_buf_addr, rw_buf, TARGET_BUF_SZ);
    ck_assert_int_eq(ret, 0);

    //check foreign buffer was read correctly
    ret = strncmp((char *) rw_buf, IFACE_RW_BUF_STR, TARGET_BUF_SZ);
    ck_assert_int_eq(ret, 0);


    //write to foreign buffer (first half)
    ret = mc_write(&s, tgt_buf_addr, (cm_byte *) w_buf, TARGET_BUF_SZ);
    ck_assert_int_eq(ret, 0);

    //re-read foreign buffer
    ret = mc_read(&s, tgt_buf_addr, rw_buf, TARGET_BUF_SZ);
    ck_assert_int_eq(ret, -1);

    //check the write was performed correctly
    ret = strncmp((char *) rw_buf, w_buf, TARGET_BUF_SZ);
    ck_assert_int_eq(ret, 0);
    


    //second test: read & write to region with no read & write permissions
    tgt_buf_addr = _get_addr(&m, IFACE_NONE_OBJ_INDEX,
                            IFACE_NONE_AREA_INDEX, IFACE_NONE_OFF);

    //write to foreign buffer
    ret = mc_write(&s, tgt_buf_addr, (cm_byte *) w_buf, TARGET_BUF_SZ);
    INFO_PRINT("[%s][no perm]<mc_write()>: returned %d\n",
               get_iface_name(iface), ret);
    
    //read foreign buffer
    ret = mc_read(&s, tgt_buf_addr, rw_buf, TARGET_BUF_SZ);
    INFO_PRINT("[%s][no perm]<mc_read()>: returned %d\n",
               get_iface_name(iface), ret);    

    //check if write succeeded
    ret = strncmp((char *) rw_buf, IFACE_RW_BUF_STR, TARGET_BUF_SZ);
    INFO_PRINT("[%s][none perm]<write check>: returned %d\n",
               get_iface_name(iface), ret);    



    //third test: read & write to unmapped memory
    tgt_buf_addr = 0x1337;

    //write to foreign buffer
    ret = mc_write(&s, tgt_buf_addr, (cm_byte *) w_buf, TARGET_BUF_SZ);
    INFO_PRINT("[%s][unmapped]<mc_write()>: returned %d\n",
               get_iface_name(iface), ret);    
    
    //read foreign buffer
    ret = mc_read(&s, tgt_buf_addr, rw_buf, TARGET_BUF_SZ);
    INFO_PRINT("[%s][unmapped]<mc_read()>: returned %d\n",
               get_iface_name(iface), ret);    

    ret = strncmp((char *) rw_buf, IFACE_RW_BUF_STR, TARGET_BUF_SZ);
    INFO_PRINT("[%s][none perm]<write check>: returned %d\n",
               get_iface_name(iface), ret);    


    //cleanup
    ret = mc_close(&s);
    ck_assert_int_eq(ret, 0);    

    ret = mc_del_vm_map(&m);
    ck_assert_int_eq(ret, 0);

    end_target(pid);

    return;
    
} END_TEST
