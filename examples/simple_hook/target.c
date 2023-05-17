#include <stdio.h>
#include <unistd.h>


int add_nums(int x, int y) {

	int c;
	c = x + y;
	return c;
}


int main() {

	int svetas_x = 4;
	int svetas_y = 16;
	int total;

	while (1) {

		total = add_nums(svetas_x, svetas_y);
		printf("%d\n", total);
		sleep(2);
	}

	return 0;
}
