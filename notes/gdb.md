## GDB
TODO: Write some instructions about debugging with GDB
```gdb
set detach-on-fork off
info inferiors
info threads
# https://sourceware.org/gdb/onlinedocs/gdb/Forks.html
set follow-fork-mode [child|parent]
show follow-fork-mode
```