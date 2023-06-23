## vector\_add()

```c
int vector_add(vector * v, unsigned long pos, byte * data, unsigned short append);
```

### description
`vector_add()` insert or append an element to a vector.

### parameters
- `*v`     : pointer to a vector to add an element to.
- `pos`    : position to add the element at.
- `*data`  : pointer to the data to add to the vector, cast to a byte pointer.
- `append` : either `APPEND_TRUE` or `APPEND_FALSE`, if true `pos` is ignored.

### return value
`0` on success, `-1` on fail.
