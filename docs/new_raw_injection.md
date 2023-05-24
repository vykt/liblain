## new\_raw\_injection()

```c
int new_raw_injection(raw_injection * r_injection_dat, maps_entry * target_region,
                      unsigned int offset, char * payload_filename);
```

### description


### parameters
- `*r_injection_dat`  : pointer to a structure storing injection data.
- `*target_region`    : pointer to the target segment.
- `offset`            : offset into the segment at which to inject.
- `*payload_filename` : path to the payload file on disk.

### return value
`0` on success, `-1` on fail.
