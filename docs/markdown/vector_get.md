## vector\_get()

```c
int vector_get(vector * v, unsigned long pos, byte * data);
```

### description
`vector_get()` retrieves a copy of the data stored inside a vector.

### parameters
- `*v`    : pointer to a `vector` structure.
- `pos`   : index of item to get.
- `*data` : pointer to the buffer to copy the item into.

### return value
`0` on success, `-1` on fail.
