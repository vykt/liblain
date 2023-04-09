#ifndef _PROCESS_MAPS_H
#define _PROCESS_MAPS_H

#include <linux/limits.h>

#include "vector.h"

#define LINE_LEN PATH_MAX + 128
#define PERMS_LEN 4
#define M_OBJ_ARR_LEN 12288 //based on benchmark of endless-sky, which has ~2300 segments

typedef struct {

	char pathname[PATH_MAX];
	char perms[PERMS_LEN]
	void * start_addr;
	void * end_addr;

} maps_entry;


typedef struct {

	char name[PATH_MAX];
	vector_t entry_vector;

} maps_obj;


typedef struct {

	int len;
	maps_obj m_obj_arr[M_OBJ_ARR_LEN];

} maps_data;


#endif
