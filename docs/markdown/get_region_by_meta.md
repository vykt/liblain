## get\_region\_by\_meta()

```c
int get_region_by_meta(char * pathname, int index, maps_entry ** matched_region,
                       maps_data * m_data);
```

### description
`get_region_by_meta()` finds a segment in callee's `m_data`. This segment should belong to a backing file matching `pathname`, at index `index`. For example, the `pathname` of "libc.so.6" and `index` of 1 would locate the second segment of libc.

### parameters
- `*pathname`        : name of the backing file for the queried region.
- `index`            : segment 'index' in the backing file.
- `**matched_region` : pointer to a segment pointer inside `m_data`.
- `*m_data`          : `maps_data` structure for the callee's process.

### return value
`0` on success, `-1` on no segment found, `-2` on fail.
