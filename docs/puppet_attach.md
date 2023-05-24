## puppet\_attach()

```c
int puppet_attach(puppet_info p_info);
```

### description
`puppet_attach()` attaches as a tracer to the process specified by `p_info`. 

### parameters
- `p_info` : tracee information structure.

### return value
`0` on success, `-1` on fail.
