#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "vector.h"


//Set element's data
int vector_set(vector_t * v, unsigned long pos, char * data) {

	//Check for NULL
	if (v == NULL || data == NULL) {
		return NULL_ERR;
	}

	//Check if asking to set out of bounds
	if (pos >= v->length) {
		return OUT_OF_BOUNDS_ERR;
	}

	//Copy data to vector entry
	memcpy(v->vector + (pos * v->data_size), data, v->data_size);
	
	return SUCCESS;
}


//Add element
int vector_add(vector_t * v, unsigned long pos, char * data, unsigned short append) {

	//Check for NULL
	if (v == NULL || v->data_size == 0) {
		return NULL_ERR;
	}

	//Check if max capacity reached
	if (v->length == UINT64_MAX) {
		return FULL_ERR;
	}

	//Check if asking for data out of bounds
	if (pos > v->length && append == VECTOR_APPEND_FALSE) {
		return OUT_OF_BOUNDS_ERR;
	}

	//Check for append
	if (append == VECTOR_APPEND_TRUE) {
		pos = v->length;
	}

	v->length = v->length + 1;
	v->vector = realloc(v->vector, (size_t) (v->length * v->data_size));
	if (v->vector == NULL) {
		return MEM_ERR;
	}

	//Move vector entries	
	if (pos != v->length - 1) {	
		memmove(
			v->vector + ((pos + 1) * v->data_size),
			v->vector + (pos * v->data_size),
			((v->length - 1 - pos) * v->data_size)
		);
	}

	//Copy data over if necessary
	if (data != NULL) {
		memcpy(v->vector + (pos * v->data_size), data, v->data_size);
	}

	return SUCCESS;
}


//Remove element
int vector_rmv(vector_t * v, unsigned long pos) {

	//Check for NULL
	if (v == NULL) {
		return NULL_ERR;
	}

	//Check if vector is empty
	if (v->length == 0) {
		return EMPTY_ERR;
	}

	//Check if asking for data out of bounds
	if (pos >= v->length) {
		return OUT_OF_BOUNDS_ERR;
	}

	if (pos < v->length - 1) {
		memmove(
			v->vector + (pos * v->data_size),
			v->vector + ((pos + 1) * v->data_size),
			(v->length - 1 - pos) * v->data_size
		);
	} //End if

	v->length = v->length - 1;
	v->vector = realloc(v->vector, v->length * v->data_size);

	return SUCCESS;
}


//Get element
int vector_get(vector_t * v, unsigned long pos, char * data) {

	//Check for NULL
	if (v == NULL) {
		return NULL_ERR;
	}

	//Check if vector is empty
	if (v->length == 0) {
		return EMPTY_ERR;
	}

	//Check if asking for data out of bounds
	if (pos >= v->length) {
		return OUT_OF_BOUNDS_ERR;
	}

	//Allocate space and clear buffer;
	if ((data = realloc(data, v->data_size)) == NULL) {
		return NULL_ERR;
	}
	memset(data, 0, v->data_size);

	//Get value
	memcpy(data, v->vector + (pos * v->data_size), v->data_size);

	return SUCCESS;
}


//Get element by reference
int vector_get_ref(vector_t * v, unsigned long pos, char ** data) {
	
	//Check for NULL
	if (v == NULL) {
		return NULL_ERR;
	}

	//Check if vector is empty
	if (v->length == 0) {
		return EMPTY_ERR;
	}

	//Check if asking for data out of bounds
	if (pos >= v->length) {
		return OUT_OF_BOUNDS_ERR;
	}

	*data = v->vector + (pos * v->data_size);

	return SUCCESS;
}


//Get index of element in vector by value
int vector_get_pos_by_dat(vector_t * v, char * data, unsigned long * pos) {

	//Check for NULL
	if (v == NULL || data == NULL) {
		return NULL_ERR;
	}

	int ret;
	char * data_cmp;

	//Try get position
	for (unsigned long i = 0; i < v->length; ++i) {
		ret = vector_get_ref(v, i, &data_cmp);
		if (ret != SUCCESS) return ret;

		//Here, compare only the sockaddr_in6 part, not the last_ping
		ret = memcmp(data, data_cmp, sizeof((struct sockaddr_in6 *) data));
		if (ret) continue;
		*pos = i;
		return SUCCESS;
	}

	return FAIL;
}


//Move element
int vector_mov(vector_t * v, unsigned long pos, unsigned long pos_new) {

	if (pos == pos_new) return SUCCESS;

	int ret;

	char * data = malloc(v->data_size);
	if (data == NULL) return MEM_ERR;

	//Get data of both indexes
	ret = vector_get(v, pos, data);
	if (ret != SUCCESS) { free(data); return ret; }

	//If moving towards beginning
	if (pos > pos_new) {
		memmove(v->vector+((pos_new + 1) * v->data_size),
				v->vector+(pos_new * v->data_size),
				v->data_size * (pos - pos_new));
	//else moving towards end
	} else {
		memmove(v->vector+(pos * v->data_size),
				v->vector+((pos + 1) * v->data_size),
			    v->data_size * (pos_new - pos));
	} //End if moving towards beginning/end

	ret = vector_set(v, pos_new, data);
	if (ret != SUCCESS) { free(data); return ret; }

	free(data);
	return SUCCESS;
}


//Start vector
int new_vector(vector_t * v, size_t data_size) {

	//Check NULL
	if (v == NULL || data_size == 0) {
		return NULL_ERR;
	}

	v->vector = malloc(0);
	v->data_size = data_size;
	v->length = 0;

	return SUCCESS;
}


//End vector, free memory
int del_vector(vector_t * v) {

	//Check NULL
	if (v == NULL) {
		return NULL_ERR;
	}

	free(v->vector);

	return SUCCESS;
}
