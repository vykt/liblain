#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include <linux/limits.h>

#include "../libpwu/libpwu.h"

int main() {

	//tests
	int ret;

	maps_data m_data;
	maps_obj m_obj;
	maps_entry m_entry;

	ret = new_maps_data(&m_data);

	FILE * fp = fopen("/proc/12555/maps", "r");

	ret = read_maps(&m_data, fp);
	for (int i = 0; i < m_data.obj_vector.length; ++i) {
		ret = vector_get(&m_data.obj_vector, i, (char *) &m_obj);
		for (int j = 0; j < m_obj.entry_vector.length; ++j) {
			ret = vector_get(&m_obj.entry_vector, j, (char *) &m_entry);
		
			printf("perms: %s - saddr: %lx - eaddr: %lx - pathname: %s\n",
			       m_entry.perms, (unsigned long) m_entry.start_addr, 
				   (unsigned long) m_entry.end_addr, m_entry.pathname);
		}
	}

	ret = del_maps_data(&m_data);
	
	return 0;

}
