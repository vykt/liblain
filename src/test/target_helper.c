//standard library
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

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
#include "suites.h"

//test target headers
#include "../lib/memcry.h"


//globals
static enum target_map_state target_state;

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
    "[vvar]",
    "[vdso]",
    "ld-linux-x86-64.so.2",
    "ld-linux-x86-64.so.2",
    "ld-linux-x86-64.so.2",
    "ld-linux-x86-64.so.2",
    "ld-linux-x86-64.so.2",
    "[stack]"
};

char objs_unchanged[TARGET_OBJS_UNCHANGED][NAME_MAX] = {
    "0x0",
    "unit_target",
    "libc.so.6",
    "[vvar]",
    "[vdso]",
    "ld-linux-x86-64.so.2",
    "[stack]"
};


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
    "[vvar]",
    "[vdso]",
    "ld-linux-x86-64.so.2",
    "ld-linux-x86-64.so.2",
    "ld-linux-x86-64.so.2",
    "ld-linux-x86-64.so.2",
    "ld-linux-x86-64.so.2",
    "[stack]"
};

char objs_mapped[TARGET_OBJS_MAPPED][NAME_MAX] = {
    "0x0",
    "unit_target",
    "[heap]",
    "libz.so.1.2.13",
    "libelf-0.188.so",
    "libc.so.6",
    "[vvar]",
    "[vdso]",
    "ld-linux-x86-64.so.2",
    "[stack]"
};


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
    "[vvar]",
    "[vdso]",
    "ld-linux-x86-64.so.2",
    "ld-linux-x86-64.so.2",
    "ld-linux-x86-64.so.2",
    "ld-linux-x86-64.so.2",
    "ld-linux-x86-64.so.2",
    "[stack]"
};

char objs_unmapped[TARGET_OBJS_UNMAPPED][NAME_MAX] = {
    "0x0",
    "unit_target",
    "[heap]",
    "libc.so.6",
    "[vvar]",
    "[vdso]",
    "ld-linux-x86-64.so.2",
    "[stack]"
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
int clean_targets() {

    int ret;
    char command_buf[TARGET_KILL_CMD_LEN];
    

    //build command string
    snprintf(command_buf, TARGET_KILL_CMD_LEN,
             TARGET_KILL_CMD_FMT, TARGET_NAME);

    //use system() to kill all existing targets
    ret = system(command_buf);
    if (ret == -1) return -1;

    return 0;
}


pid_t start_target() {

    int ret;
    __sighandler_t ret_s;

    pid_t target_pid, parent_pid;
    char pid_buf[8];
    
    char * argv[3] = {TARGET_NAME, 0, 0};
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
        ret = execve(TARGET_NAME, argv, NULL);
        ck_assert_int_ne(ret, -1);

    //parent registers signal handler for child
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
    if (_DEBUG_ACTIVE) {
        if (target_state == MAPPED) target_state = UNMAPPED;
        if (target_state == UNCHANGED) target_state = MAPPED;
    } else {
        while (target_state == old_state) {}
    }
    
    return;
}


void assert_target_map(mc_vm_map * map) {

    switch(target_state) {

        case UNCHANGED:
            assert_vm_map_areas_aslr(&map->vm_areas, areas_unchanged,
                                     0, TARGET_AREAS_UNCHANGED, true);
            assert_vm_map_objs_aslr(&map->vm_objs, objs_unchanged,
                                    0, TARGET_OBJS_UNCHANGED, true);
            break;

        case MAPPED:
            assert_vm_map_areas_aslr(&map->vm_areas, areas_mapped,
                                     0, TARGET_AREAS_MAPPED, true);
            assert_vm_map_objs_aslr(&map->vm_objs, objs_mapped,
                                    0, TARGET_OBJS_MAPPED, true);
            break;

        case UNMAPPED:
            assert_vm_map_areas_aslr(&map->vm_areas, areas_unmapped,
                                     0, TARGET_AREAS_UNMAPPED, true);
            assert_vm_map_objs_aslr(&map->vm_objs, objs_unmapped,
                                    0, TARGET_OBJS_UNMAPPED, true);
            break;
        
    } //end switch
    
    return;    
}
