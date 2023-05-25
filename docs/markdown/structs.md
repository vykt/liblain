# data structures

## vector

```c
typedef struct {
        
        byte * vector;
        size_t data_size;
        unsigned long length;

} vector;
```

### description
`vector` is this library's vector implementation.

### elements
- `vector`    : pointer to heap allocation holding the vector.
- `data_size` : size of each element.
- `length`    : number of elements in the vector.

### functions
- `vector_get()`
- `vector_get_ref()`

<br>

## maps\_entry 

```c
typedef struct {

        //read_maps()
        char pathname[PATH_MAX];
        byte perms;
        void * start_addr;
        void * end_addr;
                
        //get_caves()
        vector cave_vector; //cave

} maps_entry;
```

### description
`maps_entry` represents a single line in a `/proc/<pid>/maps` file for a single process, sorted into the desired components. Data specific to each segment is also stored here. Created automatically.

### elements
- `pathname`    : name of the backing file for this segment. refer to `proc(5)`.
- `perms`       : permissions for the region in the format taken by `mprotect(2)`.
- `start_addr`  : address of the start of this segment in `/proc/<pid>/mem`.
- `end_addr`    : address of the end of this segment in `/proc\<pid\>/mem`.
- `cave_vector` : vector of `cave` struct.

### functions
- `get_caves()`

<br>

## maps\_obj

```c
typedef struct {

        char name[PATH_MAX];
        vector entry_vector; //*maps_entry

} maps_obj;
```

### description
`maps_obj` is a 'backing file'/pathname based view of `map_entry` structures. It holds the backing file and every segment that belongs to it. See `proc(5)`. Created automatically.

### elements
- `name`         : name of the backing file.
- `entry_vector` : vector of pointers to `maps_entry` structures belonging to this backing file.

### functions
none.

<br>

## maps\_data

```c
typedef struct {

        vector obj_vector; //maps_obj
        vector entry_vector; //maps_entry

} maps_data;

```

### description
`maps_data` is the <b>overarching</b> data structure representing the entire `/proc/<pid>/maps` file for a single process. Requires initialisation. Filled by `read_maps()`.

### elements
- `obj_vector`   : vector storing a backing file oriented view of segments.
- `entry_vector` : vector storing segments as they appear in `/proc/<pid>/maps`.

### functions
- `new_maps_data()`
- `del_maps_data()`
- `read_maps()`

<br>

## pattern

```c
typedef struct {

    maps_entry * search_region;
    byte pattern_bytes[PATTERN_LEN];
    int pattern_len;
    vector offset_vector;

} pattern;
```

### description
`pattern` contains members related to performing a byte pattern search on a memory segment. Requires initialisation.

### elements
- `search_region`   : segment to carry out the search on.
- `pattern_bytes`   : pattern of bytes to search for.
- `pattern_len`     : length of the pattern of bytes to search for.
- `offset_vector`   : vector of offsets at which the pattern occurs (first byte).

### functions
- `new_pattern()`
- `del_pattern()`
- `match_pattern()`

<br>

## cave

```c
typedef struct {

    unsigned int offset;
    int size;

} cave;
```

### description
`cave` stores data about areas of unused memory where payloads may be injected. Created automatically inside `maps_entry` by `get_caves()`.

### elements
- `offset` : offset at which the cave begins (first byte).
- `size`   : size of the cave, in bytes.

### functions
- `get_caves()`

<br>

## raw\_injection

```c
typedef struct {

    maps_entry * target_region;
    unsigned int offset;

    byte * payload;
    unsigned int payload_size;

} raw_injection;
```

### description
`raw_injection` stores data for injecting a payload at an arbitrary offset inside a region. Requires initialisation.

### elements
- `target_region` : `maps_entry` segment where the injection will take place.
- `offset`        : offset at which to begin injection (first byte).
- `payload`       : pointer to heap allocated space holding the payload.
- `payload_size`  : size of the payload on the heap in bytes.

### functions
- `new_raw_injection()`
- `del_raw_injection()`
- `raw_inject()`

<br>

## rel\_jump\_hook

```c
typedef struct {

    maps_entry * from_region;
    uint32_t from_offset; //address of jump instruction

    maps_entry * to_region;
    uint32_t to_offset;

} rel_jump_hook;
```

### description
`rel_jump_hook` stores data for hooking an existing 4 byte relative jump and changing the offset to jump to another location. Set manually.

### elements
- `from_region` : `maps_entry` segment where the target relative jump is located.
- `from_offset` : offset at which the relative jump begins (first byte).
- `to_region`   : `maps_entry` segment where the target relative jump will now jump to.
- `to_offset`   : offset to which the target relative jump will now jump to inside the `to_region` segment.

### functions
- `hook_rj()`

<br>

## name\_pid

```c
typedef struct {

    char name[NAME_MAX];
    vector pid_vector; //pid_t

} name_pid;

```

### description
`name_pid` stores the name of a target process and a vector of all processes that match this name. `pid_vector` is populated by `pid_by_name()`. Requires initialisation.

### elements
- `name`       : name of the target process
- `pid_vector` : vector of process IDs that match `name`.

### functions
- `new_name_pid()`
- `del_name_pid()`
- `pid_by_name()`

<br>

## puppet\_info

```c
typedef struct {

    pid_t pid;

    struct user_regs_struct saved_state;
    struct user_fpregs_struct saved_float_state;

    struct user_regs_struct new_state;
    struct user_fpregs_struct new_float_state;

} puppet_info;

```

### description
`puppet_info` stores data required to attach to a process and change the permissions of its segments. Set `pid` manually, the rest is for internal use.

### elements
- `pid`               : target process ID.
- `saved_state`       : registers at time of puppet.
- `saved_float_state` : floating point registers at time of puppet.
- `new_state`         : registers for `mprotect` syscall.
- `new_float_state`   : floating point registers for `mprotect` call.

### functions
- `puppet_attach()`
- `puppet_detach()`
- `puppet_save_regs()`
- `puppet_write_regs()`
- `change_region_perms()`
