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
#ifndef COLL_COMPARE_H
#define COLL_COMPARE_H

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

OS_API int
stringIsLessThen(
    void *left,
    void *right);

OS_API int
pointerIsLessThen(
    void *left,
    void *right);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif /* COLL_COMPARE_H */
