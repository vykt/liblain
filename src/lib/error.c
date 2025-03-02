//standard library
#include <stdio.h>

//local headers
#include "memcry.h"
#include "error.h"


__thread int mc_errno;


/*
 *  TODO: Find a better way to do this.
 */


void mc_perror(const char * prefix) {

    switch(mc_errno) {
        // 1XX - user errors
        case MC_ERR_PROC_MEM:
            fprintf(stderr, "%s: %s", prefix, MC_ERR_PROC_MEM_MSG);
            break;
        
        case MC_ERR_PROC_MAP:
            fprintf(stderr, "%s: %s", prefix, MC_ERR_PROC_MAP_MSG);
            break;
        
        case MC_ERR_SEEK_ADDR:
            fprintf(stderr, "%s: %s", prefix, MC_ERR_SEEK_ADDR_MSG);
            break;

        // 2XX - internal errors
        case MC_ERR_INTERNAL_INDEX:
            fprintf(stderr, "%s: %s", prefix, MC_ERR_INTERNAL_INDEX_MSG);
            break;

        case MC_ERR_AREA_IN_OBJ:
            fprintf(stderr, "%s: %s", prefix, MC_ERR_AREA_IN_OBJ_MSG);
            break;
        
        case MC_ERR_UNEXPECTED_NULL:
            fprintf(stderr, "%s: %s", prefix, MC_ERR_UNEXPECTED_NULL_MSG);
            break;
        
        case MC_ERR_CMORE:
            fprintf(stderr, "%s: %s", prefix, MC_ERR_CMORE_MSG);
            break;
        
        case MC_ERR_READ_WRITE:
            fprintf(stderr, "%s: %s", prefix, MC_ERR_READ_WRITE_MSG);
            break;
        
        case MC_ERR_MEMU_TARGET:
            fprintf(stderr, "%s: %s", prefix, MC_ERR_MEMU_TARGET_MSG);
            break;
        
        case MC_ERR_MEMU_MAP_SZ:
            fprintf(stderr, "%s: %s", prefix, MC_ERR_MEMU_MAP_SZ_MSG);
            break;
        
        case MC_ERR_MEMU_MAP_GET:
            fprintf(stderr, "%s: %s", prefix, MC_ERR_MEMU_MAP_GET_MSG);
            break;
        
        case MC_ERR_PROC_STATUS:
            fprintf(stderr, "%s: %s", prefix, MC_ERR_PROC_STATUS_MSG);
            break;
        
        case MC_ERR_PROC_NAV:
            fprintf(stderr, "%s: %s", prefix, MC_ERR_PROC_NAV_MSG);
            break;

        // 3XX - environmental errors
        case MC_ERR_MEM:
            fprintf(stderr, "%s: %s", prefix, MC_ERR_MEM_MSG);
            break;
        
        case MC_ERR_PAGESIZE:
            fprintf(stderr, "%s: %s", prefix, MC_ERR_PAGESIZE_MSG);
            break;
        
        case MC_ERR_KRNCRY_MAJOR:
            fprintf(stderr, "%s: %s", prefix, MC_ERR_KRNCRY_MAJOR_MSG);
            break;
        
        case MC_ERR_MEMU_OPEN:
            fprintf(stderr, "%s: %s", prefix, MC_ERR_MEMU_OPEN_MSG);
            break;

        default:
            fprintf(stderr, "Undefined error code.\n");
            break;
    }

    return;
}


const char * mc_strerror(const int mc_errnum) {

    switch (mc_errnum) {
        // 1XX - user errors
        case MC_ERR_PROC_MEM:
            return MC_ERR_PROC_MEM_MSG;
        
        case MC_ERR_PROC_MAP:
            return MC_ERR_PROC_MAP_MSG;
        
        case MC_ERR_SEEK_ADDR:
            return MC_ERR_SEEK_ADDR_MSG;

        // 2xx - internal errors
        case MC_ERR_INTERNAL_INDEX:
            return MC_ERR_INTERNAL_INDEX_MSG;

        case MC_ERR_AREA_IN_OBJ:
            return MC_ERR_AREA_IN_OBJ_MSG;
        
        case MC_ERR_UNEXPECTED_NULL:
            return MC_ERR_UNEXPECTED_NULL_MSG;
        
        case MC_ERR_CMORE:
            return MC_ERR_CMORE_MSG;
        
        case MC_ERR_READ_WRITE:
            return MC_ERR_READ_WRITE_MSG;
        
        case MC_ERR_MEMU_TARGET:
            return MC_ERR_MEMU_TARGET_MSG;
        
        case MC_ERR_MEMU_MAP_SZ:
            return MC_ERR_MEMU_MAP_SZ_MSG;
        
        case MC_ERR_MEMU_MAP_GET:
            return MC_ERR_MEMU_MAP_GET_MSG;
        
        case MC_ERR_PROC_STATUS:
            return MC_ERR_PROC_STATUS_MSG;
        
        case MC_ERR_PROC_NAV:
            return MC_ERR_PROC_NAV_MSG;

        // 3XX - environmental errors
        case MC_ERR_MEM:
            return MC_ERR_MEM_MSG;
        
        case MC_ERR_PAGESIZE:
            return MC_ERR_PAGESIZE_MSG;
        
        case MC_ERR_KRNCRY_MAJOR:
            return MC_ERR_KRNCRY_MAJOR_MSG;
        
        case MC_ERR_MEMU_OPEN:
            return MC_ERR_MEMU_OPEN_MSG;

        default:
            return "Undefined error code.\n";
    }
}
