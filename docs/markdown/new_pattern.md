## new\_pattern()

```c
int new_pattern(pattern * ptn, maps_entry * search_region, byte * bytes_ptn,
                int bytes_ptn_len);
```

### description
`new_pattern()` initialises a new pattern structure.

### parameters
- `*ptn`           : pointer to a pattern struct to initialise.
- `*search_region` : pointer to the segment to search.
- `*bytes_ptn`     : byte pattern as a string.
- `*bytes_ptn_len` : length of the byte pattern in bytes.

### return value
`0` on success, `-1` on fail.
