#ifndef _PROCESS_MAPS_H
#define _PROCESS_MAPS_H

#include <stdio.h>

#include <linux/limits.h>

#include "vector.h"

#define LINE_LEN PATH_MAX + 128
#define PERMS_LEN 4


typedef struct {

	char pathname[PATH_MAX];
	char perms[PERMS_LEN];
	void * start_addr;
	void * end_addr;

} maps_entry;


typedef struct {

	char name[PATH_MAX];
	vector entry_vector; //maps_entry

} maps_obj;


typedef struct {

	vector obj_vector; //maps_obj

} maps_data;


//internal functions
int read_maps(maps_data * m_data, FILE * maps_stream);
int entry_path_match(maps_entry temp_m_entry, maps_data m_data);
int new_maps_data(maps_data * m_data);
int del_maps_data(maps_data * m_data);
int get_maps_line(char line[LINE_LEN], FILE * maps_stream);
int new_maps_obj(maps_obj * m_obj, char name[PATH_MAX]);
int del_maps_obj(maps_obj * m_obj);
int get_addr_range(char line[LINE_LEN], void ** start_addr, void ** end_addr);
int get_perms_name(char line[LINE_LEN], char perms[PERMS_LEN], char name[PATH_MAX]);


#endif
