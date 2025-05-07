package main

import (
	"flag"
	"fmt"
	"os"
	"syscall"
	"unsafe"
)

const MSG_MAX = 256
const MSG_KEY = 12345

type MsgBuf struct {
	mtype int64
	mtext [MSG_MAX] byte
}

func main() {
	// Define short flags
	message := flag.String("m", "", "Message to send")
	flag.Parse()

	if *message == "" {
		fmt.Fprintf(os.Stderr, "Usage: %s -m <message>\n", os.Args[0])
		flag.PrintDefaults()
		os.Exit(1)
	}

	if len(*message) >= MSG_MAX {
		fmt.Fprintf(os.Stderr, "Message too long (max %d characters)\n",
				    MSG_MAX-1)
		os.Exit(1)
	}

	// Get message queue ID
	msgid, _, errno := syscall.Syscall(syscall.SYS_MSGGET, MSG_KEY, 0, 0)
	if int(msgid) == -1 {
		fmt.Fprintf(os.Stderr, "msgget failed: %v\n", errno)
		os.Exit(1)
	}

	// Prepare message
	var msg MsgBuf
	msg.mtype = 1
	copy(msg.mtext[:], *message)

	// Send message
	_, _, errno = syscall.Syscall6(
		syscall.SYS_MSGSND,
		msgid,
		uintptr(unsafe.Pointer(&msg)),
		uintptr(len(*message)+1),
		0, 0, 0,
	)

	if errno != 0 {
		fmt.Fprintf(os.Stderr, "msgsnd failed: %v\n", errno)
		os.Exit(1)
	}

	fmt.Println("Message sent.")
}
