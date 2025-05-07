#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <unistd.h>

#define SHM_KEY 12345

int   g_shmid = 0;
char *g_shm   = NULL;

int cleanup()
{
    // Detatch shared memory
    if (g_shm != NULL && shmdt(g_shm) == -1) {
        perror("shmdt()");
        return 1;
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

    while (fgets(g_shm + 1, buf.shm_segsz - 2, stdin) != NULL) {
        *g_shm = '*'; // Signal server, that data ready to read

        if (strncmp(g_shm + 1, "stop", 4) == 0) { break; }

        // Wait for server to process the string
        do { usleep(100); } while(*g_shm == '*');

        // Print the result
        printf("%s\n", g_shm + 1);
    }

    cleanup(); // Detatch memory segment
    return 0;
}

