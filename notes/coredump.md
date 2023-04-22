## Coredump
1. Enabling coredumps
```bash
# Set the max coredump size
ulimit -S -c unlimited
# Set the coredump path
sudo sysctl -w kernel.core_pattern=/var/crash/core-%e-%s-%u-%g-%p-%t
```

2. Crash the server
    1. To achieve this for testing, I just added a simple nullptr dereference to handle_request:
        ### **`src: c-webserver/src/webserver.c`**
        ```c
        int * derefZero = NULL;
        *derefZero = -1;
        ```

    2. Run the server
        ```bash
        ./webserver
        ```

3. Read the coredump file
    ```bash
    gdb ./webserver <CORE-FILE>
    # <CORE-FILE> is a path matching `/var/crash/core-%e-%s-%u-%g-%p-%t` due to how we setup coredumps
    gef> bt
    #0  0x00005654709fdd4e in handle_request (arg=0x5654719ee6b0) at src/webserver.c:85
    #1  0x00007f6378020b43 in start_thread (arg=<optimized out>) at ./nptl/pthread_create.c:442
    #2  0x00007f63780b2a00 in clone3 () at ../sysdeps/unix/sysv/linux/x86_64/clone3.S:81
    ```

4. Profit