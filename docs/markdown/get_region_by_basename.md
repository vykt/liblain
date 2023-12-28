## get\_region\_by\_path()

```c
int get_region_by_basename(char * pathname, int index, maps_data * m_data, 
                           maps_entry ** matched_region, maps_obj ** matched_obj);
```

### description
`get_region_by_basename()` finds a segment in callee's `m_data`. This segment should belong to a backing file matching `pathname`, at index `index`. For example, the `pathname` of "libc.so.6" and `index` of 1 would locate the second segment of libc.

### parameters
- `*pathname`        : name of the backing file for the queried region.
- `index`            : segment 'index' in the backing file.
- `*m_data`          : `maps_data` structure for the callee's process.
- `**matched_region` : pointer to a segment pointer inside `m_data`.
- `**matched_obj`    : pointer to a backing object inside `m_data`.

### return value
`0` on success, `-1` on no segment found, `-2` on fail.
