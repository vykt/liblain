## del\_maps\_data()

```c
int del_maps_data(maps_data * m_data);
```

### description
`del_maps_data` frees the memory allocated for the vectors of `m_data`.

### parameters
- `*m_data` : pointer to an initialised `maps_data` structure.

### return value
`0` on success, `-1` on fail.
