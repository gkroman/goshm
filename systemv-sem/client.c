#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <unistd.h>

#define SHM_KEY 12345
#define SEM_KEY 12345
#define SEM_CLIENT_CAN_WRITE 0
#define SEM_SERVER_CAN_READ  1

int   g_semid = 0;
int   g_shmid = 0;
char *g_shm   = NULL;

void sem_op(int sem_id, int sem_num, int op) {
    struct sembuf operation = {sem_num, op, 0};
    if (semop(sem_id, &operation, 1) < 0) {
        perror("semop failed");
        exit(1);
    }
}

int cleanup()
{
    // Detatch shared memory
    if (g_shm != NULL && shmdt(g_shm) == -1) {
        perror("shmdt()");
        return 1;
    }
    if (g_semid != 0) {
        sem_op(g_semid, SEM_CLIENT_CAN_WRITE, 1); // Reset client semaphore
    }
    return 0;
}

// Make sure to delete shared memory segment on Ctrl+C
void signal_handler(int sign __attribute__((unused)))
{
    exit(cleanup());
}

int main()
{
    struct shmid_ds buf;

    signal(SIGINT, signal_handler);

    // Open shared memory segment (size is ignored)
    if ((g_shmid = shmget(SHM_KEY, 0, 0666)) == -1) {
        perror("shmget");
        exit(1);
    }

    // Get the size
    if (shmctl(g_shmid, IPC_STAT, &buf) == -1) {
        perror("shmctl");
        exit(1);
    }

    // Attach segment to our data space
    if ((g_shm = shmat(g_shmid, NULL, 0)) == (char *) -1) {
        perror("shmat");
        exit(1);
    }

    // Open semaphores
    if ((g_semid = semget(SEM_KEY, 2, 0666)) < 0) {
        perror("semget failed");
        exit(1);
    }

    // Wait until server signals we can write
    sem_op(g_semid, SEM_CLIENT_CAN_WRITE, -1);

    while (1) {
        // Get data from stdin straight into the shared memory
        if (fgets(g_shm, buf.shm_segsz - 1, stdin) == NULL) {
          break;
        }
        if (strncmp(g_shm, "exit", 4) == 0) { break; }
        // Signal to server that data is ready
        sem_op(g_semid, SEM_SERVER_CAN_READ, 1);
        // Wait until server signals we can write
        sem_op(g_semid, SEM_CLIENT_CAN_WRITE, -1);

        // Print the result
        printf("%s\n", g_shm);
    }

    cleanup(); // Detatch memory segment
    return 0;
}
