#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <dlfcn.h>
#include <unistd.h>

#define FILESIZE 4096 // 4 KB

int main() {
    // Open "/etc/fstab" file
    int fd = open("/etc/fstab", O_RDONLY);
    if (fd == -1) {
        perror("Error opening file for reading");
        exit(EXIT_FAILURE);
    }

    // Memory map empty 8KB
    char * fillMePtr = mmap(0, 8196, PROT_READ | PROT_WRITE, 
                            MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (fillMePtr == MAP_FAILED) {
        close(fd);
        perror("Error mmapping the file");
        exit(EXIT_FAILURE);
    }

    // Memory map the first 4KB of the file
    char *dataPtr = mmap(0, FILESIZE, PROT_READ, MAP_SHARED, fd, 0);
    if (dataPtr == MAP_FAILED) {
        close(fd);
        perror("Error mmapping the file");
        exit(EXIT_FAILURE);
    }

    // Load libutil.so.1
    void *libutil_handle = dlopen("libutil.so.1", RTLD_LAZY);
    if (!libutil_handle) {
        fprintf(stderr, "%s\n", dlerror());
        exit(EXIT_FAILURE);
    }


    // Do something with the memory mapped file
    printf("Press enter to continue: ");
    char cont;
    scanf("%c", &cont);


    // Unmap the file
    if (munmap(dataPtr, FILESIZE) == -1) {
        perror("Error un-mmapping the file");
        exit(EXIT_FAILURE);
    }

    // Close the file descriptor
    close(fd);


    // Unload libutil.so.1
    if (dlclose(libutil_handle) != 0) {
        fprintf(stderr, "%s\n", dlerror());
        exit(EXIT_FAILURE);
    }

    // Load libdl.so.2
    void *lib_handle = dlopen("libdl.so.2", RTLD_LAZY);
    if (!lib_handle) {
        fprintf(stderr, "%s\n", dlerror());
        exit(EXIT_FAILURE);
    }

    printf("Press enter to continue again: ");
    char cont2;
    scanf("%c", &cont2);

    // Unload libdl.so.2
    if (dlclose(lib_handle) != 0) {
        fprintf(stderr, "%s\n", dlerror());
        exit(EXIT_FAILURE);
    }

    // Unmap empty 8KB
    if (munmap(fillMePtr, 8192) == -1) {
        perror("Error un-mapping ");
        exit(EXIT_FAILURE);
    }

    return 0;
}

