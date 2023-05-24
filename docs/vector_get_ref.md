## vector\_get\_ref()

```c
int vector_get_ref(vector * v, unsigned long pos, byte ** data);
```

### description
`vector_get_ref()` retrieves a pointer to an element in a vector.

### parameters
- `*v`     : pointer to a `vector` structure.
- `pos`    : index of item to get a reference for.
- `**data` : pointer to the item at `pos` index inside the vector's heap allocation.

### return value
`0` on success, `-1` on fail.
