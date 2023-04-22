# C Webserver
An HTTP webserver written in C

## Implementation
The current implementation will accept the connection in the "Main Process Server", and then send the connected socket to a child process server to handle the connection.
Child process servers will handle up to 10 connections in a seperate process.

```mermaid
graph TD;
    client["Client"]
    main["Main Process Server"];
    child1["Child Process Server 1"];
    child2["Child Process Server 2"];
    client--Port 8080-->main
    main--Unix Socket-->child1;
    main--Unix Socket-->child2;
```

# Links
- [How to use rr](notes/rr.md)
- [How to use coredumps](notes/coredump.md)
- [How to use gdb](notes/gdb.md)