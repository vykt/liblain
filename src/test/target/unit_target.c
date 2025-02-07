//standard library
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <signal.h>

//system headers
#include <unistd.h>
#include <dlfcn.h>
#include <time.h>

#include <sys/types.h>
#include <sys/mman.h>

//local headers
#include "../target_helper.h"



/*
 * Unit target is tied directly to `../target_helper.{c,h}`. Changing this 
 * target likely necessitates also changing the target helper.
 */


/*
 * This program sleeps indefinitely, awaiting a signal from its parent,
 * which will cause it to dlopen() an additional library and hence change
 * its memory map. This is done from inside unit tests.
 */



//globals
void * libelf;
enum target_map_state state = UNCHANGED;

/*
 * These buffers are used in read/write tests for interfaces.
 */

 char rw_buf[TARGET_BUF_SZ] = IFACE_RW_BUF_STR;



//unit test signal handler
void sigusr1_handler() {

    if (state == UNCHANGED) {

        libelf = dlopen("libelf.so.1", RTLD_LAZY);
        state = MAPPED;

    } else if (state == MAPPED) {

        dlclose(libelf);
        state = UNMAPPED;
    }

    return;
}



int main(int argc, char ** argv) {

    int ch;
    pid_t parent_pid;
    void * protected_area;

    //check correct number of args is provided (quiet -Wunused-parameter)
    if (argc != 2) return -1;

    //recover parent pid
    parent_pid = atoi(argv[1]);

    //map an area that can't be accessed
    protected_area = mmap((void *) 0x10000, 0x1000, PROT_NONE,
                          MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

    //register unit test handler
    signal(SIGUSR1, sigusr1_handler);

    for (int i = 0; ++i; ) {

        //sleep for 10ms to not hoard the CPU
        usleep(100000);
    }

    return 0;
}
