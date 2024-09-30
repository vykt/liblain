#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include <unistd.h>

#include <libcmore.h>

#include "../lib/liblain.h"
#include "test.h"



//run test for an interface specified by _iface_
void test_iface(pid_t pid, int iface, ln_vm_map * vm_map, uintptr_t test_addr) {

    printf("\n\n --- [IFACE] --- \n\n");
    printf("notice: use a debugger\n");

    int ret;
    size_t bytes;

    ln_session session;
    cm_byte buf[0x2000];

    //prepare write buffer
    memset(buf, (int) 'c', 0x2000);

    //open the map
    ret = ln_open(&session, iface, pid);
    
    //initialise the map
    ret = ln_update_map(&session, vm_map);
   
    //wait for target to change
    printf("press enter when target's map has changed: ");
    char cont[8];
    fgets(cont, 8, stdin);
    fgets(cont, 8, stdin);

    //update the map
    ret = ln_update_map(&session, vm_map);

    //read & write
    bytes = ln_write(&session, test_addr, buf, 0x2000);
    bytes = ln_read(&session, test_addr, buf, 0x2000);

    //close session
    ret = ln_close(&session);

    return;
}
