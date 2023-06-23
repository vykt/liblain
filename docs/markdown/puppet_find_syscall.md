## puppet\_find\_syscall()

```c
int puppet_find_syscall(puppet_info * p_info, maps_data * m_data, fd_mem);
```

### description
`puppet_find_syscall()` locates a syscall instruction (0x0f05) in any executable segment inside the tracee, and writes its address into `p_info`.

### parameters
- `*p_info` : pointer to the tracee information structure.
- `*m_data` : `maps_data` structure for the tracee.
- `fd_mem`  : open file descriptor for tracee's `/proc/<pid>mem` file

### return value
`0` on success, `-1` on no syscall found, `-2` on failed to perform search.
