## sig\_stop()

```c
int sig_stop(pid_t pid);
```

### description
`sig_stop()` sends a SIGSTOP signal to the target process. This is a wrapper for ptrace.

### parameters
- `pid` : PID of the target process.

### return value
`0` on success, `-1` on fail.
