## new\_name\_pid()

```c
int new_name_pid(name_pid * n_pid, char * name);
```

### description
`new_name_pid()` initiates a new `name_pid` structure.

### parameters
- `*n_pid` : pointer to a `name_pid` structure.
- `*name`  : name of the target process.

### return value
`0` on success, `-1` on fail.
