## arbitrary\_syscall()

```c
int arbitrary_syscall(puppet_info * p_info, int fd_mem, 
                      unsigned long long * syscall_ret);
```

### description
`arbitrary_syscall` executes an arbitrary syscall in the tracee. `p_info->syscall_addr` must be already set. The caller must specify the syscall and its parameters by setting `p_info->new_state` and `p_info->new_float_state` manually prior to the call. The caller may optionally provide a pointer `syscall_ret` where the return of the syscall will be written to. To disregard the return, set `syscall_ret` to NULL.

### parameters
- `*p_info`      : pointer to a puppet info structure.
- `fd_mem`       : open file descriptor for the target process' `/proc/<pid>/mem`.
- `*syscall_ret` : pointer where the return from the syscall is written to.

### return value
`0` on success, `-1` on fail.
