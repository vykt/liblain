### LIBRARY
Lain memory manipulation library (liblain, -llain)


### SYNOPSIS
```c
_Thread_local int ln_errno;

void ln_perror();
const char * ln_strerror(const int ln_errnum);
```


### STRUCTURE
When a **liblain** function return a value indicating an error (typically -1 or NULL), the integer *ln_errno* is set to hold a unique error number that describes the error that occurred. Each error number has a corresponding textual description.  
  
There are 3 classes of error numbers:  

- *21XX* : Errors caused by the user of the library.
- *22XX* : Errors caused by an internal bug in the library.
- *23XX* : Errors caused by the environment (such as a failure to allocate memory).  
  
The *cm_errno* value is stored in thread-local storage. Setting it in one thread does not affect its value in any other thread.


### FUNCTIONS
The **ln_perror()** function outputs the textual description of the last error to occur, stored in *ln_errno*, to standard error.  
  
The **ln_strerror()** takes a error number and returns a pointer to the textual description for the said error number.
  
The *ln_errno* value is stored in thread-local storage. Setting it in one thread does not affect its value in any other thread.
  

### EXAMPLES
None provided. See **perror**(3) and **strerror**(3) for identical functionality.
  

### SEE ALSO
**liblain_iface**(3), **liblain_map**(3), **liblain_util**(3)
