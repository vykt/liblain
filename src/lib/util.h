#ifndef UTIL_H
#define UTIL_H

#include <libcmore.h>


//external
char * ln_pathname_to_basename(char * pathname);
int ln_pid_by_name(const char * comm, cm_vector * pid_vector);
int ln_name_by_pid(pid_t pid, char * name_buf);
void ln_bytes_to_hex(cm_byte * inp, int inp_len, char * out);


#endif
