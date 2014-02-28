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
#include "c_stringSupport.h"
#include "c_misc.h"
#include "c_base.h"
#include "c_metabase.h"
#include "c_collection.h"

#include "idl_scope.h"
#include "idl_stacDef.h"

C_CLASS(idl_stacDefReplaceInfo);

C_STRUCT(idl_stacDefReplaceInfo)
{
    /* The meta data structure */
    c_structure structure;
    /* Collection containing all indexes for which the member was replaced */
    c_iter replacedIndexes;
    /* Collection containing the replaced members */
    c_iter replacedMembers;
};

/* Contains the list of stac definitions */
static idl_stacDef idl_stacDefinitions;

static c_structure
idl_stacDefFindMetaStructureResolved(
    c_metaObject scope,
    const char *typeName);

static os_boolean
idl_stacListItemIsDefined (
    idl_stacDef stacDef,
    idl_scope scope,
    const char* typeName,
    const char* itemName);

static os_boolean
idl_stacListItemIsMemberLocated(
    const c_char* list,
    const char* itemName);

static c_baseObject
idl_stacDefResolveTypeDef(
    c_baseObject obj);

static os_boolean
idl_stacDefCanStacBeAppliedToMember(
    c_type memberType);

static c_type
idl_stacDefConvertStacApprovedMember(
    c_structure structure,
    c_type orgMemberType);

static os_int32
idl_stacDefFindMemberIndexByName(
    c_array members,
    const os_char* name);

static c_equality
idl_stacDefNamesAreEqual(
    void* exclusionName,
    void* memberName);

static os_boolean
idl_stacDefOnlyExclusionsDefined(
    os_char* stacList);

static os_boolean
idl_stacDefIsFieldExcluded(
    const os_char* stacList,
    const os_char* member);

/* Create a new stac map specified by scope,
   type name and staclist
*/
static idl_stacMap
idl_stacMapNew (
    const c_metaObject scope,
    const char *typeName,
    const char *stacList)
{
    idl_stacMap stacMap = os_malloc(C_SIZEOF(idl_stacMap));

    stacMap->stacList = os_strdup(stacList);
    stacMap->typeName = os_strdup(typeName);
    stacMap->scope = scope;

    return stacMap;
}

/* Free a stac map, without freeing the scope */
static void
idl_stacMapFree (
    const idl_stacMap stacMap)
{
    os_free (stacMap->typeName);
    os_free (stacMap->stacList);
    os_free (stacMap);
}

/* Create a new stac definition list */
idl_stacDef
idl_stacDefNew (
    void)
{
    idl_stacDef stacDef = os_malloc(C_SIZEOF(idl_stacDef));

    stacDef->stacList = c_iterNew (0);
    return stacDef;
}

/* Free a stac definition list, freeing all list elements */
void
idl_stacDefFree (
    idl_stacDef stacDef)
{
    idl_stacMap stacMap;

    if(stacDef)
    {
        while ((stacMap = c_iterTakeFirst (stacDef->stacList)))
        {
            idl_stacMapFree (stacMap);
        }
        os_free (stacDef);
    }
}

/* Add a stac definition to the specified stac definition list */
void
idl_stacDefAdd (
    idl_stacDef stacDef,
    c_metaObject scope,
    const char *typeName,
    const char *stacList)
{
    if(stacDef)
    {
        c_iterInsert (stacDef->stacList, idl_stacMapNew (scope, typeName, stacList));
    }
}

/*
 * Check if there is a stac applied to the given key.
 */
c_bool
idl_isStacDefFor(
    c_metaObject scope,
    c_char *typeName,
    c_char *key)
{
    idl_stacDef stacDef = idl_stacDefDefGet();
    idl_stacMap stacMap;
    c_long stacMapIdx;
    c_iter stacList;
    os_uint32 stacListSize;
    os_uint32 stacIdx;
    os_boolean stacDefFor = OS_FALSE;

    if (stacDef != NULL) {
        /* check all stac definition list elements */
        for (stacMapIdx = 0; stacMapIdx < c_iterLength(stacDef->stacList) && !stacDefFor; stacMapIdx++) {
            stacMap = c_iterObject(stacDef->stacList, stacMapIdx);
            if (c_metaCompare(scope, stacMap->scope) == E_EQUAL &&
                strcmp(typeName, stacMap->typeName) == 0)
            {
                /* for each stac in stacList, check if it's equal to key */
                stacList = c_splitString(stacMap->stacList, ",");
                stacListSize = c_iterLength(stacList);
                if (stacListSize == 0) {
                    stacDefFor = OS_TRUE;
                }
                else if(idl_stacDefOnlyExclusionsDefined(stacMap->stacList))
                {
                    if(!idl_stacDefIsFieldExcluded(stacMap->stacList, key))
                    {
                        stacDefFor = OS_TRUE;
                    }
                } else
                {
                    for(stacIdx = 0; stacIdx < stacListSize; stacIdx++)
                    {
                        if (strcmp(c_iterTakeFirst(stacList), key) == 0) {
                            stacDefFor = OS_TRUE;
                        }
                    }
                }
            }
        }
    }
    return stacDefFor;
}



os_boolean
idl_stacListItemIsMemberLocated(
    const c_char* list,
    const char* itemName)
{
    os_boolean isDefined = OS_FALSE;

    if(list)
    {
        c_iter items;
        c_char* item;

        items = c_splitString(list, ",");
        while(c_iterLength(items) > 0 && !isDefined)
        {
            item = c_iterTakeFirst(items);
            if(item && 0 == strcmp(item, itemName))
            {
                isDefined = OS_TRUE;
            }
        }
    }

    return isDefined;
}

os_boolean
idl_stacDef_isStacDefined(
    idl_scope scope,
    const char *name,
    idl_typeSpec typeSpec,
    idl_typeSpec* baseStringTypeDereffered)
{
    os_boolean isStacDefined = OS_FALSE;
    idl_typeSpec typeDereffered;
    idl_typeSpec subType;
    idl_scope tmpScope;
    os_char* containingElement;
    idl_basicType basic;
    c_long maxlen;

    /* resolve any type defs */
    typeDereffered = idl_typeDefResolveFully(typeSpec);
    if(idl_typeSpecType(typeDereffered) == idl_tarray)
    {
        /* If this is an array, then get the sub type and recurse deeper into this
         * operation. Arrays of bounded strings are allowed for pragma stac
         */
        subType = idl_typeArrayActual(idl_typeArray(typeDereffered));
        isStacDefined = idl_stacDef_isStacDefined(scope, name, subType, baseStringTypeDereffered);
    } else if(idl_typeSpecType(typeDereffered) == idl_tbasic)
    {
        /* Get the basic type to see if it is a string */
        basic = idl_typeBasicType(idl_typeBasic(typeDereffered));
        if(basic == idl_string)
        {
            /* If this is indeed a string, then get the 'maxLen' attribute of the
             * string type. If it is not 0, then this is a bounded string
             */
            maxlen = idl_typeBasicMaxlen(idl_typeBasic(typeDereffered));
            if(maxlen != 0)
            {
                tmpScope = idl_scopeDup(scope);
                containingElement = idl_scopeElementName(idl_scopeCur (scope));
                idl_scopePop(tmpScope);
                isStacDefined = idl_stacListItemIsDefined (idl_stacDefDefGet(), tmpScope, containingElement, name);
                if(isStacDefined && baseStringTypeDereffered)
                {
                    *baseStringTypeDereffered = typeDereffered;
                }
            } /* else stac define status not relevant for this member */
        } /* else stac define status not relevant for this member */
    } /* else stac define status not relevant for this member */
    return isStacDefined;
}

/* Find the stac list related to the specified typename in
   the specified scope
*/
os_boolean
idl_stacListItemIsDefined (
    idl_stacDef stacDef,
    idl_scope scope,
    const char *typeName,
    const char* itemName)
{
    c_long li;
    c_long si;
    idl_stacMap stacMap;
    c_metaObject typeScope;
    os_boolean isDefined = OS_FALSE;

    if(stacDef)
    {
        li = 0;
        /* check all stac definition list elements */
        while (li < c_iterLength (stacDef->stacList) && !isDefined)
        {
            stacMap = c_iterObject (stacDef->stacList, li);
            if (strcmp(typeName, stacMap->typeName) == 0)
            {
                /* if the typename equals, check if the scope compares */
                if ((idl_scopeStackSize(scope) == 0) &&
                    (stacMap->scope->definedIn == NULL))
                {
                    /* We're in the global scope */

                    /* If no members were defined for this type, then we will
                     * interprete this as a request for all bounded string
                     * members to be converted to a char array internally.
                     */
                    if(strlen(stacMap->stacList) == 0)
                    {
                        isDefined = OS_TRUE;
                    }
                    else if(idl_stacDefOnlyExclusionsDefined(stacMap->stacList))
                    {
                        if(idl_stacDefIsFieldExcluded(stacMap->stacList, itemName))
                        {
                            isDefined = OS_FALSE;
                        } else
                        {
                            isDefined = OS_TRUE;
                        }
                    } else
                    {
                        isDefined = idl_stacListItemIsMemberLocated(stacMap->stacList, itemName);
                    }
                }
                if(!isDefined)
                {
                    si = idl_scopeStackSize (scope)-1;
                    typeScope = stacMap->scope;
                    while (si >= 0)
                    {
                        /* for each scope element */
                        if (idl_scopeElementType(idl_scopeIndexed (scope, si)) == idl_tModule &&
                            strcmp (typeScope->name, idl_scopeElementName(idl_scopeIndexed (scope, si))) == 0)
                        {
                            /* the scope is a module and the scope name compares */
                            si--;
                            if (typeScope)
                            {
                                typeScope = typeScope->definedIn;
                            }
                            if (si == -1)
                            {
                            /* bottom of the stack is reached */
                                if (typeScope == NULL || typeScope->name == NULL)
                                {
                                    /* the typeScope has reached the bottom too,
                                     * thus the scopes are equal
                                     */

                                    /* If no members were defined for this type, then we will
                                     * interprete this as a request for all bounded string
                                     * members to be converted to a char array internally.
                                     */
                                    if(strlen(stacMap->stacList) == 0)
                                    {
                                        isDefined = OS_TRUE;
                                    }
                                    else if(idl_stacDefOnlyExclusionsDefined(stacMap->stacList))
                                    {
                                        if(idl_stacDefIsFieldExcluded(stacMap->stacList, itemName))
                                        {
                                            isDefined = OS_FALSE;
                                        } else
                                        {
                                            isDefined = OS_TRUE;
                                        }
                                    } else
                                    {
                                        isDefined = idl_stacListItemIsMemberLocated(stacMap->stacList, itemName);
                                    }
                                }
                            }
                        }
                        else
                        {
                            si = -1;
                        }
                    }
                }
            }
            li++;
        }
    }
    return isDefined;
}

/* Set the default stac definition list */
void
idl_stacDefDefSet (
    idl_stacDef stacDef)
{
    idl_stacDefinitions = stacDef;
}

/* Get the default stac definition list, may be null */
idl_stacDef
idl_stacDefDefGet (
    void)
{
    return idl_stacDefinitions;
}

/* This operation scans through the list of items within the 'stac' pragma.
 * For each structure that is identified it will locate the corresponding
 * meta structure. It will then locate each member mentioned in the 'stac'
 * pragma within the member list of said meta structure. It will verify the
 * located member is indeed a character array.
 * It will then proceed to replace the meta data describing the located member
 * with new meta data with as goal to have the new meta data identify the member
 * as a string instead of as a character array.
 * All replaced meta data is stored in out variables to facilitate restore the
 * member list of the meta structure back into it's original configuration.
 * This operation is needed then the meta data of the found structure is
 * converted to XML. As the XML generation code is located in the database and
 * we do not want the database to get knowledge of the 'stac' pragma.
 */
c_iter
idl_stacDefConvertAll(
    idl_stacDef stacDef)
{
    os_uint32 size;
    os_uint32 i;
    idl_stacMap stacMapItem;
    c_structure structure;
    c_iter memberNames;
    os_uint32 memberNamesSize;
    os_uint32 j;
    os_char* memberName;
    os_int32 memberIndex;
    c_member member;
    c_member newMember;
    os_uint32* replacedIndex;
    idl_stacDefReplaceInfo replaceData;
    c_iter replaceInfo = NULL;
    c_base base;
    c_iter boundedStringToBeConverted;
    os_boolean stacCanBeApplied = OS_TRUE;
    os_boolean onlyExclusionlistings;

    if(stacDef)
    {

        /* Create collections to hold the original members and their respective
         * indexes in the member collection so we can easily restore the meta
         * structure to it's original configuration at a later time.
         */
        replaceInfo = c_iterNew(NULL);
        size = c_iterLength (stacDef->stacList);
        for(i = 0; i < size; i++)
        {
            stacMapItem = c_iterObject (stacDef->stacList, i);
            /* find the matching structure in the meta data */
            structure = idl_stacDefFindMetaStructureResolved(
                stacMapItem->scope,
                stacMapItem->typeName);
            assert(structure);
            replaceData = os_malloc(C_SIZEOF(idl_stacDefReplaceInfo));
            replaceData->structure = structure;
            replaceData->replacedIndexes = c_iterNew(NULL);
            replaceData->replacedMembers = c_iterNew(NULL);
            memberNames = c_splitString(stacMapItem->stacList, ",");
            onlyExclusionlistings = idl_stacDefOnlyExclusionsDefined(stacMapItem->stacList);
            memberNamesSize = c_iterLength(memberNames);
            boundedStringToBeConverted = c_iterNew(NULL);
            if(memberNamesSize == 0 || onlyExclusionlistings)
            {
                os_uint32 membersSize;
                membersSize = c_arraySize(structure->members);
                for(j = 0; j < membersSize; j++)
                {
                    member = c_member(structure->members[j]);
                    memberName = c_specifier(member)->name;
                    /* check if this member is in the list of member names when
                     * the member names list contains exclusion listings
                     */
                    if((onlyExclusionlistings && c_iterResolve(memberNames, idl_stacDefNamesAreEqual, memberName) == NULL) ||
                       memberNamesSize == 0)
                    {
                        stacCanBeApplied = idl_stacDefCanStacBeAppliedToMember(c_specifier(member)->type);
                        if(stacCanBeApplied)
                        {
                            /* this is a bounded string, so we want to convert */
                            c_iterInsert(boundedStringToBeConverted, member);
                        }
                    }
                }
            }
            else
            {
                for(j = 0; j < memberNamesSize; j++)
                {
                    memberName = c_iterTakeFirst(memberNames);
                    if(memberName[0] == '!')
                    {
                        printf("FATAL ERROR | #pragma stac: Illegal syntax combination detected. "
                               "The pragma stac definition for structure %s contains both normal "
                               "member listings (without the '!' character in front of them) as "
                               "well as exclusion member listings (with the '!' character in front "
                               "of them). This has no relevant meaning, please see the deployment manual "
                               "for information on usage of pragma stac.\n"
                               "Ignoring the following member defintion: '%s'\n",
                               c_metaScopedName(c_metaObject(structure)),
                               memberName);
                        exit(-2);
                    }
                    memberIndex = idl_stacDefFindMemberIndexByName(
                        structure->members,
                        memberName);
                    if(memberIndex == -1)
                    {
                        printf("FATAL ERROR | #pragma stac: Unable to locate member %s "
                            "within structure %s.\n",
                            memberName, c_metaScopedName(c_metaObject(structure)));
                        exit(-2);
                    }
                    member = structure->members[memberIndex];
                    /* Verify the member is a bounded string as required */
                    stacCanBeApplied = idl_stacDefCanStacBeAppliedToMember(c_specifier(member)->type);
                    if(!stacCanBeApplied)
                    {
                        printf("FATAL ERROR | #pragma stac: Member %s within structure "
                            "%s is not a bounded string (note: may be embedded within an array or sequence) as required.\n",
                            memberName, c_metaScopedName(c_metaObject(structure)));
                            assert(0);
                        exit(-2);
                    }
                    /* this is a bounded string, so we want to convert */
                    c_iterInsert(boundedStringToBeConverted, member);
                }

            }

            while(c_iterLength(boundedStringToBeConverted) > 0)
            {
                member = c_iterTakeFirst(boundedStringToBeConverted);
                memberIndex = idl_stacDefFindMemberIndexByName(
                    structure->members,
                    c_specifier(member)->name);
                assert(memberIndex != -1);
                newMember = c_metaDefine(c_metaObject(structure), M_MEMBER);
                base = c_getBase(member);
                c_specifier(newMember)->name = c_stringNew(base, c_specifier(member)->name);
                 c_specifier(newMember)->type = idl_stacDefConvertStacApprovedMember(structure, c_specifier(member)->type);
                if(!c_specifier(newMember)->type)
                {
                    printf("FATAL ERROR | #pragma stac: An internal error occured. Member %s within structure "
                        "%s failed to convert from a bounded string to a character array.\n",
                        c_specifier(newMember)->name, c_metaScopedName(c_metaObject(structure)));
                        assert(0);
                    exit(-2);
                }
                structure->members[memberIndex] = newMember;
                c_iterInsert(replaceData->replacedMembers, member);
                replacedIndex = os_malloc(sizeof(os_uint32));
                *replacedIndex = memberIndex;
                c_iterInsert(replaceData->replacedIndexes, replacedIndex);
            }
            c_iterInsert(replaceInfo, replaceData);
        }
    }
    return replaceInfo;
}

os_boolean
idl_stacDefOnlyExclusionsDefined(
    os_char* stacList)
{
    os_boolean onlyExclusions = OS_TRUE;
    c_iter memberNames;
    os_char* memberName;

    memberNames = c_splitString(stacList, ",");
    memberName = c_iterTakeFirst(memberNames);
    while(memberName)
    {
        if(strlen(memberName) > 0 && memberName[0] != '!')
        {
            onlyExclusions = OS_FALSE;
        }
        memberName = c_iterTakeFirst(memberNames);
    }
    return onlyExclusions;
}

os_boolean
idl_stacDefIsFieldExcluded(
    const os_char* stacList,
    const os_char* member)
{
    os_boolean isExcluded = OS_FALSE;
    c_iter memberNames;
    os_char* memberName;

    memberNames = c_splitString(stacList, ",");
    memberName = c_iterTakeFirst(memberNames);
    while(memberName)
    {
        if(strlen(memberName) > 1 && memberName[0] == '!')
        {
            if(0 == strcmp(member, &(memberName[1])))
            {
                isExcluded = OS_TRUE;
            }
        }
        memberName = c_iterTakeFirst(memberNames);
    }
    return isExcluded;
}

c_equality
idl_stacDefNamesAreEqual(
    void *_exclusionName,
    void *_memberName)
{
    os_char* exclusionName;
    os_char* memberName;
    c_equality eq = C_NE;

    exclusionName = _exclusionName;
    memberName = _memberName;

    if(strlen(exclusionName) > 1 && exclusionName[0] == '!')
    {
        if(0 == strcmp(&(exclusionName[1]), memberName))
        {
            eq = C_EQ;
        }/* else C_NE */
    } else if(0 == strcmp(exclusionName, memberName))
    {
        eq = C_EQ;
    } /* else C_NE */
    return eq;
}

os_boolean
idl_stacDefCanStacBeAppliedToMember(
    c_type memberType)
{
    os_boolean stacCanBeApplied = OS_FALSE;
    c_type dereffedType;

    dereffedType = c_typeActualType(c_type(idl_stacDefResolveTypeDef(c_baseObject(memberType))));
    if(c_baseObject(dereffedType)->kind == M_COLLECTION)
    {
        /* Can be a string or an array or a sequence */
        if((c_collectionType(dereffedType)->kind == C_STRING))
        {
            if((c_collectionType(dereffedType)->maxSize != 0))
            {
                /* The string is bounded, stac can be applied */
                stacCanBeApplied = OS_TRUE;
            }
        }
        else if((c_collectionType(dereffedType)->kind == C_ARRAY))
        {
            /* bounded string embedded within arrays are allowed, recurse deeper */
            stacCanBeApplied = idl_stacDefCanStacBeAppliedToMember(c_collectionType(dereffedType)->subType);
        }
        else if((c_collectionType(dereffedType)->kind == C_SEQUENCE))
        {
            /* bounded string embedded within sequences are allowed, recurse deeper */
            stacCanBeApplied = idl_stacDefCanStacBeAppliedToMember(c_collectionType(dereffedType)->subType);
        } /* else let memberIsStacOk remain false */
    }
    return stacCanBeApplied;
}

c_type
idl_stacDefConvertStacApprovedMember(
    c_structure structure,
    c_type orgMemberType)
{
    c_type dereffedOrgType;
    c_metaObject o = NULL;
    c_type newType;
    os_char buffer[1024];
    c_metaObject found;

    memset(buffer, 0, 1024);
    dereffedOrgType = c_typeActualType(c_type(idl_stacDefResolveTypeDef(c_baseObject(orgMemberType))));
    if(c_baseObject(dereffedOrgType)->kind == M_COLLECTION)
    {
        o = c_metaObject(c_metaDefine(c_metaObject(structure), M_COLLECTION));
        /* Can be a string or an array or a sequence */
        if((c_collectionType(dereffedOrgType)->kind == C_STRING))
        {
            if((c_collectionType(dereffedOrgType)->maxSize != 0))
            {
                c_collectionType(o)->kind = C_ARRAY;
                c_collectionType(o)->subType = c_type(c_metaResolve(c_metaObject(structure), "c_char"));
                /* increase maxSize with 1 to accomodate the '\0' char found in strings */
                c_collectionType(o)->maxSize = c_collectionType(dereffedOrgType)->maxSize + 1;
                os_sprintf(
                    buffer,
                    "C_ARRAY<%s,%d>",
                    c_metaObject(c_collectionType(o)->subType)->name,
                    c_collectionType(o)->maxSize/*use maxSize of new c_type, must be the same*/);
            }
            else
            {
                printf("FATAL ERROR | #pragma stac: Trying to apply stac algorithm to listed members of struct %s, but "
                       "encountered an internal error. The conversion algorithm (idl_stacDefConvertStacApprovedMember) "
                       "has become out of synch with the verification algorithm (idl_stacDefCanStacBeAppliedToMember).\n",
                       c_metaScopedName(c_metaObject(structure)));
                assert(0);
                exit(-2);
            }
        }
        else if((c_collectionType(dereffedOrgType)->kind == C_ARRAY))
        {
            c_collectionType(o)->kind = C_ARRAY;
            /* increase maxSize with 1 to accomodate the '\0' char found in strings */
            c_collectionType(o)->maxSize = c_collectionType(dereffedOrgType)->maxSize;
            c_collectionType(o)->subType = idl_stacDefConvertStacApprovedMember(structure, c_collectionType(dereffedOrgType)->subType);
            os_sprintf(
                buffer,
                "C_ARRAY<%s,%d>",
                c_metaObject(c_collectionType(o)->subType)->name,
                c_collectionType(o)->maxSize/*use maxSize of new c_type, must be the same*/);
        }
        else if((c_collectionType(dereffedOrgType)->kind == C_SEQUENCE))
        {
            c_collectionType(o)->kind = C_SEQUENCE;
            /* increase maxSize with 1 to accomodate the '\0' char found in strings */
            c_collectionType(o)->maxSize = c_collectionType(dereffedOrgType)->maxSize;
            c_collectionType(o)->subType = idl_stacDefConvertStacApprovedMember(structure, c_collectionType(dereffedOrgType)->subType);
            os_sprintf(
                buffer,
                "C_SEQUENCE<%s,%d>",
                c_metaObject(c_collectionType(o)->subType)->name,
                c_collectionType(o)->maxSize/*use maxSize of new c_type, must be the same*/);
        }
        else
        {
            printf("FATAL ERROR | #pragma stac: Trying to apply stac algorithm to listed members of struct %s, but "
                   "encountered an internal error. The conversion algorithm (idl_stacDefConvertStacApprovedMember) "
                   "has become out of synch with the verification algorithm (idl_stacDefCanStacBeAppliedToMember).\n",
                   c_metaScopedName(c_metaObject(structure)));
            assert(0);
            exit(-2);
        }
    }
    else
    {
        printf("FATAL ERROR | #pragma stac: Trying to apply stac algorithm to listed members of struct %s, but "
               "encountered an internal error. The conversion algorithm (idl_stacDefConvertStacApprovedMember) "
               "has become out of synch with the verification algorithm (idl_stacDefCanStacBeAppliedToMember).\n",
               c_metaScopedName(c_metaObject(structure)));
        assert(0);
        exit(-2);
    }
    if(o)
    {
        c_metaObject(o)->definedIn = c_metaObject(structure);
        c_metaFinalize(o);
        found = c_metaBind(c_metaObject(structure), &buffer[0], o);
        c_free(o);
        newType = c_type(found);
    }
    else
    {
        printf("FATAL ERROR | #pragma stac: Trying to apply stac algorithm to listed members of struct %s, but "
               "encountered an internal error. The conversion algorithm (idl_stacDefConvertStacApprovedMember) "
               "has become out of synch with the verification algorithm (idl_stacDefCanStacBeAppliedToMember).\n",
               c_metaScopedName(c_metaObject(structure)));
        assert(0);
        exit(-2);
    }
    return newType;
}

void
idl_stacDefRestoreAll(
    idl_stacDef stacDef,
    c_iter replaceInfos)
{
    os_uint32 size;
    os_uint32 i;
    idl_stacDefReplaceInfo info;
    os_uint32 j;
    os_uint32 replacedMembersSize;

    assert(replaceInfos);

    if(stacDef)
    {

        size = c_iterLength (replaceInfos);
        for(i = 0; i < size; i++)
        {
            info = c_iterTakeFirst(replaceInfos);
            replacedMembersSize = c_iterLength(info->replacedMembers);
            assert(replacedMembersSize == (os_uint32)c_iterLength(info->replacedIndexes));
            for(j = 0; j < replacedMembersSize; j++)
            {
                os_uint32* index;
                c_member member;

                index = c_iterTakeFirst(info->replacedIndexes);
                member = c_iterTakeFirst(info->replacedMembers);

                info->structure->members[*index] = member;
                os_free(index);
            }
            c_iterFree(info->replacedMembers);
            c_iterFree(info->replacedIndexes);
            os_free(info);
        }
        c_iterFree(replaceInfos);
    }
}

c_structure
idl_stacDefFindMetaStructureResolved(
    c_metaObject scope,
    const char *typeName)
{
    c_baseObject object;
    c_structure structure;

    object = c_baseObject(c_metaResolve(scope, typeName));

    if(!object)
    {
        printf("FATAL ERROR | #pragma stac: Trying to locate structure '%s' in "
            "scope '%s'. But no such object exists.\n",
            typeName, c_metaScopedName(c_metaObject(scope)));
        exit(-2);
    }
    /* Resolve typedefs */
    idl_stacDefResolveTypeDef(object);
    /* The final object (after typedef resolving) should be a structure */
    if(object->kind != M_STRUCTURE)
    {
        printf("FATAL ERROR | #pragma stac: Trying to locate structure '%s' in "
            "scope '%s'. But the identified object is not a structure.\n",
            typeName, c_metaScopedName(c_metaObject(scope)));
        exit(-2);
    }
    structure = c_structure(object);

    return structure;
}

c_baseObject
idl_stacDefResolveTypeDef(
    c_baseObject obj)
{
    c_baseObject object;

    object = obj;
    while(object->kind == M_TYPEDEF)
    {
        object = c_baseObject(c_typeDef(object)->alias);
    }
    return object;
}

os_int32
idl_stacDefFindMemberIndexByName(
    c_array members,
    const os_char* name)
{
    os_uint32 membersSize;
    os_int32 memberIndex = -1;
    c_member member;
    os_uint32 i;

    membersSize = c_arraySize(members);
    for(i = 0; i < membersSize && memberIndex == -1 ; i++)
    {
        member = c_member(members[i]);
        if(0 == strcmp(c_specifier(member)->name, name))
        {
            memberIndex = i;
        }
    }
    return memberIndex;
}
