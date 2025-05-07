// server.c
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>

// Same settings on the server and client
#define MAX_TEXT 256
#define MSG_KEY  12345

struct MsgBuf {
    long mtype;
    char mtext[MAX_TEXT];
};

int g_msgid = 0;

// Delete message queue
int cleanup()
{
    if (g_msgid > 0 && msgctl(g_msgid, IPC_RMID, NULL) < 0) {
        perror("msgctl(IPC_RMID)");
        return 1;
    }
    printf("Message queue %d closed\n", MSG_KEY);
    return 0;
}

// Make sure to delete message queue on Ctrl+C
void signal_handler(int sign __attribute__((unused))) {
    exit(cleanup());
}

int main()
{
    struct MsgBuf message;

    signal(SIGINT, signal_handler);

    // Create a new message queue with key = MSG_KEY
    if ((g_msgid = msgget(MSG_KEY, IPC_CREAT | IPC_EXCL | 0666)) < 0) {
        perror("msgget");
        exit(1);
    }

    printf("Server started and message queue %d created.\n", MSG_KEY);

    do {
        // Receive messages of any type
        if (msgrcv(g_msgid, &message, sizeof(message.mtext), 0, 0) == -1) {
            perror("msgrcv");
            cleanup();
            exit(1);
        }
        printf("Received: %s\n", message.mtext);
    } while(strncmp("stop", message.mtext, 4) != 0);

    return cleanup();
}
