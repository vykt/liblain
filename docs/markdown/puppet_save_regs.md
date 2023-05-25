## puppet\_save\_regs()

```c
int puppet_save_regs(puppet_info * p_info);
```

### description
`puppet_save_regs()` saves the registers of the tracee inside `p_info`.

### parameters
- `*p_info` : pointer to the tracee information structure.

### return value
`0` on success, `-1` on fail.
