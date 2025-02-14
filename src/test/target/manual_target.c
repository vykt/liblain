//standard library
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <termios.h>
#include <fcntl.h>
#include <signal.h>

//system headers
#include <unistd.h>
#include <dlfcn.h>
#include <time.h>


/*
 * This program will continue counting up to confirm that it is running.
 * Press `ENTER` at any point to make the program dlopen() an additional
 * library, changing it's memory map.
 */


//globals
struct termios old_term, new_term;


//Ctrl+C signal handler
void sigint_handler(int signum) {

    tcsetattr(STDIN_FILENO, TCSANOW, &old_term);
    _exit(signum);
}


//set terminal to non-blocking, non-canon, non-echo mode
void setup_terminal() {

    //clone old terminal attributes
    new_term = old_term;

    //disable canonical mode & input echo
    new_term.c_lflag &= ~(ICANON | ECHO);

    //set new terminal attributes
    tcsetattr(STDIN_FILENO, TCSANOW, &new_term);

    //disable blocking on STDIN
    fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK);

    return;
}


int main() {

    int ch;
    bool map_changed = false;


    //get old terminal attributes
    tcgetattr(STDIN_FILENO, &old_term);

    //register SIGINT handler
    signal(SIGINT, sigint_handler);

    //setup terminal
    setup_terminal();

    for (int i = 0; ++i; ) {

        printf("Target running: %d\n", i);  
        ch = getchar();

        //if `ENTER` key is pressed, dlopen() a library
        if (ch == '\n' && map_changed == false) {

            //change process maps
            dlopen("libelf.so.1", RTLD_LAZY);
            map_changed = true;

            puts("Target memory map changed.");
        }

        sleep(1);
    }

    return 0;
}
