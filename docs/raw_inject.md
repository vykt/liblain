## raw\_inject()

```c
int raw_inject(raw_injection r_injection_dat, int fd_mem);
```

### description
`raw_inject()` injects a paylaod at an arbitrary address. NOTE: write permissions are required. See `change_region_perms()`.

### parameters
- `r_injection_dat` : injection information.
- `fd_mem`      : open file descriptor for `/proc/<pid>/mem` for the target process.

### return value
`0` on success, `-1` on fail.
