package main

import (
    "log"
    "bufio"
    "bytes"
    "strings"
    "fmt"
    "os"
    "time"
    "golang.org/x/sys/unix"

)

const (
    SHM_KEY = 12345
)

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

    // Make sure to detach memory
    defer func() {
        if err = unix.SysvShmDetach(shm); err != nil {
            log.Fatal("sysv detach failed:", err);
        }
    }()

    // Main loop
    scanner := bufio.NewScanner(os.Stdin)
    for scanner.Scan() {
        text := scanner.Text()
        copy(shm[1:], []byte(text))
        shm[len(text) + 1] = 0
        shm[0] = '*'

        if strings.HasPrefix(text, "stop") { break }

        for shm[0] == '*' {
            time.Sleep(100 * time.Microsecond)
        }

        fmt.Println(string(shm[1 : 1 + bytes.IndexByte(shm[1:], 0)]))
    }
}
