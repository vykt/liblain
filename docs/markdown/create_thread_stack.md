## create\_thread\_stack()

```c
int create_thread_stack(puppet_info * p_info, int fd_mem, void ** stack_addr,
                        unsigned int stack_size);
```

### description
`create_thread_stack()` allocates memory for the stack of a new thread by creating an anonymous map.

### parameters
- `*p_info`      : pointer to a puppet info structure.
- `fd_mem`       : open file descriptor for the target process' `/proc/<pid>/mem`.
- `**stack_addr` : pointer to a void pointer that will store the new stack allocation.
- `stack_size`   : how much space should be allocated for the stack.

### return value
`0` on success, `-1` on fail.
