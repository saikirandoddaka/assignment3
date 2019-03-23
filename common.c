#include <stdio.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#include "common.h"

void deallocate() {
	struct shmid_ds shmid_ds;
	int result;

	result = shmctl(shmid, IPC_RMID, &shmid_ds);
	EXIT_ON_ERROR(result, "shmctl")
}

void allocate(int with_cleanup) {
	key_t key;
	int shmflg;
    shmid = shmget (KEY, 1024 * 1024, IPC_CREAT | 0666);

	EXIT_ON_ERROR(shmid, "shmget")
	if (with_cleanup)
		atexit(deallocate);
}

void get_shmid() {
	key_t key;
	int shmflg;
    shmid = shmget (KEY, 1024 * 1024, 0666);

	EXIT_ON_ERROR(shmid, "shmget")
}

void attach() {
    get_shmid();
    shm = shmat(shmid, NULL, 0);
	EXIT_ON_ERROR(shm, "shmat")
}

void detach() {
	int result;

	result = shmdt(shm);
	EXIT_ON_ERROR(result, "shmdt")
}

char *shm_data() {
    return (char*)(&shm->inputs + shm->num);
}
