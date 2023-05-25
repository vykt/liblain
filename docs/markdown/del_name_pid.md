## del\_name\_pid()

```c
int del_name_pid(name_pid * n_pid);
```

### description
`del_name_pid()` deallocates heap memory used by a `name_pid` structure.

### parameters
- `*n_pid` : pointer to a `name_pid` structure.

### return value
`0` on success, `-1` on fail.
