## name\_by\_pid()

```c
int name_by_pid(pid_t pid, char * name);
```

### description
`name_by_pid()` fills buffer pointed to by `name` with the name of the process taken from `/proc/pid/comm`.

### parameters
- `pid`        : PID of the process.
- `*name`      : pointer to an allocated `pid_t` integer where the first PID that matches the name provided by `n_pid` is stored. This is a shortcut.

### return value
`0` on success, '-1' on fail.
