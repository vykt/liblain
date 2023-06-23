## start\_thread()

```c
int start_thread(puppet_info * p_info, int fd_mem, new_thread_setup n_t_setup,
                 int * tid);
```

### description
`start_thread()` starts a new thread in the target process by 

### parameters
- `*p_info`   : puppet process structure pointer.
- `fd_mem`    : open file descriptor for the target process' `/proc/<pid>/mem`.
- `n_t_setup` : `new_thread_setup` structure, refer to structs.md.
- `*tid`      : thread ID pointer, filled with new thread's ID on success.

### return value
`0` on success, `-1` on fail.
