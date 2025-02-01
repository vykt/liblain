//standard library
#include <stdio.h>
#include <stdbool.h>

//system headers
#include <unistd.h>
#include <signal.h>

#include <linux/limits.h>

#include <sys/wait.h>

//external libraries
#include <check.h>

//local headers
#include "target_helper.h"
#include "map_helper.h"

//test target headers
#include "../lib/memcry.h"



//globals
static enum target_map_state target_state;

#define TARGET_AREAS_UNCHANGED 22
char areas_unchanged[TARGET_AREAS_UNCHANGED][NAME_MAX] = {
    "",
    "unit_target",
    "unit_target",
    "unit_target",
    "unit_target",
    "unit_target",
    "",
    "libc.so.6",
    "libc.so.6",
    "libc.so.6",
    "libc.so.6",
    "libc.so.6",
    "",
    "",
    "ld-linux-x86-64.so.2",
    "ld-linux-x86-64.so.2",
    "ld-linux-x86-64.so.2",
    "ld-linux-x86-64.so.2",
    "ld-linux-x86-64.so.2",
    "[stack]",
    "[vvar]",
    "[vdso]"
};

#define TARGET_OBJS_UNCHANGED 7
char objs_unchanged[TARGET_OBJS_UNCHANGED][NAME_MAX] = {
    "0x0",
    "unit_target",
    "libc.so.6",
    "ld-linux-x86-64.so.2",
    "[stack]",
    "[vvar]",
    "[vdso]"
};

#define TARGET_AREAS_MAPPED 33
char areas_mapped[TARGET_AREAS_MAPPED][NAME_MAX] = {
    "",
    "unit_target",
    "unit_target",
    "unit_target",
    "unit_target",
    "unit_target",
    "[heap]",
    "libz.so.1.2.13",
    "libz.so.1.2.13",
    "libz.so.1.2.13",
    "libz.so.1.2.13",
    "libz.so.1.2.13",
    "libelf-0.188.so",
    "libelf-0.188.so",
    "libelf-0.188.so",
    "libelf-0.188.so",
    "libelf-0.188.so",
    "",
    "libc.so.6",
    "libc.so.6",
    "libc.so.6",
    "libc.so.6",
    "libc.so.6",
    "",
    "",
    "ld-linux-x86-64.so.2",
    "ld-linux-x86-64.so.2",
    "ld-linux-x86-64.so.2",
    "ld-linux-x86-64.so.2",
    "ld-linux-x86-64.so.2",
    "[stack]",
    "[vvar]",
    "[vdso]"
};

#define TARGET_OBJS_MAPPED 10
char objs_mapped[TARGET_OBJS_MAPPED][NAME_MAX] = {
    "0x0",
    "unit_target",
    "[heap]",
    "libz.so.1.2.13",
    "libelf-0.188.so",
    "libc.so.6",
    "ld-linux-x86-64.so.2",
    "[stack]",
    "[vvar]",
    "[vdso]"
};

#define TARGET_AREAS_UNMAPPED 23
char areas_unmapped[TARGET_AREAS_UNMAPPED][NAME_MAX] = {
    "",
    "unit_target",
    "unit_target",
    "unit_target",
    "unit_target",
    "unit_target",
    "[heap]",
    "",
    "libc.so.6",
    "libc.so.6",
    "libc.so.6",
    "libc.so.6",
    "libc.so.6",
    "",
    "",
    "ld-linux-x86-64.so.2",
    "ld-linux-x86-64.so.2",
    "ld-linux-x86-64.so.2",
    "ld-linux-x86-64.so.2",
    "ld-linux-x86-64.so.2",
    "[stack]",
    "[vvar]",
    "[vdso]"
};

#define TARGET_OBJS_UNMAPPED 8
char objs_unmapped[TARGET_OBJS_UNMAPPED][NAME_MAX] = {
    "0x0",
    "unit_target",
    "[heap]",
    "libc.so.6",
    "ld-linux-x86-64.so.2",
    "[stack]",
    "[vvar]",
    "[vdso]"
};



//signal handlers
static void _sigusr1_handler() {

    if (target_state == UNCHANGED) {
        target_state = MAPPED;
        
    } else if (target_state == MAPPED) {
        target_state = UNMAPPED;

    }

    return;
}



//helpers
pid_t start_target() {

    int ret;
    __sighandler_t ret_s;

    pid_t target_pid, parent_pid;
    char pid_buf[8];
    
    char * argv[3] = {TARGET_PATH, 0, 0};
    target_state = UNCHANGED;
    

    //get current pid to pass to target
    parent_pid = getpid();
    snprintf(pid_buf, 8, "%d", parent_pid);
    argv[1] = pid_buf;

    //fork a new process
    target_pid = fork();
    ck_assert_int_ne(target_pid, -1);

    //change image to target in child
    if (target_pid == 0) {

        ret = execve(TARGET_PATH, argv, NULL);
        ck_assert_int_ne(ret, -1);

    //if parent, register signal handler for child
    } else {
        
        ret_s = signal(SIGUSR1, _sigusr1_handler);
        ck_assert(ret_s != SIG_ERR);
    }
    
    return target_pid;
}



void end_target(pid_t pid) {

    int ret;
    __sighandler_t ret_s;

    pid_t ret_p;


    //unregister signal handler
    ret_s = signal(SIGUSR1, SIG_DFL);

    //terminate target process
    ret = kill(pid, SIGTERM);
    ck_assert_int_eq(ret, 0);

    //wait for it to terminate
    ret_p = waitpid(pid, NULL, 0);
    ck_assert_int_eq(ret_p, pid);

    return;
}



void change_target_map(pid_t pid) {

    int ret;
    __sighandler_t ret_s;

    enum target_map_state old_state = target_state;


    //assert the target hasn't performed all map transformations already
    ck_assert(target_state != UNMAPPED);

    //send SIGUSR1 to the target process to request a change in its memory map
    ret = kill(pid, SIGUSR1);
    ck_assert_int_eq(ret, 0);

    //busy-wait for target to change its memory map
    while(target_state == old_state) {}

    return;
}



void assert_target_map(pid_t pid, mc_vm_map * map) {

    switch(target_state) {

        case UNCHANGED:

            assert_vm_map_areas_aslr(&map->vm_areas, (char **) areas_unchanged,
                                     0, TARGET_AREAS_UNCHANGED);
            assert_vm_map_objs_aslr(&map->vm_objs, (char **) objs_unchanged,
                                    0, TARGET_OBJS_UNCHANGED);
            break;

        case MAPPED:

            assert_vm_map_areas_aslr(&map->vm_areas, (char **) areas_mapped,
                                     0, TARGET_AREAS_MAPPED);
            assert_vm_map_objs_aslr(&map->vm_objs, (char **) objs_mapped,
                                    0, TARGET_OBJS_MAPPED);
            break;

        case UNMAPPED:

            assert_vm_map_areas_aslr(&map->vm_areas, (char **) areas_unmapped,
                                     0, TARGET_AREAS_UNMAPPED);
            assert_vm_map_objs_aslr(&map->vm_objs, (char **) objs_unmapped,
                                    0, TARGET_OBJS_UNMAPPED);
            break;

        
    } //end switch
    
    return;    
}
