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
#include "os_heap.h"
#include "os_stdlib.h"
#include "c_typebase.h"
#include "c_iterator.h"

#include "idl_fileMap.h"

/* defFileMap stores the default file map */
static idl_fileMap defFileMap = NULL;

C_CLASS(idl_file);

/* "idl_fileMap" registers all files that are processed.
*/
C_STRUCT(idl_fileMap) {
    c_iter files;
};

/* "idl_file" registers all meta objects related to a file,
   "fileName" specifies the file name,
   "contains" stores all meta data object references.
*/
C_STRUCT(idl_file) {
    c_char *fileName;
    c_iter contains;
};

C_CLASS(idl_map);

/* "idl_map" contains a filename meta object association
*/
C_STRUCT(idl_map) {
    c_char *fileName;
    c_baseObject object;
};

/* Get the default file map */
idl_fileMap
idl_fileMapDefGet(void)
{
    return defFileMap;
}

/* Set the default file map */
void
idl_fileMapDefSet(
    const idl_fileMap fileMap)
{
    defFileMap = fileMap;
}

/* Create a new file, identified by fileName */
static idl_file
idl_fileNew(
    const char *fileName)
{
    /* QAC EXPECT 5007; will not use wrapper */
    idl_file file = os_malloc ((size_t)C_SIZEOF(idl_file));

    file->fileName = os_strdup(fileName);
    file->contains = c_iterNew(0);

    return file;
}

/* QAC EXPECT 5007; suppress QACtools error */
/* Free the specified file object without releasing all contained elements */
static void
idl_fileFree(
    /* QAC EXPECT 3673; No solution to the message here, but no problem either */
    const idl_file file)
{
    void *listElement;

    listElement = c_iterTakeFirst(file->contains);
    while (listElement != NULL) {
        listElement = c_iterTakeFirst(file->contains);
    }
    /* QAC EXPECT 5007; will not use wrapper */
    os_free(file->fileName);
}

/* Check if the specified object list element is equal to the meta object specified
   by object
*/
static c_equality
idl_objectCompare(
    /* QAC EXPECT 3673; No solution to the message here, but no problem either */
    void* _listObject,
    /* QAC EXPECT 3673; No solution to the message here, but no problem either */
    void* _object)
{
    c_baseObject listObject;
    c_baseObject object;
    c_equality result = C_NE;

    listObject = _listObject;
    object = _object;

    if (listObject == object) {
        result = C_EQ;
    }

    return result;
}

/* Check if the specified object list element is equal to the meta object specified
   by the map
*/
static c_equality
idl_objectMap(
    /* QAC EXPECT 3673; No solution to the message here, but no problem either */
    void* _listObject,
    /* QAC EXPECT 3673; No solution to the message here, but no problem either */
    void* _map)
{
    c_baseObject listObject;
    idl_map map;
    c_equality result = C_NE;

    listObject = _listObject;
    map = _map;

    if (listObject == map->object) {
        result = C_EQ;
    }

    return result;
}

/* Add the specified meta object to the specified file */
static void
idl_fileObjectAdd(
    /* QAC EXPECT 3673; No solution to the message here, but no problem either */
    const idl_file file,
    const c_baseObject object)
{
    c_baseObject o;

    /* find the meta object in the specified file */
    o = c_iterResolve(file->contains, idl_objectCompare, object);
    if (o == NULL) {
        /* if not found, add it to the file */
        c_iterInsert (file->contains, object);
    }
}

/* Create a new file map */
idl_fileMap
idl_fileMapNew(void)
{
    /* QAC EXPECT 5007; will not use wrapper */
    idl_fileMap fileMap = os_malloc((size_t)C_SIZEOF(idl_fileMap));

    fileMap->files = c_iterNew(0);
    return fileMap;
}

/* QAC EXPECT 5007; suppress QACtools error */
/* Free a file map releasing all files associated with it */
void
idl_fileMapFree(
    const idl_fileMap fileMap)
{
    idl_file file;

    file = c_iterTakeFirst(fileMap->files);
    while (file) {
        idl_fileFree (file);
        file = c_iterTakeFirst(fileMap->files);
    }
    /* QAC EXPECT 5007; will not use wrapper */
    os_free (fileMap);
}

/* Check if the provided filename is associated with the provided file */
static c_equality
idl_fileCompare(
    /* QAC EXPECT 3673; No solution to the message here, but no problem either */
    void* _file,
    void* _fileName)
{
    idl_file file;
    c_char *fileName;
    c_equality result = C_NE;

    file = _file;
    fileName = _fileName;

    /* QAC EXPECT 5007, 3416; will not use wrapper, No side effect here */
    if (strcmp(file->fileName, fileName) == 0) {
        result = C_EQ;
    }

    return result;
}

/* Add a file to the file map */
void
idl_fileMapAdd(
    /* QAC EXPECT 3673; No solution to the message here, but no problem either */
    const idl_fileMap fileMap,
    const char *fileName)
{
    idl_file file;

    /* Find the file in the file map based on the file name */
    file = c_iterResolve(fileMap->files, idl_fileCompare, (c_iterResolveCompareArg)fileName);
    if (file == NULL) {
        /* If not found, then add it now */
        file = idl_fileNew(fileName);
        c_iterAppend(fileMap->files, file);
    }
}

/* Add an meta object association to a file for a specified file map */
void
idl_fileMapAssociation(
    /* QAC EXPECT 3673; No solution to the message here, but no problem either */
    const idl_fileMap fileMap,
    const c_baseObject object,
    const char *fileName)
{
    idl_file file;

    /* Find the file based upon the spcified filename */
    file = c_iterResolve(fileMap->files, idl_fileCompare, (c_iterResolveCompareArg)fileName);
    /* QAC EXPECT 3416; No side effect here */
    assert(file);
    /* Add the object association to the file */
    idl_fileObjectAdd(file, object);
}

/* idl_searchObject will search in the specified "file"
   association for the object specified in map->object.
   If the object is found, it is returned and the associated
   filename is stored in map->fileName.
*/
static void
idl_searchObject(
    /* QAC EXPECT 3673; No solution to the message here, but no problem either */
    const idl_file file,
    const idl_map map)
{
    c_baseObject o;

    /* Search for the object in the specified "file" */
    o = c_iterResolve(file->contains, idl_objectMap, map);
    if (o != NULL) {
        /* Store the associated filename */
        map->fileName = file->fileName;
    }
}

/* Resolve the file which is related to the specified meta object in the specified file map */
c_char *
idl_fileMapResolve(
    /* QAC EXPECT 3673; No solution to the message here, but no problem either */
    const idl_fileMap fileMap,
    const c_baseObject object)
{
    /* QAC EXPECT 5007; will not use wrapper */
    idl_map map = os_malloc((size_t)C_SIZEOF(idl_map));
    c_char *fileName = NULL;

    if (fileMap != NULL) {
	/* map will contain the meta object reference before the c_iterWalk */
        map->object = object;
        map->fileName = NULL;
        c_iterWalk(fileMap->files, (c_iterWalkAction)idl_searchObject, map);
	/* map will contain the meta object reference and fileName reference
	   after the c_iterWalk.
	*/
        fileName = map->fileName;
        if (fileName == NULL) {
            /* file name is undefined for implicit defined types */
            fileName = os_strdup("");
        }
        /* QAC EXPECT 5007; will not use wrapper */
        os_free(map);
    }
    return fileName;
}

/* Check if the specified meta object is specified with associated with
   the specified file in the specified file map
*/
c_bool
idl_fileMapObject(
    /* QAC EXPECT 3673; No solution to the message here, but no problem either */
    const idl_fileMap fileMap,
    const char *fileName,
    const c_baseObject object)
{
    idl_file file;
    c_baseObject o;
    c_bool result = FALSE;

    if (fileMap != NULL) {
	/* find the file in the file map */
        file = c_iterResolve(fileMap->files, idl_fileCompare, (c_iterResolveCompareArg)fileName);
        if (file) {
	    /* find the meta object in the file association */
            o = c_iterResolve (file->contains, idl_objectCompare, object);
            if (o) {
              /* if the object is found, the association is true */
                result = TRUE;
            }
        }
    }
    return result;
}

/* Add object to source */
void idl_fileMapFillList(
    c_baseObject object,
    c_iter objects)
{
    switch(object->kind) {
    case M_LITERAL:
    case M_EXPRESSION:
    case M_EXCEPTION:
    case M_PARAMETER:
        break;
    default:
        c_iterInsert(objects, object);
        break;
    }
}

/* Create source */
c_iter
idl_fileMapGetObjects(
    const idl_fileMap fileMap,
    const char *fileName)
{
    idl_file file;
    c_iter result;

    result = 0;

    if(fileMap != NULL) {
        /* Lookup file */
        file = c_iterResolve(fileMap->files, idl_fileCompare, (c_iterResolveCompareArg)fileName);
        if (file) {
            result = c_iterNew(0);

            /* Walk metaobjects in filemap, add to source. */
            if(file->contains) {
                c_iterWalk(file->contains, (c_iterWalkAction)idl_fileMapFillList, result);
            }
        }
    }

    return result;
}

static void
idl_checkFinalized(
    c_baseObject o,
    int* unfinalCount) {
    char* name;

    switch(o->kind) {
    case M_STRUCTURE:
    case M_UNION:
        if(!c_isFinal(c_metaObject(o))) {
            name = c_metaScopedName(c_metaObject(o));
            printf("missing implementation for struct\\union %s.\n", name);
            (*unfinalCount)++;
            free(name);
        }
        break;
    default:
        break;
    }
}

/* */
c_bool idl_fileMapCheckFinalized(
    const idl_fileMap fileMap,
    const char* fileName)
{
    idl_file file;
    int count;

    count = 0;

    if(fileMap != NULL) {
        file = c_iterResolve(fileMap->files, (c_iterResolveCompare)idl_fileCompare, (c_iterResolveCompareArg)fileName);
        if(file) {

            c_iterWalk(file->contains, (c_iterWalkAction)idl_checkFinalized, &count);
        }
    }
    return !count;
}




