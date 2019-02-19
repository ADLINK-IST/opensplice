/*
 *                         Vortex OpenSplice
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */
#ifndef IDL_FILEMAP_H
#define IDL_FILEMAP_H

#include <c_metabase.h>
#include <c_base.h>
#include <c_iterator.h>

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
    const char *fileName,
    const char *inclName);

void
idl_fileMapAssociation(
    const idl_fileMap fileMap,
    const c_baseObject object,
    const char *fileName);

const c_char *
idl_fileMapResolve(
    const idl_fileMap fileMap,
    const c_baseObject object);

const c_char *
idl_fileMapResolveInclude(
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
