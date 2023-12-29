## get\_region\_by\_addr()

```c
int get_region_by_addr(void * addr, maps_entry ** matched_region,
                       unsigned * int offset, maps_data * m_data);
```

### description
`get_region_by_addr()` takes an address and returns the segment and offset into the segment at which this address occurs in `m_data`. Acquiring the underlying backing object can be done using `(*matched_region)->obj_vector_index`.


### parameters
- `*addr`            : address to query.
- `**matched_region` : pointer to a segment pointer in `m_data`, filled by the call.
- `*offset`          : offset into the `matched_region` segment where `addr` occurs.
- `*m_data`          : pointer to a `maps_data` structure for the target process.

### return value
`0` on success, `-1` on no segment found, `-2` on fail.
