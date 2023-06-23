## get\_symbol\_addr()

```c
void * get_symbol_addr(char * symbol, sym_resolve s_resolve);
```

### description
`get_symbol_addr()` returns the address of a symbol within the callee's process. This can be used to calculate symbol offsets for use in the target process.

### parameters
- `*symbol`   : null terminated symbol string.
- `s_resolve` : symbol resolve structure, refer to structs.md

### return value
address of the symbol on success, NULL on fail.
