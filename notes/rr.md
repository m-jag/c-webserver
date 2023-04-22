# RR
1. Setup RR
    1. Install RR
    ```bash
    sudo apt-get install rr
    ```
    2. edit rr.conf
        ### **`src: /etc/sysctl.d/10-rr.conf`**
        ```conf 
        # 
        kernel.perf_event_paranoid = 1
        ```
        - kernel.perf_event_paranoid - TODO: Why?
            ```bash
            rr needs /proc/sys/kernel/perf_event_paranoid <= 1, but it is 2.
            Change it to 1, or use 'rr record -n' (slow).
            Consider putting 'kernel.perf_event_paranoid = 1' in /etc/sysctl.d/10-rr.conf.
            See 'man 8 sysctl', 'man 5 sysctl.d' (systemd systems)
            and 'man 5 sysctl.conf' (non-systemd systems) for more details.
            ```
    3. Reload sysctl
        ```bash
        sudo sysctl --system
        ```

2. Add a bug to crash the server
    1. I just added a simple nullptr dereference to handle_request:
        ```c
        int * derefZero = NULL;
        *derefZero = -1;
        ```

    2. Build the server
        ```bash
        make DEBUG=1
        ```
3. Use RR to record the crash
    ```bash
    rr record ./webserver
    ```
4. Find the process we want to trace
    ```bash
    rr ps
    # an exit code that isn't 0 is more likely to be the crashing process
    ```

5. Load the replay of the crash
    1. Replay the main process
        ```bash
        rr replay
        ```
    2. Replay a child process via fork
        ```bash
        rr replay --onfork=<PID>
        # PID is from `rr ps` from above.
        ```
    3. Replay a child process via exec
        ```bash
        rr replay --onprocess=<PID>
        # PID is from `rr ps` from above.
        ```
6. Watch back the crash with time-travel debugging thanks to `rr`
    ```bash
    # We don't care about sigpipes in most cases
    gef> handle SIGPIPE nostop pass
    # Continue to the crash
    gef> c
    # Run reverse-* commands and other normal GDB commands
    ```
7. Profit

## References:
- https://rr-project.org/
- https://jade.fyi/blog/debugging-rr-children/