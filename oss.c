
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <time.h>

#include "common.h"

/* Global variables */

time_t start_time;
int num_lines;
char **lines;

/* Arguments */
char *bin_name;
int concurrent_limit = 3;
int time_limit = 25;
char *input_file = "input.dat";
char *output_file = "log.dat";
FILE *output;

/* State */

uint total_procs;
uint concurrent_procs;
pid_t *pids;

/* Global variables done */

/* Common functions */

/* Spawn a child and log it */
void spawn_process(int index) {
	pid_t pid = fork();
    char in_str[32];
    sprintf(in_str, "%d", index);

    if (pid == 0)
        execlp("./palin", "./palin", in_str, NULL);

    pids[total_procs] = pid;
    fprintf(output, "%d %d %s\n", pid, index, lines[index]);
	total_procs++;
	concurrent_procs++;
}

/* Log child termination */
void child_terminated(pid_t pid) {
	for (int i = 0; i < total_procs; i++) {
		if (pids[i] == pid) {
			pids[i] = 0;
		}
	}
	concurrent_procs--;
}

/* Terminate the OSS process */
void terminate() {
	/* Terminate children */
	for (int i = 0; i < total_procs; i++) {
		if (pids[i] != 0) {
			kill(pids[i], 15);
		}
	}

	fclose(output);
	detach();
	exit(0);
}

void handle_sigint() {
    fprintf(stderr, "Interrupted!\n");
    terminate();
}

/* Read data into process's memory, copied into shm later when required memory is known. */
void read_data() {
	FILE *input = fopen(input_file, "r");
	if (input == NULL)
		perror("fopen input");

	lines = NULL;
	char line[1024];
	num_lines = 0;

	while (fgets(line, 1024, input)) {
		lines = realloc(lines, sizeof(char *) * (num_lines + 1));
        strtok(line, "\n");
		lines[num_lines] = strdup(line);
		num_lines++;
	}

	fclose(input);
    fprintf(stderr, "Lines read: %d\n", num_lines);
}

/* Get memory required to store the data in shm */
size_t get_size() {
    size_t size = 0;
	for (int i = 0; i < num_lines; i++)
        size += sizeof(size_t) + sizeof(char) * (strlen(lines[i]) + 1);
	return size;
}

/* Copies data from process memory into shared memory */
void copy_data_to_shm() {
    shm->num = num_lines;
    /* Pointer to inputs offset array */
    size_t *inputs = &shm->inputs;
    /* Pointer to just past the inputs array to store the lines.
     * Will be incremented to current line. */
    char *data_ptr = shm_data();
    /* Pointer to the beginning of data */
    char *data_start_ptr = data_ptr;

    for (int i = 0; i < num_lines; i++) {
        /* Copy line to shm */
        strcpy(data_ptr, lines[i]);
        /* Store offset to the line in inputs array */
        inputs[i] = data_ptr - data_start_ptr;
        /* Advance pointer */
        data_ptr += strlen(lines[i]) + 1;
    }
}

/* Initialize OSS process */
void init() {
    concurrent_procs = 0;
    total_procs = 0;
    allocate(1); /* Will call deallocate automatically */
	attach();
    sem_init(&shm->pal_sem, 1, 1);
    sem_init(&shm->nopal_sem, 1, 1);
    time(&shm->start_time);
	copy_data_to_shm();

	output = fopen(output_file, "w");
	if (output == NULL)
		perror("fopen output");

    pids = calloc(1024, sizeof(pid_t));

    signal(SIGALRM, terminate);
    signal(SIGINT, handle_sigint);

	alarm(time_limit);
}

/* Main process */

void main_proc() {
    int i = 0;
	while(1) {
        /* Check if can spawn new worker */
        if (concurrent_procs < concurrent_limit && i < num_lines)
            spawn_process(i++);

		/* Check if any children terminated */
		pid_t pid = waitpid(-1, NULL, WNOHANG);
		if (pid > 0) {
			child_terminated(pid);
		}

        /* Terminate? */
        if (concurrent_procs == 0 && i == num_lines)
            terminate();
	}

}

/* Pre-fork functions */

void show_usage() {
	printf("Usage: %s [options]\n", bin_name);
	printf("Options:\n");
	printf("	-h		Show this message\n");
    printf("	-s NUM		Set concurrent process limit. Default: 3\n");
	printf("	-i INPUT	Set input filename. Default: input.dat\n");
	printf("	-l OUTPUT	Set log filename. Default: log.dat\n");
}

void do_args(int argc, char **argv) {
	int opt;
	while ((opt = getopt(argc, argv, "hi:o:s:n:")) != -1) {
		switch(opt) {
			case 'h':
				show_usage();
				exit(0);
			case 's':
				concurrent_limit = atoi(optarg);
				if (concurrent_limit > 20)
					concurrent_limit = 20;
				break;
			case 'i':
				input_file = optarg;
				break;
			case 'o':
				output_file = optarg;
		}
	}
}

int main(int argc, char **argv) {
	bin_name = argv[0];

    unlink("palin.out");
    unlink("nopalin.out");

	do_args(argc, argv);

	/* Read input data */
	read_data();
	
	/* Dealing with shm */
	init();

	main_proc();
}
