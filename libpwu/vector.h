#ifndef VECTOR_H
#define VECTOR_H

/*
 *	Pulled from vykt/netnote
 */

#include <sys/types.h>

#define SUCCESS 0
#define FAIL 1

#define FULL_ERR 2
#define EMPTY_ERR 3
#define OUT_OF_BOUNDS_ERR 4
#define MEM_ERR 5
#define NULL_ERR 6

#define VECTOR_APPEND_TRUE 1
#define VECTOR_APPEND_FALSE 0


typedef struct vector vector_t;

struct vector {

	char * vector;
	size_t data_size;
	unsigned long length;

};


int vector_set(vector_t * v, unsigned long pos, char * data);
int vector_add(vector_t * v, unsigned long pos, char * data, unsigned short append);
int vector_rmv(vector_t * v, unsigned long pos);
int vector_get(vector_t * v, unsigned long pos, char * data);
int vector_get_ref(vector_t * v, unsigned long pos, char ** data);
int vector_get_pos_by_dat(vector_t * v, char * data, unsigned long * pos);
int vector_mov(vector_t * v, unsigned long pos, unsigned long pos_new);

//SUCCESS, NULL_ERR
int new_vector(vector_t * v, size_t data_size);
//SUCCESS, NULL_ERR
int del_vector(vector_t * v);


#endif
