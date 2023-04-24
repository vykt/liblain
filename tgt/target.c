#include <stdio.h>
#include <unistd.h>


int main() {

	char * test = "hello!";
	while (1) {
		puts(test);
		sleep(1);
	}

}
