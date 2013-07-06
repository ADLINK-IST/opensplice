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
#ifndef IDL_FILEMAP_H
#define IDL_FILEMAP_H

#include <c_metabase.h>
#include <c_base.h>

C_CLASS(idl_fileMap);

idl_fileMap
idl_fileMapDefGet(void);

void
idl_fileMapDefSet(
    const idl_fileMap fileMap);

idl_fileMap
idl_fileMapNew(void);

void
idl_fileMapFree(
    const idl_fileMap fileMap);

void
idl_fileMapAdd(
    const idl_fileMap fileMap,
    const char *fileName);

void
idl_fileMapAssociation(
    const idl_fileMap fileMap,
    const c_baseObject object,
    const char *fileName);

c_char *
idl_fileMapResolve(
    const idl_fileMap fileMap,
    const c_baseObject object);

c_bool
idl_fileMapObject(
    const idl_fileMap fileMap,
    const char *fileName,
    const c_baseObject object);

c_bool idl_fileMapCheckFinalized(
    const idl_fileMap fileMap,
    const char* fileName);

c_iter
idl_fileMapGetObjects(
    const idl_fileMap fileMap,
    const char *fileName);

#endif /* IDL_FILEMAP_H */
