## apply\_mutations()

```c
int apply_mutations(byte * payload_buffer, vector mutation_vector);
```

### description
`apply_mutations()` takes a vector of mutation structures and applies each mutation to an in-memory payload.

### parameters
- `*payload_buffer` : pointer to a payload buffer.
- `mutation_vector` : vector of mutation structures to apply to the payload.

### return value
`0` on success, `-1` on fail.
