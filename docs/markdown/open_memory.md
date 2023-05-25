## open\_memory()

```c
int open_memory(pid_t pid, FILE ** fd_maps, int * fd_mem);
```

### description
`open_memory()` opens `/proc/<pid>/{mem,maps}` for use by other functions.

### parameters
- `pid`       : PID of the target process.
- `**fd_maps` : pointer to a file stream pointer for `/proc/<pid>/maps`.
- `*fd_mem`   : pointer to a file descriptor for `/proc/<pid>/mem`.

### return value
`0` on success, `-1` on fail.

### notes
It is not possible to mmap process memory (anonymous maps excluded).
