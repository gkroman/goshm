package main

import (
    "log"
    "bufio"
    "bytes"
    "strings"
    "fmt"
    "os"
    "golang.org/x/sys/unix"
    "syscall"
    "unsafe"
)

const (
    SHM_KEY = 12345
    SEM_KEY = 12345
    SEM_CLIENT_CAN_WRITE = 0
    SEM_SERVER_CAN_READ  = 1
)

// Helper functions to work with System-V semaphore from:
// https://github.com/shubhros/drunkendeluge/blob/master/semaphore/semaphore.go
type Semaphore struct {
	semid int
	nsems int
}

type semop struct {
	semNum  uint16
	semOp   int16
	semFlag int16
}

func errnoErr(errno syscall.Errno) error {
	switch errno {
	case syscall.Errno(0):
		return nil
	default:
		return error(errno)
	}
}

func SemGet(key int, nsems int, flags int) (*Semaphore, error) {
	r1, _, errno := syscall.Syscall(syscall.SYS_SEMGET,
		uintptr(key), uintptr(nsems), uintptr(flags))
	if errno == syscall.Errno(0) {
		return &Semaphore{semid: int(r1), nsems: nsems}, nil
	} else {
		return nil, errnoErr(errno)
	}
}

func (s *Semaphore) Post(semNum int) error {
	post := semop{semNum: uint16(semNum), semOp: 1, semFlag: 0}
	_, _, errno := syscall.Syscall(syscall.SYS_SEMOP, uintptr(s.semid),
		uintptr(unsafe.Pointer(&post)), 1)
	return errnoErr(errno)

}

func (s *Semaphore) Wait(semNum int) error {
	wait := semop{semNum: uint16(semNum), semOp: -1, semFlag: 0}
	_, _, errno := syscall.Syscall(syscall.SYS_SEMOP, uintptr(s.semid),
		uintptr(unsafe.Pointer(&wait)), 1)
	return errnoErr(errno)
}

// ------------------------------------------------


func main() {
    // Get shared memory ID
    shmid, err := unix.SysvShmGet(SHM_KEY, 0, 0666)
    if err != nil {
        log.Fatal("sysv shm create failed:", err)
    }

    // Attach
    shm, err := unix.SysvShmAttach(shmid, 0, 0)
    if err != nil {
        log.Fatal("sysv shm attach failed:", err)
    }
	  fmt.Println("Size of shared memory: ", len(shm))

    sem, err := SemGet(SEM_KEY, 2, 0666)
    if err != nil {
        log.Fatal("Failed to get semaphores:", err)
    }

    if err = sem.Wait(SEM_CLIENT_CAN_WRITE); err != nil {
        log.Fatal("Failed to wait on semaphore", err);
    }

    // Make sure to detach memory
    defer func() {
        if err = unix.SysvShmDetach(shm); err != nil {
            log.Fatal("sysv detach failed:", err);
        }
        // Reset state of the client semaphore
        if err = sem.Post(SEM_CLIENT_CAN_WRITE); err != nil {
            log.Fatal("Failed to post semaphore", err);
        }
    }()

    // Main loop
    scanner := bufio.NewScanner(os.Stdin)
    for scanner.Scan() {
        text := scanner.Text()
        copy(shm, []byte(text))
        shm[len(text)] = 0

        if err = sem.Post(SEM_SERVER_CAN_READ); err != nil {
            log.Fatal("Failed to post semaphore", err);
        }

        if strings.HasPrefix(text, "stop") { break }

        if err = sem.Wait(SEM_CLIENT_CAN_WRITE); err != nil {
            log.Fatal("Failed to wait on semaphore", err);
        }

        fmt.Println(string(shm[0 : bytes.IndexByte(shm, 0)]))
    }
}
