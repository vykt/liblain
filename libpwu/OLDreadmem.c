#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <limits.h>

#include <linux/limits.h>

#include <sys/wait.h>
#include <sys/ptrace.h>

#define LINE_LEN 256

/*
 *  This is a stack overflow hack turned into a proper program. In no way is this
 *  well organised
 */


void dump_memory_segment(int p_mem_fd, unsigned long start_addr, off_t len) {

	long page_len;
	unsigned char * page;

	page_len = sysconf(_SC_PAGESIZE);
	if (page_len == -1) {
		fprintf(stderr, "Couldn't fetch page size.\n");
		exit(1);
	}

	page = malloc(page_len);
	if (page == NULL) {
		fprintf(stderr, "Malloc failed.\n");
		exit(1);
	}

	if (lseek(p_mem_fd, (off_t) start_addr, SEEK_SET) == -1) {
		fprintf(stderr, "lseek error.\n");
		exit(1);
	}

	for (unsigned long i = start_addr; i < start_addr + len; i += page_len) {
		//printf("\ntrying to read %ld bytes, when pages are %ld long\n", len, page_len);
		read(p_mem_fd, page, page_len);
		fwrite(page, 1, page_len, stdout);
	}

	free(page);
}


void get_num(char line[LINE_LEN], unsigned long * start_addr, unsigned long * end_addr) {

	int second = 0;
	int start_count = 0;
	int end_count = 0;
	char buf_start[LINE_LEN / 2] = {0};
	char buf_end[LINE_LEN / 2] = {0};

	for (int i = 0; i < LINE_LEN; ++i) {

		if (line[i] == '-') {second = 1; continue;}
		if (line[i] == ' ') break;

		if(!second) {
			buf_start[start_count] = line[i];
			++start_count;
		} else {
			buf_end[end_count] = line[i];
			++end_count;
		}
	}

	*start_addr = (unsigned long) strtol(buf_start, NULL, 16);
	*end_addr = (unsigned long) strtol(buf_end, NULL, 16);
	printf("\nstart: %lx, end: %lx\n", (long) (*start_addr), (long) (*end_addr));
}


int main(int argc, char **argv) {

    if (argc != 2) {
        fprintf(stderr, "use: %s <pid>\n", argv[0]);
        exit(1);
	}

	int pid = atoi(argv[1]);
	long ptrace_ret = ptrace(PTRACE_ATTACH, pid, NULL, NULL);

	if (ptrace_ret == -1) {
		fprintf(stderr, "Attachment failed.\n");
		exit(1);
	}
	wait(NULL);

	char line[LINE_LEN];

	unsigned long start_addr = 0;
	unsigned long end_addr = 0;

	char maps_filename[PATH_MAX] = {0};
	char mem_filename[PATH_MAX] = {0};
	
	FILE* p_maps_stream;
	int p_mem_fd;

	sprintf(maps_filename, "/proc/%s/maps", argv[1]);
	sprintf(mem_filename, "/proc/%s/mem", argv[1]);
	p_maps_stream = fopen(maps_filename, "r");
	p_mem_fd = open(mem_filename, O_RDWR);

	while (fgets(line, LINE_LEN, p_maps_stream) != NULL) {

		get_num(line, &start_addr, &end_addr);
		dump_memory_segment(p_mem_fd, start_addr, end_addr - start_addr);
	}

	fclose(p_maps_stream);
	close(p_mem_fd);

	ptrace(PTRACE_CONT, pid, NULL, NULL);
	ptrace(PTRACE_DETACH, pid, NULL, NULL);

	return 0;
}
