#ifndef _PROCESS_MAPS_H
#define _PROCESS_MAPS_H

#include <stdio.h>

#include <linux/limits.h>

#include "libpwu.h"
#include "vector.h"

#define LINE_LEN PATH_MAX + 128


//external
int read_maps(maps_data * m_data, FILE * maps_stream);
int new_maps_data(maps_data * m_data);
int del_maps_data(maps_data * m_data);

//internal
int build_obj_vector(maps_data * m_data);
int entry_path_match(maps_entry temp_m_entry, maps_data m_data);
int get_maps_line(char line[LINE_LEN], FILE * maps_stream);
int new_maps_obj(maps_obj * m_obj, char pathname[PATH_MAX], char basename[NAME_MAX]);
int del_maps_obj(maps_obj * m_obj);
int new_maps_entry(maps_entry * m_entry);
int del_maps_entry(maps_entry * m_entry);
int get_addr_range(char line[LINE_LEN], void ** start_addr, void ** end_addr);
int get_perms_name(char line[LINE_LEN], byte * perms, char name[PATH_MAX]);


#endif
