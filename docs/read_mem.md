## read\_mem()

```c
int read_mem(int fd, void * addr, byte * read_buf, int len);
```

### description
`read_mem()` reads the memory of a process.

### parameters
- `fd`        : open file descriptor for `/proc/<pid>/mem` for the target process.
- `*addr`     : address to read from.
- `*read_buf` : read buffer.
- `len`       : read buffer length.

### return value
`0` on success, `-1` on fail.
