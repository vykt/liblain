## close\_lib()

```c
void close_lib(sym_resolve * s_resolve);
```

### description
`close_lib()` closes an open handle on a shared object within the callee's process. If no references to the shared object remain (see man 3 dlopen), calling `close_lib()` will unload the shared object.

### parameters
- `*s_resolve` : symbol resolution structure, refer to structs.md.

### return value
Void.
