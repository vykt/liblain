## symbol\_resolve()

```c
int resolve_symbol(char * symbol, sym_resolve s_resolve,
                   maps_entry ** matched_region, unsigned int * matched_offset);
```

### description
`resolve_symbol()` returns a segment and an offset at which a dynamically linked symbol appears in the target process. This is done by calculating the offset in the host process, and translating it (with ASLR in mind) to the target process. The library must be opened in the host process manually with a call to `open_lib()`.

### parameters
- `*symbol`          : null terminated symbol string.
- `s_resolve`        : symbol resolve structure, refer to structs.md.
- `**matched_region` : pointer to a matched segment pointer, written to on success.
- `*matched_offset`  : offset pointer, written to on success.

### return value
`0` on success, `-1` on no symbol found, `-2` on fail.
