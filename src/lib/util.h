#ifndef UTIL_H
#define UTIL_H

//external libraries
#include <cmore.h>


#ifdef DEBUG
//internal
void _line_to_name(const char * line_buf, char * name_buf);
int _get_status_name(char * name_buf, const pid_t pid);
#endif


//external
const char * mc_pathname_to_basename(const char * pathname);
int mc_pid_by_name(const char * comm, cm_vct * pid_vector);
int mc_name_by_pid(const pid_t pid, char * name_buf);
void mc_bytes_to_hex(const cm_byte * inp, const int inp_len, char * out);

#endif
