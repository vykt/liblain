#ifndef DEBUG_H
#define DEBUG_H


/*
 *  Do not define internal functions as static in debug builds
 */

#ifdef DEBUG 
#define DBG_STATIC
#define DBG_INLINE
#else
#define DBG_STATIC static
#define DBG_INLINE inline
#endif


#endif
