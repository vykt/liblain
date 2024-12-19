#ifndef KRNCRY_H
#define KRNCRY_H

#ifdef __cplusplus
extern "C" {
#endif

//system headers
#include <linux/limits.h>



/*
 *  This header comes from the krncry project.
 */

//krncry ioctl call numbers
#define KRNCRY_IOCTL_OPEN_TGT    0
#define KRNCRY_IOCTL_RELEASE_TGT 1
#define KRNCRY_IOCTL_GET_MAP     2
#define KRNCRY_IOCTL_GET_MAP_SZ  3

//templates for ioctl calls
#define KRNCRY_TEMPLATE_OPEN_TGT    0x40080000
#define KRNCRY_TEMPLATE_RELEASE_TGT 0x00000001
#define KRNCRY_TEMPLATE_GET_MAP     0xc0080002
#define KRNCRY_TEMPLATE_GET_MAP_SZ  0x40080003

//template macro
#define KRNCRY_APPLY_TEMPLATE(major, krncry_template) (((major << 8) & 0x0000ff00) | krncry_template)


//vma protection - taken from linux/pgtable_types.h
typedef unsigned long krncry_pgprot_t;

//permission bitmask
#define VM_PROT_MASK 0x0000000F

//specific permission bitmasks
#define VM_READ		 0x00000001
#define VM_WRITE	 0x00000002
#define VM_EXEC		 0x00000004
#define VM_SHARED	 0x00000008



/*
 *  --- [DATA TYPES] ---
 */

// [byte]
typedef unsigned char kc_byte;



// [ioctl argument]
struct ioctl_arg {
    kc_byte * u_buf;
    int target_pid;
};


// [map entry]
struct vm_entry {

    unsigned long vm_start;
    unsigned long vm_end;
    
    unsigned long file_off;
    krncry_pgprot_t prot;
    char file_path[PATH_MAX];
};


#ifdef __cplusplus
}
#endif

#endif
