#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>

#include <linux/limits.h>

//DEBUG TODO
#include <sys/mman.h>

#include "../libpwu/libpwu.h"

int main() {

	printf("READ: %d\nWRITE: %d\nEXEC: %d\n", PROT_READ, PROT_WRITE, PROT_EXEC);
	printf("TOGETHER: %d\n", PROT_READ | PROT_WRITE | PROT_EXEC);
	printf("TOGETH3R: %d\n", PROT_READ + PROT_WRITE + PROT_EXEC);
}
