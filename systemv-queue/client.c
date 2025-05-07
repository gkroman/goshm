#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>

// Same settings on the server and client
#define MAX_TEXT 256
#define MSG_KEY  12345 // NOTE; Should use ftok() in real application

struct MsgBuf {
    long mtype;
    char mtext[MAX_TEXT];
};

int main(int argc, char *argv[]) {
    int           msgid;
    struct MsgBuf message;

    if (argc != 2) {
        fprintf(stderr, "Usage: %s <message>\n", argv[0]);
        exit(1);
    }

    // Try to access the message queue
    msgid = msgget(MSG_KEY, 0);
    if (msgid == -1) {
        perror("msgget (queue not found)");
        exit(1);
    }

    // Prepare and send the message
    message.mtype = 1;
    strncpy(message.mtext, argv[1], MAX_TEXT - 1);
    message.mtext[MAX_TEXT - 1] = '\0'; // Ensure null termination

    if (msgsnd(msgid, &message, strlen(message.mtext) + 1, 0) == -1) {
        perror("msgsnd");
        exit(1);
    }

    printf("Message sent.\n");
    return 0;
}

