## get\_caves()

```c
int get_caves(maps_entry * m_entry, int fd_mem, int min_size, cave * first_cave);
```

### description
`get_caves()` populates `maps_entry->caves_vector` with caves that are at least `min_size` bytes in size.

### parameters
- `*m_entry`    : entry for the target segment.
- `fd_mem`      : open file descriptor for `/proc/<pid>/mem` for the target process.
- `min_size`    : minimum size of a cave to search for in bytes.
- `*first_cave` : pointer to a `cave` structure where the first found cave will be copied to.

### return value
number of found caves is returned on success, `-1` on fail.

### notes
The search is performed by searching for continuous 0x00 or 0xFF bytes.
