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
#ifndef GAPI_MAP_H
#define GAPI_MAP_H

#include "gapi_common.h"
#include "os_if.h"

#if defined (__cplusplus)
extern "C" {
#endif

#ifdef OSPL_BUILD_CORE
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

#define gapi_map(o)		((gapi_map)(o))
C_CLASS(gapi_map);

#define gapi_mapIter(o)		((gapi_mapIter)(o))
C_CLASS(gapi_mapIter);

OS_API gapi_map
gapi_mapNew (
    gapi_equality (*compare)(),
    gapi_boolean free_key,
    gapi_boolean free_object);

OS_API void
gapi_mapFree (
    gapi_map map);

OS_API gapi_returnCode_t
gapi_mapAdd (
    gapi_map map,
    gapi_object key,
    gapi_object object);

OS_API void
gapi_mapRemove (
    gapi_map map,
    gapi_object key);

OS_API c_long
gapi_mapLength(
    gapi_map map);

OS_API gapi_boolean
gapi_mapIsEmpty (
    gapi_map map);

OS_API gapi_mapIter
gapi_mapFind (
    gapi_map map,
    const gapi_object key);

OS_API gapi_mapIter
gapi_mapFirst (
    gapi_map map);

OS_API gapi_mapIter
gapi_mapLast (
    gapi_map map);

OS_API void
gapi_mapIterFree (
    gapi_mapIter mapIter);

OS_API gapi_long
gapi_mapIterNext (
    gapi_mapIter mapIter);

OS_API gapi_long
gapi_mapIterPrev (
    gapi_mapIter mapIter);
OS_API 
gapi_long
gapi_mapIterRemove (
    gapi_mapIter mapIter);

OS_API gapi_object
gapi_mapIterKey (
    gapi_mapIter mapIter);

OS_API gapi_object
gapi_mapIterObject (
    gapi_mapIter mapIter);

OS_API gapi_long
gapi_mapIterSize (
    gapi_mapIter mapIter);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif
