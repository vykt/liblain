#ifndef UTIL_H
#define UTIL_H

#include "libpwu.h"

/* inp's length is inp_len
 * out's length is (2*inp_len) - 1
 * '0x beginning is omitted' from out
 */
void bytes_to_hex(byte * inp, int inp_len, char * out);

#endif
