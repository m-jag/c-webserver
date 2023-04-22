# RR
1. Setup RR
    1. Install RR
    ```bash
    sudo apt-get install rr
    ```
    2. edit rr.conf
        #### **`src: /etc/sysctl.d/10-rr.conf`**
        ```conf 
        # 
        kernel.perf_event_paranoid = 1
        ```
        - kernel.perf_event_paranoid - khuey (rr developer) 'rr uses the PERF_COUNT_SW_CONTEXT_SWITCHES which the kernel considers "kernel profiling" (thus requiring perf_event_paranoid <= 1 or a user with CAP_PERFMON)' (src: https://github.com/rr-debugger/rr/issues/2835)
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