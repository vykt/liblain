#ifndef LAINKO_H
#define LAINKO_H

#ifdef __cplusplus
extern "C" {
#endif


#include <linux/limits.h>


/*
 *  This header is shared with userspace.
 */

//lainmemu ioctl call numbers
#define LAINMEMU_IOCTL_OPEN_TGT    0
#define LAINMEMU_IOCTL_RELEASE_TGT 1
#define LAINMEMU_IOCTL_GET_MAP     2
#define LAINMEMU_IOCTL_GET_MAP_SZ  3


//templates for ioctl calls
#define LAINMEMU_TEMPLATE_OPEN_TGT    0x40080000
#define LAINMEMU_TEMPLATE_RELEASE_TGT 0x00000001
#define LAINMEMU_TEMPLATE_GET_MAP     0xc0080002
#define LAINMEMU_TEMPLATE_GET_MAP_SZ  0x40080003

//template macro
#define LAINKO_APPLY_TEMPLATE(major, lainmemu_template) (((major << 8) & 0x0000ff00) | lainmemu_template)


//vma protection - taken from linux/pgtable_types.h
typedef unsigned long lainko_pgprot_t;

#define VM_READ		 0x00000001
#define VM_WRITE	 0x00000002
#define VM_EXEC		 0x00000004
#define VM_SHARED	 0x00000008

#define VM_PROT_MASK 0x0000000F


typedef char lainko_byte;
typedef unsigned char lainko_ubyte;



//ioctl argument
struct ioctl_arg {
    lainko_byte * u_buf;
    int target_pid;
};


//map entry
struct vm_entry {

    unsigned long vm_start;
    unsigned long vm_end;
    
    unsigned long file_off;
    lainko_pgprot_t prot;
    char file_path[PATH_MAX];
};

#ifdef __cplusplus
}
#endif

#endif
