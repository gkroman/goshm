# GO:Shared memory and IPC Examples
Minimalistic practical example of using UNIX shared memory API for interprocess communication between Go and C programms

## Setup
We have `Server` and `Client` applications which will be communicating over shared memory using System V IPC.
`Server` is a C program, and for `Client` we have the original C program as well as Go program simulating the same behaviour.  
Although, different OSes may have additional features, for this example we will focus on basic UNIX IPC API which are well supported in both Linux and macos.

## System V API
One of the oldest standards for shared memory and IPC in UNIX systems.  
Still well supported by different OSes, including Linux and macOS.

### Tooling
OS also provides a command-line tool to facilitate System V message artifacts - `ipcs`

#### List all open messages queues in the sytem                                 
Run `ipcs -q`                                                                   
Result will look something like below (on Mac):                                 
```bash                                                                         
roman@home systemv % ipcs -q                                                    
IPC status from <running system> as of Sun Apr  6 22:23:27 JST 2025             
T     ID     KEY        MODE       OWNER    GROUP                               
Message Queues:                                                                 
q 131072 0x53108ecb --rw-rw-rw- roman    staff                                  
```                                                                             
