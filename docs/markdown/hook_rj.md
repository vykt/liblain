## hook\_rj()

```c
uint32_t hook_rj(rel_jump_hook rj_hook_dat, int fd_mem);
```

### description
`hook_rj()` hooks an existing 32 bit relative jump by changing its offset to point elsewhere. NOTE: write permissions for the region are required. See `change_region_perms()`.

### parameters
- `rj_hook_dat` : hooking data.
- `fd_mem`      : open file descriptor for `/proc/<pid>/mem` for the target process.

### return value
returns the old relative jump offset on success, `0` on fail.
