
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <semaphore.h>
#include <time.h>

#include "common.h"

#ifdef DEBUG
#define LOG(...) { fprintf(log, __VA_ARGS__); fflush(log); }
#else
#define LOG(...) {}
#endif

#define SLEEP(SEC) { LOG("Sleeping for %d seconds\n", SEC); sleep(SEC); }

FILE *log;

/* Check if the string is a palindrome */
int check_pal(char *line) {
    for (int i = 0, j = strlen(line)-1;
         i < j;
         i++, j--) {
        if (line[i] != line[j])
            return 0;
    }
    return 1;
}

/* Write it to the file */
void write_pal(int pal, int index) {
    char *filename = pal ? "palin.out" : "nopalin.out";
    FILE *out = fopen(filename, "a");
    fprintf(out, "%d\n", index);
    fclose(out);
}

/* Process a palindrome, including semaphores and waiting */
void do_palin(int index) {
    size_t *lines = &shm->inputs;
    LOG("Getting line %d\n", lines[index]);
    char *line = shm_data() + lines[index];
    LOG("SHM: 0x%x; DATA: 0x%x; LINE: 0x%x\n", shm, shm_data(), line)
    LOG("Line is %s\n", line);
    int is_pal = check_pal(line);
    LOG("Seems to %s\n", is_pal ? "be a palindrome" : "not be a palindrome");
    sem_t *sem;

    if (is_pal)
        sem = &shm->pal_sem;
    else
        sem = &shm->nopal_sem;

    LOG("Entering loop\n")
    for (int i = 0; i < 5; i++) {
        LOG("Iteration\n");
        int secs = rand() % 4;
        SLEEP(secs);
        sem_wait(sem);
        fprintf(stderr, "Entered %s critical section at %d\n", is_pal ? "palin" : "nopalin", time(NULL) - shm->start_time);
        secs = (rand() % 2)+1;
        SLEEP(secs);
        /* Write the data only once
         * Not sure why does it even has to enter critical section five times */
        if (i == 0) {
            LOG("Writing to file\n");
            write_pal(is_pal, index);
        }
        sem_post(sem);
        fprintf(stderr, "Exited %s critical section at %d\n", is_pal ? "palin" : "nopalin", time(NULL) - shm->start_time);
    }
}

int main(int argc, char **argv) {
    uint index = atoi(argv[1]);

#ifdef DEBUG
    log = fopen(argv[1], "w");
#endif

    srand(time(NULL));

    LOG("attach\n");
    attach();

    LOG("run %d\n", index);
    do_palin(index);

#ifdef DEBUG
    fclose(log);
#endif
    return 0;
}
