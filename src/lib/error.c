#include <stdio.h>

#include "liblain.h"
#include "error.h"


__thread int ln_errno;


void ln_perror() {

    switch(ln_errno) {

        // 1XX - user errors
        case LN_ERR_PROC_MEM:
            fprintf(stderr, LN_ERR_PROC_MEM_MSG);
            break;
        
        case LN_ERR_PROC_MAP:
            fprintf(stderr, LN_ERR_PROC_MAP_MSG);
            break;
        
        case LN_ERR_SEEK_ADDR:
            fprintf(stderr, LN_ERR_SEEK_ADDR_MSG);
            break;

        // 2XX - internal errors
        case LN_ERR_INTERNAL_INDEX:
            fprintf(stderr, LN_ERR_INTERNAL_INDEX_MSG);
            break;
        
        case LN_ERR_UNEXPECTED_NULL:
            fprintf(stderr, LN_ERR_UNEXPECTED_NULL_MSG);
            break;
        
        case LN_ERR_LIBCMORE:
            fprintf(stderr, LN_ERR_LIBCMORE_MSG);
            break;
        
        case LN_ERR_READ_WRITE:
            fprintf(stderr, LN_ERR_READ_WRITE_MSG);
            break;
        
        case LN_ERR_MEMU_TARGET:
            fprintf(stderr, LN_ERR_MEMU_TARGET_MSG);
            break;
        
        case LN_ERR_MEMU_MAP_SZ:
            fprintf(stderr, LN_ERR_MEMU_MAP_SZ_MSG);
            break;
        
        case LN_ERR_MEMU_MAP_GET:
            fprintf(stderr, LN_ERR_MEMU_MAP_GET_MSG);
            break;
        
        case LN_ERR_PROC_STATUS:
            fprintf(stderr, LN_ERR_PROC_STATUS_MSG);
            break;
        
        case LN_ERR_PROC_NAV:
            fprintf(stderr, LN_ERR_PROC_NAV_MSG);
            break;


        // 3XX - environmental errors
        case LN_ERR_MEM:
            fprintf(stderr, LN_ERR_MEM_MSG);
            break;
        
        case LN_ERR_PAGESIZE:
            fprintf(stderr, LN_ERR_PAGESIZE_MSG);
            break;
        
        case LN_ERR_LAINKO_MAJOR:
            fprintf(stderr, LN_ERR_LAINKO_MAJOR_MSG);
            break;
        
        case LN_ERR_MEMU_OPEN:
            fprintf(stderr, LN_ERR_MEMU_OPEN_MSG);
            break;

        default:
            fprintf(stderr, "Undefined error code.\n");
            break;
    }

    return;
}


const char * ln_strerror(const int ln_errnum) {

    switch (ln_errnum) {

        // 1XX - user errors
        case LN_ERR_PROC_MEM:
            return LN_ERR_PROC_MEM_MSG;
        
        case LN_ERR_PROC_MAP:
            return LN_ERR_PROC_MAP_MSG;
        
        case LN_ERR_SEEK_ADDR:
            return LN_ERR_SEEK_ADDR_MSG;

        // 2xx - internal errors
        case LN_ERR_INTERNAL_INDEX:
            return LN_ERR_INTERNAL_INDEX_MSG;
        
        case LN_ERR_UNEXPECTED_NULL:
            return LN_ERR_UNEXPECTED_NULL_MSG;
        
        case LN_ERR_LIBCMORE:
            return LN_ERR_LIBCMORE_MSG;
        
        case LN_ERR_READ_WRITE:
            return LN_ERR_READ_WRITE_MSG;
        
        case LN_ERR_MEMU_TARGET:
            return LN_ERR_MEMU_TARGET_MSG;
        
        case LN_ERR_MEMU_MAP_SZ:
            return LN_ERR_MEMU_MAP_SZ_MSG;
        
        case LN_ERR_MEMU_MAP_GET:
            return LN_ERR_MEMU_MAP_GET_MSG;
        
        case LN_ERR_PROC_STATUS:
            return LN_ERR_PROC_STATUS_MSG;
        
        case LN_ERR_PROC_NAV:
            return LN_ERR_PROC_NAV_MSG;

        // 3XX - environmental errors
        case LN_ERR_MEM:
            return LN_ERR_MEM_MSG;
        
        case LN_ERR_PAGESIZE:
            return LN_ERR_PAGESIZE_MSG;
        
        case LN_ERR_LAINKO_MAJOR:
            return LN_ERR_LAINKO_MAJOR_MSG;
        
        case LN_ERR_MEMU_OPEN:
            return LN_ERR_MEMU_OPEN_MSG;

        default:
            return "Undefined error code.\n";
    }
}
