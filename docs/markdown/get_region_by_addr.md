## get\_region\_by\_addr()

```c
int get_region_by_addr(void * addr, maps_entry ** matched_region,
                       unsigned * int offset, int * obj_index, maps_data * m_data);
```

### description
`get_region_by_addr()` takes an address and returns the segment and offset into the segment at which this address resides in the callee. It also takes an optional pointer `obj_index`, which if not NULL contains on return the 'index' into the corresponding backing file. For example, libc may contain 4 segments. If the address is found in the second segment of libc, `obj_index` will be set to 1 on return.

### parameters
- `*addr`            : address to query.
- `**matched_region` : pointer to a segment pointer in `m_data`, filled by the call.
- `*offset`          : offset into the `matched_region` segment where `addr` occurs.
- `*obj_index`       : 'index' into the backing file.
- `*m_data`          : pointer to a `maps_data` structure for the target process.

### return value
`0` on success, `-1` on no segment found, `-2` on fail.
