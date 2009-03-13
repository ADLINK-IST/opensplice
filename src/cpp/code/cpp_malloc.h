#ifndef _CPP_MALLOC_
#define _CPP_MALLOC_

#include "stdincs.h"
#include "os_heap.h"

#ifdef __cplusplus
extern "C"
{
#endif

   extern char *copyofstr (char *);
   extern char *copyofblk (char *, int);
   extern void err_head (void);
   extern void Check_malloc (char *);

#ifdef __cplusplus
}
#endif

#define NEW(type) ((type *) os_malloc(sizeof(type)))
#define OLD(x) os_free((char *) x)
#define check_os_malloc(ptr) Check_malloc((char *)(ptr))

#endif
