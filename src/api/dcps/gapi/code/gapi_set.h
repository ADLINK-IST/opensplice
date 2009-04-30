/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2009 PrismTech 
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE 
 *
 *   for full copyright notice and license terms. 
 *
 */
#ifndef GAPI_SET_H
#define GAPI_SET_H

#include "gapi_common.h"
#include "os_if.h"

#if defined (__cplusplus)
extern "C" {
#endif

#ifdef OSPL_BUILD_DCPSGAPI
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

#define gapi_set(o)		((gapi_set)(o))
C_CLASS(gapi_set);

#define gapi_setIter(o)		((gapi_setIter)(o))
C_CLASS(gapi_setIter);

OS_API gapi_set
gapi_setNew (
    gapi_equality (*compare)());

OS_API void
gapi_setFree (
    gapi_set set);

OS_API gapi_returnCode_t
gapi_setAdd (
    gapi_set set,
    gapi_object object);

OS_API void
gapi_setRemove (
    gapi_set set,
    gapi_object object);

OS_API gapi_boolean
gapi_setIsEmpty (
    gapi_set set);

OS_API gapi_setIter
gapi_setFind (
    gapi_set set,
    gapi_object object);

OS_API gapi_setIter
gapi_setFirst (
    gapi_set set);

OS_API gapi_setIter
gapi_setLast (
    gapi_set set);

OS_API gapi_setIter
gapi_setIterNew (
    void);

OS_API void
gapi_setIterFree (
    gapi_setIter setIter);

OS_API gapi_long
gapi_setIterNext (
    gapi_setIter setIter);

OS_API gapi_long
gapi_setIterPrev (
    gapi_setIter setIter);

OS_API gapi_long
gapi_setIterRemove (
    gapi_setIter setIter);

OS_API gapi_object
gapi_setIterObject (
    gapi_setIter setIter);

OS_API gapi_long
gapi_setIterSize (
    gapi_setIter setIter);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif
