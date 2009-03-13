#ifndef IDL_FILEMAP_H
#define IDL_FILEMAP_H

#include <c_metabase.h>

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

#endif /* IDL_FILEMAP_H */
