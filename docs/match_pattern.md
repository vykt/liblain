## match\_pattern()

```c
int match_pattern(pattern * ptn, int fd_mem);
```

### description


### parameters
- `*ptn`   : pointer to the `pattern` structure.
- `fd_mem` : open file descriptor for `/proc/<pid>/mem` for the target process.

### return value
number of matched patterns on success including 0, `-1` on fail.
