## new\_maps\_data()

```c
int new_maps_data(maps_data * m_data);
```

### description
`new_maps_data()` allocates the necessary memory on the heap for `m_data`. 

### parameters
- `*m_data` : pointer to an uninitiated `maps_data` structure.

### return value
`0` on success, `-1` on fail.
