## sig\_cont()

```c
int sig_cont(pid_t pid);
```

### description
`sig_cont()` sends a SIGCONT signal to the target process. This is a wrapper for ptrace.

### parameters
- `pid` : PID of the target process.

### return value
`0` on success, `-1` on fail.
