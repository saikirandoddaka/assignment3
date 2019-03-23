
#ifndef COMMON_H
#define COMMON_H

#include <semaphore.h>

#include "types.h"

#define KEY 19283746

#define EXIT_ON_ERROR(VAR, STR) if (VAR < 0) { \
		perror(STR); \
		exit(1); \
	}

typedef struct {
    time_t start_time;
    sem_t pal_sem;
	sem_t nopal_sem;
    size_t num;
    size_t inputs;
} shm_header;

int shmid;
shm_header *shm;

void allocate(int with_cleanup);
void deallocate();
void get_shmid();
void attach();
void detach();

char *shm_data();

#endif
