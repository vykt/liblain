## del\_pattern()

```c
int del_pattern(pattern * ptn);
```

### description
`del_pattern()` deallocates heap memory that is part of the `ptn` structure.

### parameters
- `*ptn` : pointer to the `pattern` structure to be deallocated.

### return value
`0` on success, `-1` on fail.
