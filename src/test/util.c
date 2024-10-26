#include <stdio.h>

#include <unistd.h>

#include <linux/limits.h>

#include <libcmore.h>

#include "../lib/liblain.h"
#include "test.h"


pid_t test_utils(char * comm) {

    printf("\n\n --- [UTILS] --- \n\n");
    
    int ret;
    pid_t pid, pid_iter;
    
    char name_buf[NAME_MAX];
    cm_byte hex_buf[4] = {0xde, 0xad, 0xc0, 0xde};
    char hexstr_buf[10];

    cm_vector pid_vector;

    //test pathname to basename conversion
    char * path = "/everything/is/connected";
    char * base = ln_pathname_to_basename(path);
    printf("basename of '%s': '%s'\n", path, base);

    //test converting process name to pid(s)
    pid = ln_pid_by_name(comm, &pid_vector);
    printf("comm: %s, pid: %d\n", comm, pid);

    for (int i = 0; i < pid_vector.len; ++i) {
        ret = cm_vector_get_val(&pid_vector, i, (cm_byte *) &pid_iter);
        printf("\npid %d: %d\n", i, pid_iter);
    } //end for
    cm_del_vector(&pid_vector);

    //test converting pid to process name 
    ret = ln_name_by_pid(pid, name_buf);
    printf("pid: %d, comm: %s\n", pid, name_buf);

    //test bytes to hex string

    ln_bytes_to_hex(hex_buf, 4, hexstr_buf);
    printf("should see 'deadcode': %s\n", hexstr_buf);

    return pid;
}
