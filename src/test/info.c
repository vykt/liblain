//local includes
#include "../lib/memcry.h"



//mc_iface_type names
static char * _iface_names[2] = {
    "PROCFS",
    "KRNCRY"
};



//convert enum to name
char * get_iface_name(enum mc_iface_type iface) {

    return _iface_names[(int) iface];
}
