## open\_lib()

```c
int open_lib(char * lib_path, sym_resolve * s_resolve);
```

### description
`open_lib()` opens a handle on a shared object, loading into the callee's address space if not already present. The returned handle is identical to that returned by dlopen().

### parameters
- `*lib_path`  : path to the shared object.
- `*s_resolve` : symbol resolution structure, refer to structs.md.

### return value
`0` on success, `-1` on fail.
