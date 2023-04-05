#include <stdio.h>
#include "../libpwu/libpwu.h"

int main() {

	void * x = 0;
	void * y = 0;

	char line[256] = "16-16 gjaiasaeads\n";
	int ret = get_addr_range(line, &x, &y);
	printf("ret: %d\nx: %lu\ny: %lu\n", ret, (unsigned long) x, (unsigned long) y);

	return 0;

}
