#ifndef IFACE_HELPER_H
#define IFACE_HELPER_H

//standard library
#include <stdint.h>

//system headers
#include <unistd.h>

//external libraries
#include <cmore.h>

//test target headers
#include "../lib/memcry.h"


//map helper functions
void assert_iface_open_close(enum mc_iface_type iface,
                             void (* assert_session)(mc_session *, pid_t));
void assert_iface_update_map(enum mc_iface_type iface);
void assert_iface_read_write(enum mc_iface_type iface);

#endif
