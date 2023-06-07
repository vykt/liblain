#include <string.h>

#include "libpwu.h"
#include "mutate.c"
#include "vector.h"

//apply mutations to payload
int apply_mutations(byte * payload_buffer, vector mutation_vector) {

	int ret;
	mutation * mutation_ref;

	//for every mutation
	for (int i = 0; i < mutation_vector.length; ++i) {

		//get mutation
		ret = vector_get_ref(&mutation_vector, i, (byte **) &mutation_ref);
		if (ret == -1) return -1;

		memcpy(payload_buffer + mutation_ref->offset, mutation_ref->mod,
		       mutation_ref->mod_len);

	} //end for every mutation

	return 0;
}
