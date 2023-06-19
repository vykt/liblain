#define _GNU_SOURCE

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

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
		printf("This is tid: %d, inject check: %d\n", gettid(), total);
		sleep(2);
	}

	return 0;
}


void int2ascii(int num, char* str) {
   int i = 0, sign = 0;
   
   // Handle negative numbers
   if (num < 0) {
      num = -num;
      sign = 1;
   }
   
   // Build string in reverse order
   do {
      str[i++] = (num % 10) + '0';
      num /= 10;
   } while (num > 0);
   
   // Add sign if necessary
   if (sign) {
      str[i++] = '-';
   }
   
   // Null terminate the string
   str[i] = '\0';

   // Reverse the string
   int j, len = strlen(str);
   for(j = 0; j < i / 2; j++){
       char t = str[j];
       str[j] = str[i - 1 - j];
       str[i - 1 - j] = t;
   }
}


int thread_work() {

    ssize_t ret;
    pid_t tid = gettid();
    char tid_buf[24] = {};
    int2ascii(tid, tid_buf);

    while(1) {
        write(1, "This is tid: ", 13);
        write(1, tid_buf, strlen(tid_buf));
        putchar('\n');
        sleep(1);
    }
}
