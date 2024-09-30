#include <stdio.h>
#include <stdint.h>

#include <unistd.h>

#include <linux/limits.h>

#include <libcmore.h>

#include "../lib/liblain.h"
#include "test.h"


int main() {

    int ret, iface;
    uintptr_t test_addr;
    pid_t pid;
    char comm[NAME_MAX];
    ln_vm_map vm_map;

    //test utils
    printf("target name: ");
    scanf("%s", comm);
    pid = test_utils(comm);

    ln_new_vm_map(&vm_map);

    //test ifaces
    printf("interface (0 - lainko, 1 - procfs): ");
    scanf("%d", &iface);
    printf("rw- buffer to read/write 0x2000 bytes from/to: ");
    scanf("%lx", &test_addr);
    test_iface(pid, iface, &vm_map, test_addr);

    //test map
    test_map(&vm_map);

    ret = ln_del_vm_map(&vm_map);

    return 0;
}
