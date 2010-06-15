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
#include "c_stringSupport.h"

#include "idl_databaseValidation.h"
#include "idl_keyDef.h"
#include "idl_catsDef.h"
#include "os.h"


/*************************************************************************************************
 * Local Defines
 *************************************************************************************************/

#define IDL_MAX_ERRORSIZE 512

/*************************************************************************************************
 * Local Type Definitions
 *************************************************************************************************/

typedef enum {
    idl_UndeclaredIdentifier,
    idl_IllegalKeyFields,
    idl_NoCorrespondingCats
} idl_errorIndex;


/*************************************************************************************************
 * Local Read Only Data
 *************************************************************************************************/

static char *errorText [] = {
    "Undeclared referenced identifier %s.",
    "Key fields are only supported for struct type.",
    "No corresponding '#pragma cats' for '#pragma keylist %s %s' which is a char array."
};


/*************************************************************************************************
 * Local Functions and Prototypes
 *************************************************************************************************/

static void
idl_printError(
    const char* filename,
    char *text)
{
    printf("*** DDS error in file %s: %s\n", filename, text);
}


/*
 * return the real type (typedef are resoved) for a key (from a keylist).
 * scope should be the data type.
 */
static c_type
idl_getTypeOfKey(
    c_type scope,
    os_char* keyName)
{
    c_iter fields = NULL;
    c_specifier sp;
    char *fieldName;

    /* Key field name consists of [<field>.]*<field> */
    fields = c_splitString(keyName, ".");
    fieldName = c_iterTakeFirst(fields);
    /* find specificer (sp) corresponding to keylist field name */
    while (fieldName != NULL) {
        if (scope) {
            sp = c_specifier(c_metaFindByName(c_metaObject(scope), fieldName, CQ_FIXEDSCOPE | CQ_MEMBER | CQ_CASEINSENSITIVE));
            assert(sp && (strcmp(sp->name, fieldName) == 0));
            scope = sp->type;
        }
        fieldName = c_iterTakeFirst(fields);
    }

    /* now scope is type of key. But it can be typedef. Determine the actual type. */
    return c_typeActualType(scope);
}

/*
 *
 */
static c_bool
idl_isCatsDefFor(
    c_metaObject scope,
    c_char *typeName,
    c_char *key)
{
    idl_catsDef catsDef = idl_catsDefDefGet();
    idl_catsMap catsMap;
    c_long catsMapIdx;
    c_iter catsList;
    os_uint32 catsListSize;
    os_uint32 catsIdx;

    if (catsDef != NULL) {
        /* check all cats definition list elements */
        for (catsMapIdx = 0; catsMapIdx < c_iterLength(catsDef->catsList); catsMapIdx++) {
            catsMap = c_iterObject(catsDef->catsList, catsMapIdx);
            if (c_metaCompare(scope, catsMap->scope) == E_EQUAL &&
                strcmp(typeName, catsMap->typeName) == 0)
            {
                /* for each cats in catsList, check if it's equal to key */
                catsList = c_splitString(catsMap->catsList, ",");
                catsListSize = c_iterLength(catsList);
                for(catsIdx = 0; catsIdx < catsListSize; catsIdx++)
                {
                    if (strcmp(c_iterTakeFirst(catsList), key) == 0) {
                        return OS_TRUE;
                    }
                }
            }
        }
    }

    return OS_FALSE;
}



/*
 * Check if each usage of a char array as a key has a corresponding
 * "#pragma cats" declaration.
 */
static c_bool
idl_checkCatsUsage(
    c_base base,
    const char* filename)
{
    char errorBuffer [IDL_MAX_ERRORSIZE];
    idl_keyDef keyDef = idl_keyDefDefGet();
    c_long keyMapIdx;
    idl_keyMap keyMap;
    c_type type;
    c_structure structure;
    c_iter keysList;
    os_uint32 keysListSize;
    os_uint32 keyIdx;
    c_char* keyName;
    os_uint32 i;
    c_iter keyNameList;
    os_uint32 keyNameListSize;
    c_structure tmpStructure;
    c_specifier sp;
    c_type subType;
    c_string typeName;
    c_type spType;

    if (keyDef != NULL) {
        /* check all key definition list elements */
        for (keyMapIdx = 0; keyMapIdx < c_iterLength(keyDef->keyList); keyMapIdx++) {
            keyMap = c_iterObject(keyDef->keyList, keyMapIdx);
            /* if a keylist is defined for the type */
            if (keyMap->keyList && strlen(keyMap->keyList) > 0) {
                /* find meteobject for the type */
                type = c_type(c_metaResolveType(keyMap->scope, keyMap->typeName));
                if (!type) {
                    snprintf(errorBuffer, IDL_MAX_ERRORSIZE-1, errorText[idl_UndeclaredIdentifier], keyMap->typeName);
                    idl_printError(filename, errorBuffer);
                    return OS_FALSE;
                }
                /* type can be a typedef. Determine the actual type. */
                type = c_typeActualType(type);

                /* type should be a structure */
                if (c_baseObject(type)->kind != M_STRUCTURE) {
                    snprintf(errorBuffer, IDL_MAX_ERRORSIZE-1, errorText[idl_IllegalKeyFields]);
                    idl_printError(filename, errorBuffer);
                    return OS_FALSE;
                }
                structure = c_structure(type);

                /* for each key in keyList, check if type is a char array */
                keysList = c_splitString(keyMap->keyList, ",");
                keysListSize = c_iterLength(keysList);
                for(keyIdx = 0; keyIdx < keysListSize; keyIdx++)
                {
                    keyName = c_iterTakeFirst(keysList);
                    /* We might be dealing with a field of a field definition in
                     * the keylist, so let's split this up
                     */
                    keyNameList = c_splitString(keyName, ".");
                    keyNameListSize = c_iterLength(keyNameList);
                    tmpStructure = structure;
                    for(i = 0; i < keyNameListSize; i++)
                    {

                        keyName = c_iterTakeFirst(keyNameList);
                        /* Now get the actual member defined by the name */
                        sp = c_specifier(c_metaFindByName(
                            c_metaObject(tmpStructure),
                            keyName,
                            CQ_FIXEDSCOPE | CQ_MEMBER | CQ_CASEINSENSITIVE));
                        if(sp)
                        {
                            spType = c_typeActualType(sp->type);
                            /* If the member is a structure, we need to
                             * recurse deeper.
                             */
                            if(c_baseObject(spType)->kind == M_STRUCTURE)
                            {
                                tmpStructure = c_structure(spType);
                            }
                            /* If the member is a collection then we need to
                             * ensure it is not a character array, but if it
                             * is we need to ensure a corresponding CATS pragma
                             * can be located
                             */
                            else if(c_baseObject(spType)->kind == M_COLLECTION && c_collectionType(spType)->kind == C_ARRAY)
                            {
                                subType = c_typeActualType(c_collectionType(spType)->subType);
                                if(c_baseObject(subType)->kind == M_PRIMITIVE &&
                                   c_primitive(subType)->kind == P_CHAR)
                                {

                                    typeName = c_metaName(c_metaObject(tmpStructure));
                                    /* check if there is corresponding catsDef */
                                    if (!idl_isCatsDefFor(c_metaObject(tmpStructure)->definedIn,
                                                          typeName,
                                                          keyName))
                                    {
                                        snprintf(
                                            errorBuffer,
                                            IDL_MAX_ERRORSIZE-1,
                                            errorText[idl_NoCorrespondingCats],
                                            c_metaObject(structure)->name,
                                            keyName);
                                        idl_printError(filename, errorBuffer);
                                        return OS_FALSE;
                                    }
                                    c_free(typeName);
                                }

                            }
                        }
                    }
                }
            }
        }
    }

    return OS_TRUE;
}


/*************************************************************************************************
 * Global Functions
 *************************************************************************************************/

/*
 * Call all validation checks
 */
c_bool
idl_validateDatabase (
    c_base base,
    const char* filename)
{
    return idl_checkCatsUsage(base, filename);
}






