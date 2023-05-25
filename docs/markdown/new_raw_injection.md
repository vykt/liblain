## new\_raw\_injection()

```c
int new_raw_injection(raw_injection * r_injection_dat, maps_entry * target_region,
                      unsigned int offset, char * payload_filename);
```

### description
`new_raw_injection()` initialises the `raw_injection` structure and copies the contents of the payload file into memory.

### parameters
- `*r_injection_dat`  : pointer to a structure storing injection data.
- `*target_region`    : pointer to the target segment.
- `offset`            : offset into the segment at which to inject.
- `*payload_filename` : path to the payload file on disk.

### return value
`0` on success, `-1` on fail.

### notes
The fact that the payload is copied into memory prior to injection allows you to dynamically mutate the payload on each execution.
