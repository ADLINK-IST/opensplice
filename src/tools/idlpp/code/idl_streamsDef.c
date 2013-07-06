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

#include "idl_streamsDef.h"

/* Static function declarations */
static idl_streamsMap idl_streamsMapNew(const c_metaObject scope, const char *typeName);
static void idl_streamsMapFree(const idl_streamsMap streamsMap);

/* Contains the list of streams definitions */
static idl_streamsDef idl_streamsDefinitions = NULL;

/* Create a new streams definition list */
idl_streamsDef
idl_streamsDefNew(
    void)
{
    idl_streamsDef streamsDef = os_malloc(C_SIZEOF(idl_streamsDef));

    streamsDef->streamsList = c_iterNew(0);
    return streamsDef;
}

/* Set the default streams definition list */
void
idl_streamsDefDefSet(
    idl_streamsDef streamsDef)
{
    idl_streamsDefinitions = streamsDef;
}

/* Get the default streams definition list, may be null */
idl_streamsDef
idl_streamsDefDefGet(
    void)
{
    return idl_streamsDefinitions;
}

/* Add a streams definition to the specified streams definition list */
void
idl_streamsDefAdd (
    idl_streamsDef streamsDef,
    c_metaObject scope,
    const char *typeName)
{
    if(streamsDef)
    {
        c_iterInsert (streamsDef->streamsList, idl_streamsMapNew(scope, typeName));
    }
}

/* Create a new streams map specified by scope and type name */
static idl_streamsMap
idl_streamsMapNew(
    const c_metaObject scope,
    const char *typeName)
{
    idl_streamsMap streamsMap = os_malloc(C_SIZEOF(idl_streamsMap));

    streamsMap->typeName = os_strdup(typeName);
    streamsMap->scope = scope;

    return streamsMap;
}

/* Free a streams definition list, freeing all streams elements */
void
idl_streamsDefFree (
    idl_streamsDef streamsDef)
{
    idl_streamsMap streamsMap;

    while ((streamsMap = c_iterTakeFirst(streamsDef->streamsList))) {
        idl_streamsMapFree(streamsMap);
    }
    os_free(streamsDef);
}


/* Free a streams map, without freeing the scope */
static void
idl_streamsMapFree(
    const idl_streamsMap streamsMap)
{
    os_free(streamsMap->typeName);
    os_free(streamsMap);
}


/* Check if */
c_bool
idl_streamsResolve(
    idl_streamsDef streamsDef,
    idl_scope scope,
    const char *typeName)
{
    c_long streamsIndex, streamsCount;
    c_long scopeIndex;
    c_bool result;
    idl_streamsMap streamsMap;
    c_metaObject typeScope;

    result = FALSE;
    /* Check all streams definition elements */
    streamsCount = c_iterLength(streamsDef->streamsList);
    for (streamsIndex = 0; (streamsIndex < streamsCount) && (result == FALSE); streamsIndex++) {
        streamsMap = c_iterObject(streamsDef->streamsList, streamsIndex);
        /* Check if the typename matches */
        if (strcmp(typeName, streamsMap->typeName) == 0) {
            if ((idl_scopeStackSize(scope) == 0) && (streamsMap->scope->definedIn == NULL)) {
                /* It is in the global scope */
                result = TRUE;
            } else {
                /* Check other (module) scopes */
                scopeIndex = idl_scopeStackSize(scope) - 1;
                typeScope = streamsMap->scope;
                while (scopeIndex >= 0) {
                    /* Check if scope is a module and the names match */
                    if ((idl_scopeElementType(idl_scopeIndexed(scope, scopeIndex)) == idl_tModule) &&
                        (strcmp(typeScope->name, idl_scopeElementName(idl_scopeIndexed(scope, scopeIndex))) == 0)) {
                        if (typeScope) {
                            typeScope = typeScope->definedIn;
                        }
                        scopeIndex--;
                        /* Top-level scope reached */
                        if (scopeIndex == -1) {
                            /* Check if scope matches */
                            if (typeScope == NULL || typeScope->name == NULL) {
                                result = TRUE;
                            }
                        }
                    } else {
                        break;
                    }
                }
            }
        }
    }
    return result;
}
