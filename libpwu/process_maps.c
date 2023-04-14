#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <linux/limits.h>

#include "libpwu.h"
#include "process_maps.h"
#include "vector.h"

/*
 *	Intended use:
 *
 *	1) call new_maps_data()
 *	2) call read_maps()
 *	3) do whatever you want with the data
 *	4) call del_maps_data()
 */

//read /proc/<pid>/maps file
int read_maps(maps_data * m_data, FILE * maps_stream) {

	int ret;
	int pos;
	char line[LINE_LEN];
	maps_entry temp_m_entry;
	maps_obj temp_m_obj;

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

		//store address range in temporary entry
		ret = get_addr_range(line, &temp_m_entry.start_addr, &temp_m_entry.end_addr);
		if (ret != 0) return -1; //error reading maps file
		
		//store permissions and name of backing file in temporary entry
		ret = get_perms_name(line, temp_m_entry.perms, temp_m_entry.pathname);
		
		//if there is no pathname for a given entry, set it to tag
		if (ret != 0) strcpy(temp_m_entry.pathname, "<NO_PATHNAME>");

		//look for a matching maps_obj
		pos = entry_path_match(temp_m_entry, *m_data);
		
		//if there isn't a matching backing file that exists
		if (pos == -1) {

			//create new temporary map object
			ret = new_maps_obj(&temp_m_obj, temp_m_entry.pathname);
			if (ret != 0) return -1;
			
			//add temporary entry to temporary map object
			ret = vector_add(&temp_m_obj.entry_vector, 0, (char *) &temp_m_entry,
			                 APPEND_TRUE);
			if (ret != 0) return -1;
			
			//add temporary map object to vector array
			ret = vector_add(&m_data->obj_vector, 0, (char *) &temp_m_obj, 
			                 APPEND_TRUE);
			if (ret != 0) return -1;

		} else {
			//else append to existing object
			ret = vector_get(&m_data->obj_vector, pos, (char *) &temp_m_obj);
			if (ret != 0) return -1;
			ret = vector_add(&temp_m_obj.entry_vector, 0, (char *) &temp_m_entry,
			                 APPEND_TRUE);
			if (ret != 0) return -1;
			
			//reset value in vector in case pointer changed
			ret = vector_set(&m_data->obj_vector, pos, (char *) &temp_m_obj);
			if (ret != 0) return -1;
		}
	} //end while there are entries in /proc/<pid>/maps

	//if out of loop, all entries are read successfully
	return 0;
}


//search through m_obj_arr for a maps_obj with matching name
//return: success: index, fail: -1
int entry_path_match(maps_entry temp_m_entry, maps_data m_data) {

	int ret;
	maps_obj m_obj;

	//for every maps_obj in m_data
	for (int i = 0; i < m_data.obj_vector.length; ++i) {

		ret = vector_get(&m_data.obj_vector, i, (char *) &m_obj);
		if (ret != 0) return -1; //unable to fetch vector entry

		ret = strcmp(temp_m_entry.pathname, m_obj.name);
		if (ret == 0) {
			return (int) i; //return index into obj_vector in case of match
		}
	}
	//if no match found
	return -1;
}


//initialise new maps_data
int new_maps_data(maps_data * m_data) {

	int ret;
	ret = new_vector(&m_data->obj_vector, sizeof(maps_obj));
	if (ret != 0) return -1;

	return 0;
}


//detele maps_data, free all allocated memory inside
int del_maps_data(maps_data * m_data) {

	int ret;
	maps_obj m_obj;

	//for every maps_data object
	for (int i = 0; i < m_data->obj_vector.length; ++i) {

		ret = vector_get(&m_data->obj_vector, i, (char *) &m_obj);
		if (ret != 0) return -1; //unable to fetch vector entry

		ret = del_maps_obj(&m_obj);
		if (ret != 0) return -1; //attempting to free non-existant vector

	}

	ret = del_vector(&m_data->obj_vector);
	if (ret != 0) return -1; //unable to delete vector

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
int new_maps_obj(maps_obj * m_obj, char name[PATH_MAX]) {

	int ret;

	//first, zero out the name field
	memset(m_obj->name, '\0', PATH_MAX);

	//now copy name in
	strcpy(m_obj->name, name);

	//now, initialise vector member
	ret = new_vector(&m_obj->entry_vector, sizeof(maps_entry));
	if (ret == -1) return -1;
	return 0;
}


//delete maps_obj
int del_maps_obj(maps_obj * m_obj) {

	//this is mostly a wrapper for old vector functions
	int ret;
	ret = del_vector(&m_obj->entry_vector);
	if (ret == -1) return -1;
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

		if(!next) {
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
	memset(perms, '\0', PERMS_LEN);

	//get to end of line
	int i = 0;
	int j = 0;
	size_t name_len;
	int column_count = 0;
	while (line[i] != '\0' && i < LINE_LEN-1) {
		
		//if at permissions
		if(column_count == 1) {
			strncpy(perms, &line[i], PERMS_LEN-1);
			i+=4;
		}

		//if reached the void between offset and name
		if (column_count == 5) {
			if (line[i] == '\0') break;
			if (line[i] == ' ') {
				++i;
				continue;
			}
			name[j] = line[i];
			++j;
		}

		if (line[i] == ' ') {++column_count;}
		++i;
	} //end get to end of line
	
	//if name exists
	if (j) {

		name_len = strlen(name);
		name[name_len-1] = '\0';
		return 0;
	//if no name exists
	} else {
		return -1;
	}
}
