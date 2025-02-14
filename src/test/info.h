#ifndef INFO_H
#define INFO_H

//standard library
#include <stdio.h>

//local includes
#include "../lib/memcry.h"


#define INFO_PRINT(format, ...)\
    printf("[INFO]: " format, ##__VA_ARGS__)


char * get_iface_name(enum mc_iface_type iface);

#endif
