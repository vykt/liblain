#ifndef VECTOR_H
#define VECTOR_H

/*
 *	Pulled from vykt/netnote
 */

#include <sys/types.h>

#include "libpwu.h"

//external
int new_vector(vector * v, size_t data_size);
int del_vector(vector * v);
int vector_set(vector * v, unsigned long pos, byte * data);
int vector_add(vector * v, unsigned long pos, byte * data, unsigned short append);
int vector_rmv(vector * v, unsigned long pos);
int vector_get(vector * v, unsigned long pos, byte * data);
int vector_get_ref(vector * v, unsigned long pos, byte ** data);

//internal
int vector_set(vector * v, unsigned long pos, byte * data);
int vector_get_pos_by_dat(vector * v, byte * data, unsigned long * pos);
int vector_mov(vector * v, unsigned long pos, unsigned long pos_new);

#endif
