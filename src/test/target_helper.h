#ifndef TARGET_HELPER_H
#define TARGET_HELPER_H

//system headers
#include <unistd.h>

//test target headers
#include "../lib/memcry.h"


/*
 * The target helper is directly tied to `target/unit_target.c`, changing
 * the target helper means you likely have to change the unit target and 
 * vice versa.
 */


#define TARGET_AREAS_UNCHANGED 22
extern char areas_unchanged[TARGET_AREAS_UNCHANGED][NAME_MAX];

#define TARGET_OBJS_UNCHANGED 7
extern char objs_unchanged[TARGET_OBJS_UNCHANGED][NAME_MAX];

#define TARGET_AREAS_MAPPED 33
extern char areas_mapped[TARGET_AREAS_MAPPED][NAME_MAX];

#define TARGET_OBJS_MAPPED 10
extern char objs_mapped[TARGET_OBJS_MAPPED][NAME_MAX];

#define TARGET_AREAS_UNMAPPED 23
extern char areas_unmapped[TARGET_AREAS_UNMAPPED][NAME_MAX];

#define TARGET_OBJS_UNMAPPED 8
extern char objs_unmapped[TARGET_OBJS_UNMAPPED][NAME_MAX];



//the state of the target's memory map
enum target_map_state {
    UNINIT,    //waiting for child to start
    UNCHANGED, //default state
    MAPPED,    //new areas mapped
    UNMAPPED   //newly mapped areas now unmapped
};


//target metadata
#define TARGET_NAME "unit_target"

#define TARGET_KILL_CMD_FMT "kill $(pidof %s) > /dev/null 2> /dev/null"
#define TARGET_KILL_CMD_LEN NAME_MAX + 64

#define TARGET_BUF_SZ 16 /* must be even */
#define IFACE_RW_BUF_STR "read & write me "
#define IFACE_W_BUF_STR  "buffer written  "

#define IFACE_RW_BUF_OBJ_INDEX  1
#define IFACE_RW_BUF_AREA_INDEX 4
#define IFACE_RW_BUF_OFF        0x60

#define IFACE_NONE_OBJ_INDEX  0
#define IFACE_NONE_AREA_INDEX 0
#define IFACE_NONE_OFF        0x0


//target helpers
int clean_targets();
pid_t start_target();
void end_target(pid_t pid);
void change_target_map(pid_t pid);
void assert_target_map(mc_vm_map * map);

#endif
