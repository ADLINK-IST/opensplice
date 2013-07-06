/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2013 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE 
 *
 *   for full copyright notice and license terms. 
 *
 */
#ifndef _CPP_MALLOC_
#define _CPP_MALLOC_

#include "stdincs.h"
#include "os_heap.h"

#ifdef __cplusplus
extern "C"
{
#endif

   extern char *copyofstr (const char *);
   extern char *copyofblk (const char *, int);
   extern void err_head (void);
   extern void Check_malloc (const char *);

#ifdef __cplusplus
}
#endif

#define NEW(type) ((type *) os_malloc(sizeof(type)))
#define OLD(x) os_free((char *) x)
#define check_os_malloc(ptr) Check_malloc((const char *)(ptr))

#endif
