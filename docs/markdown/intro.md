# intro

Thanks for trying out libpwu! This quick introduction will explore some key parts of the library. In the end we will have a tool that can map out process memory and overwrite executable segments.

## mapping process memory

In order to manipulate a process, a good first step is to map out its memory. The kernel exposes memory maps for every process in the `/proc/<pid>/maps` special file. libpwu can read this file into a data structure and organise it into multiple 'views' to make it easy to work with.

The following code snippet defines a `maps_data` structure used to hold contents of `/proc/<pid>/maps`. The data structure is then initialised. After initialisation, it is passed to `read_maps()`, the function that does the reading of the maps special file.

```c
int ret;           //return integer
FILE * fs_maps;    //file stream for the /proc/<pid>/maps special file
int fd_mem;        //file descriptor for /proc/<pid>/mem special file
maps_data m_data;  //maps_data structure, holds all segments

//open 'maps' and 'mem' special files
ret = open_memory(<pid>, &fs_maps, &fd_mem);
//initialise /proc/<pid>/maps
ret = new_maps_data(&m_data);
//read /proc/<pid>/maps into the m_data structure
ret = read_maps(&m_data, &fd_maps);
```

## accessing vectors

Now that we have mapped out the memory, individual segments can be accessed through the `maps_data` structure. The segments are stored inside a custom vector implementation, represented by the `vector` structure. Each segment is represented by a `maps_entry`.

There are two functions for accessing vector elements. `vector_get()` retrieves a <i>copy</i> of the element. Modifying the copy does not modify the element inside the vector. `vector_get_ref()` returns a pointer to the element inside the vector. Modifying the value at the pointer does modify the vector contents. Note however that resizing of the vector causes a call to 'realloc()' which <i>invalidates all previous pointers to the vector</i>.

The above is best illustrated with an example. Let's continue our previous example. We can take the `m_data` structure and get its fifth segment, then print the start and end address of this segment.

```c
maps_entry * m_entry; //maps_entry structure, holds a single segment

//get a reference to the 5th segment in m_data, using the 'entry_vector' view
ret = vector_get_ref(&m_data.entry_vector, 4, (byte **) &m_entry);
//print the start and end addresses for the 5th segment
printf("start addr: %lx, end addr: %lx\n", m_entry.start_addr, m_entry.end_addr);
```

The vectors used by libpwu can hold any data type, so the buffers passed to their getter functions should be cast to `byte` (char). 


## changing segment permissions

If you'd like to modify the executable segments of a process, you'll first have to change the permissions of its executable segment to allow for writing. Unlike on Windows, on Linux processes may only change the permissions of their own segments. libpwu allows you to work around this limitation. The `puppet_attach()` function lets you attach to a target process 'tracee' as a 'tracer' (man 2 ptrace). The target process, now referred to as 'tracee',  can then be made to execute an 'mprotect' syscall which can modify segment permissions. If state is restored following this call, the tracee can continue its execution as if nothing happened, with the exception that its executable region is now writeable. libpwu can automate this process for you with the `change_region_perms()` function.

Lets continue and change the permissions of the segment we acquired in the previous example.

```c
puppet_info p_info; //puppet_info structure
p_info.pid = <pid>; //manually set the desired process ID

//attach to the target process
ret = puppet_attach(p_info);
//change the segment permissions to read, write & execute.
ret = change_region_perms(&p.inf, 7, fd_mem, &m_data, m_entry);
```

NOTE: Unlike POSIX file permissions, memory permissions are inverted. 1 = read, 2 = write, 4 = exec (man 2 mprotect).


## writing memory & exploitation

libpwu provides a multitude of options for exploiting processes. The most simple method is uses `write_mem()` to directly write memory. `raw_inject()` can be used to inject payloads from the disk. `hook_rj()` allows for hooking relative 32bit jumps used extensively by compilers.

Carrying on our chain of examples, let's assume you used a reverse engineering tool like radare2 to find the offset from the 5th segment at which you'd like to overwrite a 'CMP' instruction with 3 'NOP's. Here is how that would look:

```c
//a NOP operation is 0x90 in hex.
byte * write_buffer = "\x90\x90\x90";

//write the buffer to the m_entry segment at <offset>
ret = write_mem(fd_mem, m_entry->start_addr + <offset>, 3);
```


## clean up

It is important to cleanly deallocate all used structures when they are no longer in use. Typically, every structure that is initialise with a function prefixed with 'new_' will need to be deallocated with the corresponding destructor prefixed with 'del_'.

For our ongoing example, the clean up section will consist of the following:

```c
//delete the maps_data structure
ret = del_maps_data(&m_data);
//detatch from the target process to let it continue execution
ret = puppet_detach(p_info);
```

## other capabilities

This introduction covered just a small section of libpwu's capabilities. libpwu also provides functions for discovering code caves, pattern matching, trampolining new threads, and more. Take a look at documentation at `./docs` or read the header file at `./libpwu/libpwu.h` for a full list of capabilities.


## examples

See example implementations in `./examples`
