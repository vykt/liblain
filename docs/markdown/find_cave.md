## find\_cave()

```c
int find_cave(maps_entry * m_entry, int fd_mem, int required_size,
              cave * matched_cave);
```

### description
`find_cave()` finds a cave listed in `m_entry->cave_vector` that is of size `required_size` or bigger.

### parameters
- `*m_entry`      : segment to search for an cave of size `required_size` in.
- `fd_mem`        : open file descriptor for `/proc/<pid>mem` for the target process.
- `required_size` : minimum cave size to accept.
- `*matched_cave` : pointer to a cave that meets the criteria, filled on success.

### return value
`0` on success, `-1` on fail.
