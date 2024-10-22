### LIBRARY
Lain memory manipulation library (liblain, -llain)


### SYNOPSIS
```c
//these macros take a cm_list_node pointer
#define LN_GET_NODE_AREA(node)  ((ln_vm_area *) (node->data))
#define LN_GET_NODE_OBJ(node)   ((ln_vm_obj *) (node->data))
#define LN_GET_NODE_PTR(node)   *((cm_list_node **) (node->data))

//ln_vm_area.access bitmasks
#define LN_ACCESS_READ    0x01
#define LN_ACCESS_WRITE   0x02
#define LN_ACCESS_EXEC    0x04
#define LN_ACCESS_SHARED  0x08


struct _ln_vm_obj;

// --- [memory area]
typedef struct {

    char * pathname;
    char * basename;

    uintptr_t start_addr;
    uintptr_t end_addr;

    cm_byte access;

    cm_list_node * obj_node_ptr;      //STORES: own vm_obj *
    cm_list_node * last_obj_node_ptr; //STORES: last encountered vm_obj *

    int id;
    bool mapped; //can be set to false with map update

} ln_vm_area;


// --- ['backing' object]
struct _ln_vm_obj {

    char pathname[PATH_MAX];
    char basename[NAME_MAX];

    uintptr_t start_addr;
    uintptr_t end_addr;

    cm_list vm_area_node_ptrs; //STORES: cm_list_node * of ln_vm_area

    int id;
    bool mapped; //can be set to false with map update
};
typedef struct _ln_vm_obj ln_vm_obj;


// --- [memory map]
typedef struct {

    //up to date entries
    cm_list vm_areas;   //STORES: ln_vm_area
    cm_list vm_objs;    //STORES: ln_vm_obj

    //unmapped entries (storage for future deallocation)
    cm_list vm_areas_unmapped; //STORES: cm_list_node * of ln_vm_area
    cm_list vm_objs_unmapped;  //STORES: cm_list_node * of ln_vm_obj

    // [internal]
    int next_id_area;
    int next_id_obj;

} ln_vm_map;


void ln_new_vm_map(ln_vm_map * vm_map);
int ln_del_vm_map(ln_vm_map * vm_map);
int ln_map_clean_unmapped(ln_vm_map * vm_map);

off_t ln_get_area_offset(const cm_list_node * area_node, const uintptr_t addr);
off_t ln_get_obj_offset(const cm_list_node * obj_node, const uintptr_t addr);
cm_list_node * ln_get_vm_area_by_addr(const ln_vm_map * vm_map, 
                                      const uintptr_t addr, const off_t * offset);

cm_list_node * ln_get_vm_obj_by_addr(const ln_vm_map * vm_map, 
                                     const uintptr_t addr, off_t * offset);
cm_list_node * ln_get_vm_obj_by_pathname(const ln_vm_map * vm_map, 
                                         const char * pathname);
cm_list_node * ln_get_vm_obj_by_basename(const ln_vm_map * vm_map, 
                                         const char * basename);
```


### STRUCTURE
**liblain** heavily relies on the linked list implementation provided by **libcmore**. Have a look at **libcmore_list**(3) to understand their interface.

A memory map represents the virtual memory area address mappings of a process. The memory map of a target is represented by a *ln_vm_map* structure. This structure consists of 2 main linked lists: *vm_areas* and *vm_objs*. The *vm_areas* list stores the virtual memory areas of a process. The *vm_objs* list stores the backing objects of a process.

The *ln_vm_area* structure represents a single virtual memory area of a process (kernel: struct *vm_area_struct*). Access permissions of an area can be checked by applying the *LN_ACCESS_READ*, *LN_ACCESS_WRITE*, *LN_ACCESS_EXEC*, and *LN_ACCESS_SHARED* bitmasks to the *access* member.

The *ln_vm_obj* structure represents a single 'backing file' of a set of virtual memory areas (kernel *vm_area_struct.vm_file).

Traversal between an area and its object can be done in O(1). Each area stores a pointer to its corresponding object list node, *obj_node_ptr*, if one is present. Each area without a corresponding object stores a pointer to the last encountered object list node instead, *last_obj_node_ptr*. Each object contains a list of pointers to its corresponding area list nodes, *vm_area_node_ptrs*.

The macros *LN_GET_NODE_AREA()*, *LN_GET_NODE_OBJ()*, and *LN_GET_NODE_PTR()* have been provided to easily fetch the data held by a linked list node.

A memory map is populated and updated by calling **ln_update_map()** on an open session. See **liblain_iface**(3).

Following an update to a map, some areas and objects may become unmapped. To prevent pointer invalidation, their list nodes will be moved to the *vm_areas_unmapped* and *vm_objs_unmapped* linked lists. The *mapped* values of their *ln_vm_area* and *ln_vm_obj* structures will be set to false, and the *next* and *prev* pointers of their nodes will be set to NULL. When ready, all unmapped nodes can be deallocated with **ln_map_clean_unmapped()**.


### FUNCTIONS
The **ln_new_vm_map()** function initialises a new map *vm_map*.

The **ln_del_vm_map()** function deallocates all contents of a map *vm_map*.

The **ln_map_clean_unmapped()** function deallocates all unmapped areas and objects of a map *vm_map*.

The **ln_get_area_offset()** function returns the offset of *addr* from the start of the area *area_node*.

The **ln_get_obj_offset()** function returns the offset of *addr* from the start of the obj *obj_node*.

The **ln_get_area_offset_bnd()** function returns the offset of *addr* from the start of the area *area_node*, or -1 if the address is not in the area.

The **ln_get_obj_offset_bnd()** function returns the offset of *addr* from the start of the obj *obj_node*, or -1 if the address is not in the area.

The **ln_get_vm_area_by_addr()** functions returns a pointer to the area node that *addr* falls into. If *offset* is not NULL, it is set to the offset of *addr* from the beginning of the area.

The **ln_get_vm_obj_by_addr()** function returns a pointer to the object node that *addr* falls into. If *offset* is not NULL, it is set to the offset of *addr* from the beginning of the object.

The **ln_get_vm_obj_by_pathname()** function returns a pointer to the first object who's path matches *pathname*.

The **ln_get_vm_obj_by_basename()** function returns a pointer to the first object who's name matches *basename*.



### RETURN VALUES
**ln_new_vm_map()**, **ln_del_vm_map()**, and **ln_map_clean_unmapped()** functions return 0 on success and -1 on error. 

**ln_get_area_offset()**, and **ln_get_obj_offset()** return an offset on success, and -1 if *addr* does not belong in the area/object.

**ln_get_vm_area_by_addr()** return a *cm_list_node \** holding a *ln_vm_area* on success, and NULL on error.

**ln_get_vm_obj_by_addr()**, **ln_get_vm_obj_by_pathname()**, and **ln_get_vm_obj_by_basename()** return a *cm_list_node \** on success, and NULL on error.

On error, *ln_errno* is set. See **liblain_error**(3).


### EXAMPLES
See *src/test/map.c* for examples.
  

### SEE ALSO
**liblain_error**(3), **liblain_iface**(3), **liblain_util**(3)
