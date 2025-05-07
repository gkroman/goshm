// server.c
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <unistd.h>

#define SHM_SIZE (2*1024*1024)
#define SHM_KEY 12345 // 0x3039
#define SEM_KEY 12345
#define SEM_CLIENT_CAN_WRITE 0
#define SEM_SERVER_CAN_READ  1

int   g_semid = 0;
int   g_shmid = 0;
char *g_shm   = NULL;

int cleanup()
{
    if (g_shmid == 0) { return 0; }
    // Delete shared memory segment

    if (shmctl(g_shmid, IPC_RMID, NULL) == -1) {
        perror("shmctl(IPC_RMID) failed");
        return 1;
    }

    // Detatch shared memory
    if (g_shm != NULL && shmdt(g_shm) == -1) {
        perror("shmdt()");
        return 1;
    }

    // Delete semaphores
    if (g_semid != 0) {
      semctl(g_semid, 0, IPC_RMID);
    }

    printf("\nExiting.\n");
    return 0;
}

// Make sure to delete shared memory segment on Ctrl+C
void signal_handler(int sign __attribute__((unused)))
{
    exit(cleanup());
}

void sem_op(int sem_id, int sem_num, int op) {
    struct sembuf operation = {sem_num, op, 0};
    if (semop(sem_id, &operation, 1) < 0) {
        perror("semop failed");
        exit(1);
    }
}

int main()
{
    signal(SIGINT, signal_handler);

    // Create new segment
    if ((g_shmid =
                shmget(SHM_KEY, SHM_SIZE, IPC_CREAT | IPC_EXCL | 0666)) < 0) {
        perror("shmget");
        exit(1);
    }

    printf("Server started. Created %d bytes shared memory segment.\n",
           SHM_SIZE);

    // Attach segment to our data space
    if ((g_shm = shmat(g_shmid, NULL, 0)) == (char *) -1) {
        perror("shmat");
        exit(1);
    }

    printf("Attached to shared memory segment.\nStart polling...\n");

    // Get and initalize semaphores
    if ((g_semid = semget(SEM_KEY, 2, IPC_CREAT | 0666)) < 0) {
        perror("semget failed");
        exit(1);
    }

    // Initialize semaphores: client can write (1), server waits (0)
    semctl(g_semid, SEM_CLIENT_CAN_WRITE, SETVAL, 1);
    semctl(g_semid, SEM_SERVER_CAN_READ, SETVAL, 0);

    printf("Semaphores are initialized\n");

    *g_shm = '\0';
    while (1) {
        printf("Server: wait for client indicating server can read\n");
        // Wait until client writes data
        sem_op(g_semid, SEM_SERVER_CAN_READ, -1);
        printf("Server: received data: %s\n", g_shm);
        if (strncmp(g_shm, "stop", 4) == 0) { break; }
        // Process data
        char *w = g_shm;
        g_shm[SHM_SIZE - 1] = '\0'; // Guarantee end of string

        while (*w != '\0') {
            // Skip whitespaces
            for (; *w != '\0' && strchr(" \t\n\r", *w) != NULL; ++w);
            if (*w == '\0') { break; }
            // Uppercase first letter of the word
            if (*w >= 'a' && *w <= 'z') { *w = (*w - 'a' + 'A'); }
            // Skip non-whitespaces
            for (; *w != '\0' && strchr(" \t\n\r", *w) == NULL; ++w);
        }
        printf("Processed %d bytes.\n", (int)(w - g_shm));

        // Indicate client can write new data
        sem_op(g_semid, SEM_CLIENT_CAN_WRITE, 1);
    }

    printf("Stopping\n");
    cleanup(); // Delete and detatch memory segment
    return 0;
}
