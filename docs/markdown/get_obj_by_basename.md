## get\_obj\_by\_basename()

```c
int get_obj_by_basename(char * basename, maps_data * m_data, 
                        maps_obj ** matched_obj);
```

### description
`get_obj_by_basename()` locates a backing object `matched_obj` in the provided `m_data` structure with a matching `basename` string. Use this function to locate backing objects by name.

### parameters
- `*basename`        : name of the backing file for the queried region.
- `*m_data`          : `maps_data` structure to search.
- `**matched_obj`    : pointer to a backing object inside `m_data`.

### return value
`0` on success, `-1` on no object found, `-2` on fail.
