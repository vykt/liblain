## get\_obj\_by\_pathname()

```c
int get_obj_by_pathname(char * pathname, maps_data * m_data, 
                        maps_obj ** matched_obj);
```

### description
`get_obj_by_pathname()` locates a backing object `matched_obj` in the provided `m_data` structure with a matching `pathname` string. Use this function to locate backing objects by name.

### parameters
- `*pathname`        : name of the backing file for the queried region.
- `*m_data`          : `maps_data` structure to search.
- `**matched_obj`    : pointer to a backing object inside `m_data`.

### return value
`0` on success, `-1` on no object found, `-2` on fail.
