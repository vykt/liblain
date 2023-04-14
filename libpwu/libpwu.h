#ifndef _LIBPWU_H
#define _LIBPWU_H

#include "process_maps.h"

//read /proc/<pid>/maps into allocated maps_data object
extern int read_maps(maps_data * m_data, FILE * maps_stream);

//returns: 0 - success, -1 - failed to allocate space
extern int new_maps_data(maps_data * m_data);

//returns: 0 - success, -1 - failed to deallocate maps_data
extern int del_maps_data(maps_data * m_data);

#endif
