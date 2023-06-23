#define _GNU_SOURCE

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>


//main function will loop before and after our new thread is started
int main() {

	while (1) {

		printf("This is tid: %d\n", gettid());
		sleep(1);
	}

	return 0;
}


//an internal function to convert an integer thread ID to a string for printing 
void int2ascii(int num, char* str) {
   int i = 0, sign = 0;
   
   if (num < 0) {
      num = -num;
      sign = 1;
   }

   do {
      str[i++] = (num % 10) + '0';
      num /= 10;
   } while (num > 0);
   
   if (sign) {
      str[i++] = '-';
   }
   
   str[i] = '\0';
   int j, len = strlen(str);
   for(j = 0; j < i / 2; j++){
       char t = str[j];
       str[j] = str[i - 1 - j];
       str[i - 1 - j] = t;
   }
}


//our new thread will begin at this function
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
