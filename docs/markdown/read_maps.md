## read\_maps()

```c
int read_maps(maps_data * m_data, FILE * maps_stream);
```

### description
`read_maps` populates the `m_data` data structure with the contents of `/proc/\<pid\>/maps`, excluding caves.

### parameters
- `*m_data` : pointer to an initialised `maps_data` structure.
- `*maps_stream` : open file stream for `/proc/<pid>/maps`.

### return value
`0` on success, `-1` on fail.
