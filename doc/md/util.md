### LIBRARY
Lain memory manipulation library (liblain, -llain)


### SYNOPSIS
```c
char * ln_pathname_to_basename(char * pathname);
pid_t ln_pid_by_name(const char * basename, cm_vector * pid_vector);
int ln_name_by_pid(pid_t pid, char * name_buf);
void ln_bytes_to_hex(cm_byte * inp, int inp_len, char * out);
```


### STRUCTURE
**liblain** provides some utility functions.


### FUNCTIONS
The **ln_pathname_to_basename()** function returns a pointer to the basename component of the provided *pathname*.

The **ln_pid_by_name()** function returns an allocated vector *pid_vector* of process IDs with a name matching *basename*. On success, *pid_vector* must be manually deallocated with **cm_del_vector()**. See **libcmore_vector**(3).

The **ln_name_by_pid()** function stores the basename (comm) inside *name_buf*. The name is fetched from */proc/<pid>/status* to mimic behaviour of utilities like *top* and *ps*.

The **ln_bytes_to_hex()** function converts a binary buffer *inp* into its hexadecimal string representation, prefixed by '0x' and stored in the *out* buffer. Note that the length of *out* must be *inp_len* \* 2 + 2.


### RETURN VALUES
**ln_pathname_to_basename()** returns a pointer to the basename on success, and NULL on error.

**ln_pid_by_name()** returns the first pid in *pid_vector* on success, and -1 on error.

**ln_name_by_pid()** returns 0 on success, and -1 on error.

On error, *ln_errno* is set. See **liblain_error**(3).


### EXAMPLES
See *src/test/util.c* for examples.
  

### SEE ALSO
**liblain_error**(3), **liblain_iface**(3), **liblain_map**(3)
