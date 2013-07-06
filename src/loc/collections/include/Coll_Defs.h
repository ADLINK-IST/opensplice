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
#ifndef COLL_DEFS_H
#define COLL_DEFS_H

#if defined (__cplusplus)
extern "C" {
#endif 
#include "os_if.h"

#ifdef OSPL_BUILD_LOC_COLLECTIONS
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

#define COLL_ERROR_START                (0x400u)
#define COLL_OK                         (COLL_ERROR_START)
#define COLL_ERROR_ALLOC                (COLL_ERROR_START + 1)
#define COLL_ERROR_NOT_EMPTY            (COLL_ERROR_START + 2)
#define COLL_ERROR_ALREADY_EXISTS       (COLL_ERROR_START + 3)
#define COLL_ERROR_PRECONDITION_NOT_MET (COLL_ERROR_START + 4)

#ifndef NULL
#define NULL ((void *) 0)
#endif

#undef OS_API

#if defined (__cplusplus)
}
#endif 

#endif /* COLL_DEFS_H */
