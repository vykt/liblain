#ifndef LIBMOD_H
#define LIBMOD_H

#ifdef __cplusplus
extern "C"{
#endif

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>

#include <unistd.h>

#include <linux/limits.h>

#include <libcmore.h>


//these macros take a cm_list_node pointer
#define LN_GET_NODE_AREA(node)  ((ln_vm_area *) (node->data))
#define LN_GET_NODE_OBJ(node)   ((ln_vm_obj *) (node->data))
#define LN_GET_NODE_PTR(node)   *((cm_list_node **) (node->data))


//ln_vm_area.access bitmasks
#define LN_ACCESS_READ    0x01
#define LN_ACCESS_WRITE   0x02
#define LN_ACCESS_EXEC    0x04
#define LN_ACCESS_SHARED  0x08


/*
 *  --- [DATA TYPES] ---
 */

#define LN_IFACE_LAINKO 0
#define LN_IFACE_PROCFS 1


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


// --- [interface / session]
struct _ln_session;

typedef struct {

    int (*open)(struct _ln_session *, int);
    int (*close)(struct _ln_session *);
    int (*update_map)(struct _ln_session *, ln_vm_map *);
    int (*read)(struct _ln_session *, uintptr_t, cm_byte *, size_t);
    int (*write)(struct _ln_session *, uintptr_t, cm_byte *, size_t);

} ln_iface;


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

    long page_size;
    ln_iface iface;

}; 
typedef struct _ln_session ln_session;


/*
 *  --- [FUNCTIONS] ---
 */

// --- [utils]
//return: basename = success, NULL = fail/error
extern char * ln_pathname_to_basename(char * pathname);
//must destroy 'pid_vector' manually on success | pid = success, -1 = fail/error
extern pid_t ln_pid_by_name(const char * comm, cm_vector * pid_vector);
//return: 0 = success, -1 = fail/error
extern int ln_name_by_pid(pid_t pid, char * name_buf);
//'out' must have space for double the length of 'inp' + 1
extern void ln_bytes_to_hex(cm_byte * inp, int inp_len, char * out);


// --- [virtual interface (session)]
//all return 0 = success, -1 = fail/error
extern int ln_open(ln_session * session, int iface, int pid);
extern int ln_close(ln_session * session);
extern int ln_update_map(ln_session * session, ln_vm_map * vm_map);
extern int ln_read(ln_session * session, uintptr_t addr, 
                     cm_byte * buf, size_t buf_sz);
extern int ln_write(ln_session * session, uintptr_t addr,
                      cm_byte * buf, size_t buf_sz);

// --- [map operations]
//all return 0 = success, -1 = fail/error
extern void ln_new_vm_map(ln_vm_map * vm_map);
extern int ln_del_vm_map(ln_vm_map * vm_map);
extern int ln_map_clean_unmapped(ln_vm_map * vm_map);


// --- [map utils]
//return: offset from area/object = success, -1 = not in area/object
extern off_t ln_get_area_offset(cm_list_node * area_node, uintptr_t addr);
extern off_t ln_get_obj_offset(cm_list_node * obj_node, uintptr_t addr);
//return area node * = success, NULL = fail/error
extern cm_list_node * ln_get_vm_area_by_addr(ln_vm_map * vm_map, 
                                   uintptr_t addr, off_t * offset);
//return obj node * = success, NULL = fail/error
extern cm_list_node * ln_get_vm_obj_by_addr(ln_vm_map * vm_map, 
                                   uintptr_t addr, off_t * offset);
extern cm_list_node * ln_get_vm_obj_by_pathname(ln_vm_map * vm_map, char * pathname);
extern cm_list_node * ln_get_vm_obj_by_basename(ln_vm_map * vm_map, char * basename);


// --- [error handling]
//void return
extern void ln_perror();
extern const char * ln_strerror(int ln_errnum);


/*
 *  --- [ERROR HANDLING] ---
 */

extern _Thread_local int ln_errno;

// [error codes]

// 1XX - user errors
#define LN_ERR_PROC_MEM         2100
#define LN_ERR_PROC_MAP         2101
#define LN_ERR_SEEK_ADDR        2102

// 2XX - internal errors
#define LN_ERR_INTERNAL_INDEX   2200
#define LN_ERR_UNEXPECTED_NULL  2201
#define LN_ERR_LIBCMORE         2202
#define LN_ERR_READ_WRITE       2203
#define LN_ERR_MEMU_TARGET      2204
#define LN_ERR_MEMU_MAP_SZ      2205
#define LN_ERR_MEMU_MAP_GET     2206
#define LN_ERR_PROC_STATUS      2207
#define LN_ERR_PROC_NAV         2208

// 3XX - environmental errors
#define LN_ERR_MEM              2300
#define LN_ERR_PAGESIZE         2301
#define LN_ERR_LAINKO_MAJOR     2302
#define LN_ERR_MEMU_OPEN        2303


// [error code messages]

// 1XX - user errors
#define LN_ERR_PROC_MEM_MSG         "Can't open /proc/<pid>/mem for specified pid.\n"
#define LN_ERR_PROC_MAP_MSG         "Can't open /proc/<pid>/maps for specified pid.\n"
#define LN_ERR_SEEK_ADDR_MSG        "Could not seek to specified address.\n"

// 2XX - internal errors
#define LN_ERR_INTERNAL_INDEX_MSG   "Internal: Indexing error.\n"
#define LN_ERR_UNEXPECTED_NULL_MSG  "Internal: Unexpected NULL pointer.\n"
#define LN_ERR_LIBCMORE_MSG         "Internal: Libcmore error. See cm_perror().\n"
#define LN_ERR_READ_WRITE_MSG       "Internal: Read/write failed.\n"
#define LN_ERR_MEMU_TARGET_MSG      "Internal: Lainko target open failed.\n"
#define LN_ERR_MEMU_MAP_SZ_MSG      "Internal: Lainko map size fetch failed..\n"
#define LN_ERR_MEMU_MAP_GET_MSG     "Internal: Lainko map transfer failed.\n"
#define LN_ERR_PROC_STATUS_MSG      "Internal: Failed to use /proc/<pid>/status.\n"
#define LN_ERR_PROC_NAV_MSG         "Internal: Failed to navigate /proc directories.\n"

// 3XX - environmental errors
#define LN_ERR_MEM_MSG              "Failed to acquire the necessary memory.\n"
#define LN_ERR_PAGESIZE_MSG         "Unable to fetch pagesize through sysconf().\n"
#define LN_ERR_LAINKO_MAJOR_MSG     "Could not fetch lainko's major number.\n"
#define LN_ERR_MEMU_OPEN_MSG        "Could not open lainmemu device.\n"


#ifdef __cplusplus
}
#endif

#endif
