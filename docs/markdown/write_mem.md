## write\_mem()

```c
int write_mem(int fd, void * addr, bute * write_buf, int len);
```

### description
`write_mem()` writes to the memory of a process.

### parameters
- `fd`        : open file descriptor for `/proc/<pid>/mem` for the target process.
- `*addr`     : address to write to.
- `*read_buf` : write buffer.
- `len`       : write buffer length.

### return value
`0` on success, `-1` on fail.
