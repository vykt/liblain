#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <linux/limits.h>

#include "process_maps.h"


/*
 *	Intended use:
 *
 *	1) call new_maps_data()
 *	2) call read_maps()
 *	3) do whatever you want with the data
 *	4) call del_maps_data()
 */

/*
 *	TODO TODO TODO
 *
 *	none of this is tested yet
 */

//read /proc/<pid>/maps file
int read_maps(maps_data * m_data, FILE * maps_stream) {

	int ret;
	char line[LINE_LEN];
	maps_entry temp_m_entry;

	/*	1) create temporary node, read next entry into it
	 *
	 *	2) search through nodes for matching name. return index if found, -1 if not
	 *
	 *		+if node does exist, add temporary node to its entry_vector
	 *
	 *		-if node does not exist, use next free slot to create new node with this name
	 */

	//while there are entries in /proc/<pid>/maps left to process
	while(!get_maps_line(line, maps_stream)) {

		ret = get_addr_range(line, &temp_m_entry.start_addr, &temp_m_entry.end_addr);
		if (ret != 0) return -1; //error reading maps file
		ret = get_perms_name(line, temp_m_entry.perms, temp_m_entry.pathname);
		//if there is no pathname for a given entry
		if (ret != 0) strcpy(temp_m_entry.pathname, "<NO_PATHNAME>");

		//look for a matching maps_obj
		ret = entry_path_match(temp_m_entry, *m_data);
		if (ret == -1) {
			//if there is no matching maps_obj, create new one
			ret = new_maps_obj(&m_data->m_obj_arr[m_data->len]);
			if (ret != 0) return -1;
			//now append to object
			ret = vector_add(&m_data->m_obj_arr[m_data->len].entry_vector, 0, 
					         (char *) &temp_m_entry, VECTOR_APPEND_TRUE);
			if (ret != SUCCESS) return -1;
		} else {
			//else append to existing object
			ret = vector_add(&m_data->m_obj_arr[ret].entry_vector, 0, 
					         (char *) &temp_m_entry, VECTOR_APPEND_TRUE);
			if (ret != SUCCESS) return -1;
		}
	} //end while there are entries in /proc/<pid>/maps

	//if out of loop, all entries are read successfully
	return 0;
}


//search through m_obj_arr for a maps_obj with matching name
//return: success: index, fail: -1
int entry_path_match(maps_entry temp_m_entry, maps_data m_data) {

	int ret;

	//for every maps_obj in m_data
	for (int i = 0; i < m_data.len; ++i) {

		ret = strcmp(temp_m_entry.pathname, m_data.m_obj_arr[i].name);
		if (ret == 0) {
			return i;
		}
	}
	//if no match found
	return -1;
}


//initialise new maps_data
void new_maps_data(maps_data * m_data) {

	m_data->len = 0;
	memset(m_data->m_obj_arr, 0, M_OBJ_ARR_LEN * sizeof(maps_obj));
}


//detele maps_data, free all allocated memory inside
int del_maps_data(maps_data * m_data) {

	int ret;

	//for every maps_data object
	for (int i = 0; i < m_data->len; ++i) {

		ret = del_maps_obj(&m_data->m_obj_arr[i]); 
		if (ret != 0) return -1; //attempting to free non-existant vector
	}
	return 0; //memory freed, can discard maps_data
}


//get entry from maps file
int get_maps_line(char line[LINE_LEN], FILE * maps_stream) {

	//first, zero out line
	memset(line, '\0', LINE_LEN);

	if (fgets(line, LINE_LEN, maps_stream) == NULL) {
		return -1;
	}
	return 0;
}


//initialise new maps_obj
int new_maps_obj(maps_obj * m_obj) {

	int ret;

	//first, zero out the name field
	memset(m_obj->name, '\0', PATH_MAX);

	//now, initialise vector member
	ret = new_vector(&m_obj->entry_vector, sizeof(maps_entry));
	if (ret == NULL_ERR) return -1;
	return 0;
}


//delete maps_obj
int del_maps_obj(maps_obj * m_obj) {

	//this is mostly a wrapper for old vector functions
	int ret;
	ret = del_vector(&m_obj->entry_vector);
	if (ret == NULL_ERR) return -1;
	return 0;
}


//get the address range values from a line in /proc/<pid>/maps
int get_addr_range(char line[LINE_LEN], void ** start_addr, void ** end_addr) {

	int next = 0;
	int start_count = 0;
	int end_count = 0;

	char buf_start[LINE_LEN / 2] = {0};
	char buf_end[LINE_LEN / 2] = {0};

	//for every character of line
	for (int i = 0; i < LINE_LEN; ++i) {

		if (line[i] == '-') {next = 1; continue;}
		if (line[i] == ' ') break;

		if(next) {
			buf_start[start_count] = line[i];
			++start_count;
		} else {
			buf_end[end_count] = line[i];
			++end_count;
		}

	} //end for every character of line
	
	*start_addr = (void *) strtol(buf_start, NULL, 16);
	*end_addr = (void *) strtol(buf_end, NULL, 16);
	if (*start_addr == 0 || *end_addr == 0) {
		return -1; //convertion failed
	} else {
		return 0;  //convertion successful
	}
}


//get name for line in /proc/<pid>/maps
int get_perms_name(char line[LINE_LEN], char perms[PERMS_LEN], char name[PATH_MAX]) {

	//zero out name first
	memset(name, '\0', PATH_MAX);

	//get to end of line
	int i = 0;
	int j = 0;
	int column_count = 0;
	while (line[i] != '\0' && i < LINE_LEN-1) {
		
		//if at permissions
		if(column_count == 1) {
			strncpy(name, &line[i], PERMS_LEN);
			i+=4;
		}

		//if reached the void between offset and name
		if (column_count >= 5) {
			//if reached name
			if (line[i] != ' ') {
				j = i;
				while (j != '\0') {
					name[j-i] = line[j];
					++j;
				}
			}
		}

		if (line[i] == ' ') {++column_count;}
		++i;
	} //end get to end of line
	
	//if name exists
	if (j) {
		return 0;
	//if no name exists
	} else {
		return -1;
	}
}
