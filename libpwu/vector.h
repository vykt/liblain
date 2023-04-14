#ifndef VECTOR_H
#define VECTOR_H

/*
 *	Pulled from vykt/netnote
 */

#include <sys/types.h>

#define APPEND_TRUE 1
#define APPEND_FALSE 0


typedef struct {

	char * vector;
	size_t data_size;
	unsigned long length;

} vector;


int vector_set(vector * v, unsigned long pos, char * data);
int vector_add(vector * v, unsigned long pos, char * data, unsigned short append);
int vector_rmv(vector * v, unsigned long pos);
int vector_get(vector * v, unsigned long pos, char * data);
int vector_get_ref(vector * v, unsigned long pos, char ** data);
int vector_get_pos_by_dat(vector * v, char * data, unsigned long * pos);
int vector_mov(vector * v, unsigned long pos, unsigned long pos_new);

//0, -1
int new_vector(vector * v, size_t data_size);
//0, -1
int del_vector(vector * v);


#endif
