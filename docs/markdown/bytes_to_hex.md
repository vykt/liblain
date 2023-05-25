## bytes\_to\_hex()

```c
void bytes_to_hex(byte * inp, int inp_len, char * out);
```

### description
`bytes_to_hex()` takes a byte input and converts it to its hexadecimal representation, storing it in a string.

### parameters
- `*inp`    : input byte buffer.
- `inp_len` : length of the input byte buffer.
- `*out`    : allocated output string buffer, should be double the size of `inp_len`.

### return value
void.
