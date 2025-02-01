#ifndef TARGET_H
#define TARGET_H

//system headers
#include <unistd.h>

//test target headers
#include "../lib/memcry.h"



/*
 * The target helper is directly tied to `target/unit_target.c`, changing
 * the target helper means you likely have to change the unit target and 
 * vice versa.
 */



//the state of the target's memory map
enum target_map_state {
    UNCHANGED, //default state
    MAPPED,    //new areas mapped
    UNMAPPED   //newly mapped areas now unmapped
};



//target metadata
#define TARGET_PATH "target"
#define TARGET_BUF_SZ 16

#define IFACE_RW_BUF_STR "read & write me "

#define IFACE_RW_BUF_OBJ_INDEX  1
#define IFACE_RW_BUF_AREA_INDEX 4
#define IFACE_RW_BUF_OFF        0x40

#define IFACE_NONE_OBJ_INDEX  0
#define IFACE_NONE_AREA_INDEX 0
#define IFACE_NONE_OFF        0x0



//target helpers
pid_t start_target();
void end_target(pid_t pid);
void change_target_map(pid_t pid);
void assert_target_map(pid_t pid, mc_vm_map * map);


#endif
