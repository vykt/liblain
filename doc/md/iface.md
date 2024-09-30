### LIBRARY
Lain memory manipulation library (liblain, -llain)


### SYNOPSIS
```c
#define LN_IFACE_LAINKO 0
#define LN_IFACE_PROCFS 1


struct _ln_session {

    union {
        struct {
            int fd_mem;
            int pid;
        }; //procfs_data
        struct {
            char major;
            int fd_dev_memu;
        }; //lainko_data
    };

    ln_iface iface;

}; 
typedef struct _ln_session ln_session;

int ln_open(ln_session * session, int iface, int pid);
int ln_close(ln_session * session);
int ln_update_map(ln_session * session, ln_vm_map * vm_map);
int ln_read(ln_session * session, uintptr_t addr, 
            cm_byte * buf, size_t buf_sz);
int ln_write(ln_session * session, uintptr_t addr,
             cm_byte * buf, size_t buf_sz);
```


### STRUCTURE
**liblain** provides 2 interfaces for operating on targets: *procfs* and *lainko*. The *procfs* interface uses Linux's inbuilt */proc* pseudo-filesystem for accessing the target. The *lainko* interface uses the **lain.ko kernel module** provided separately from this library. Both interfaces provide identical functionality. If your target does not employ any countermeasures, it is easier to stick with the *procfs* interface.

To operate on a target it must first be opened. The *ln_session* structure stores data relevant to a single open target. If you desire to operate on multiple targets at the same time, you must open multiple sessions. You can have multiple sessions utilising the same interface, and multiple sessions utilising different interfaces.

A session does not include a memory map. You are free to maintain multiple memory maps associated with a single session.


### FUNCTIONS
The **ln_open()** function opens a *session* on a target with the specified *pid*. The interface to use should be specified with the *iface* argument and should take the value of *LN_IFACE_LAINKO* or *LN_IFACE_PROCFS*.
  
The **ln_close()** function closes an opened *session*.

The **ln_update_map()** function updates the passed memory map *vm_map*. This function can be called both to populate a map for the first time, and to update it.

The **ln_read()** function reads *buf_sz* bytes at address *addr* into a buffer pointed to by *buf*,

The **ln_write()** function writes *buf_sz* bytes at address *addr* from a buffer pointed to by *buf*.


### RETURN VALUES
**ln_open()**, **ln_close()**, **ln_update_map()**, **ln_read()**, and **ln_write()** functions return 0 on success and -1 on error. 

On error, *ln_errno* is set. See **liblain_error**(3).


### EXAMPLES
See *src/test/iface.c* for examples.
  

### SEE ALSO
**liblain_error**(3), **liblain_map**(3), **liblain_util**(3)
