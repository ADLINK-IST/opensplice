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
#include "idl_catsDef.h"


C_CLASS(idl_catsDefReplaceInfo);

C_STRUCT(idl_catsDefReplaceInfo)
{
    /* The meta data structure */
    c_structure structure;
    /* Collection containing all indexes for which the member was replaced */
    c_iter replacedIndexes;
    /* Collection containing the replaced members */
    c_iter replacedMembers;
};

/* Contains the list of cats definitions */
static idl_catsDef idl_catsDefinitions;

static os_boolean
idl_catsListItemIsDefined (
    idl_catsDef catsDef,
    idl_scope scope,
    const char* typeName,
    const char* itemName);

static c_structure
idl_catsDefFindMetaStructureResolved(
    c_metaObject scope,
    const char *typeName);

static os_boolean
idl_catsListItemIsMemberLocated(
    const c_char* list,
    const char* itemName);

static c_baseObject
idl_catsDefResolveTypeDef(
    c_baseObject obj);

static os_int32
idl_catsDefFindMemberIndexByName(
    c_array members,
    const os_char* name);

/* Create a new cats map specified by scope,
   type name and catslist
*/
static idl_catsMap
idl_catsMapNew (
    const c_metaObject scope,
    const char *typeName,
    const char *catsList)
{
    idl_catsMap catsMap = os_malloc(C_SIZEOF(idl_catsMap));

    catsMap->catsList = os_strdup(catsList);
    catsMap->typeName = os_strdup(typeName);
    catsMap->scope = scope;

    return catsMap;
}

/* Free a cats map, without freeing the scope */
static void
idl_catsMapFree (
    const idl_catsMap catsMap)
{
    os_free (catsMap->typeName);
    os_free (catsMap->catsList);
    os_free (catsMap);
}

/* Create a new cats definition list */
idl_catsDef
idl_catsDefNew (
    void)
{
    idl_catsDef catsDef = os_malloc(C_SIZEOF(idl_catsDef));

    catsDef->catsList = c_iterNew (0);
    return catsDef;
}

/* Free a cats definition list, freeing all list elements */
void
idl_catsDefFree (
    idl_catsDef catsDef)
{
    idl_catsMap catsMap;

    if(catsDef)
    {
        while ((catsMap = c_iterTakeFirst (catsDef->catsList)))
        {
            idl_catsMapFree (catsMap);
        }
        os_free (catsDef);
    }
}

/* Add a cats definition to the specified cats definition list */
void
idl_catsDefAdd (
    idl_catsDef catsDef,
    c_metaObject scope,
    const char *typeName,
    const char *catsList)
{
    if(catsDef)
    {
        c_iterInsert (catsDef->catsList, idl_catsMapNew (scope, typeName, catsList));
    }
}

/*
 * Check if there is a cats applied to the given key.
 */
c_bool
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
                if (catsListSize == 0) {
                    return OS_TRUE;
                }
                else
                {
                    for(catsIdx = 0; catsIdx < catsListSize; catsIdx++)
                    {
                        if (strcmp(c_iterTakeFirst(catsList), key) == 0) {
                            return OS_TRUE;
                        }
                    }
                }
            }
        }
    }

    return OS_FALSE;
}


os_boolean
idl_catsDef_isCatsDefined(
    idl_scope scope,
    const char *name,
    idl_typeSpec typeSpec)
{
    os_boolean isCatsDefined = OS_FALSE;
    idl_typeSpec typeDereffered;
    idl_typeSpec subType;
    idl_basicType basic;
    idl_scope tmpScope;
    os_char* containingElement;

    typeDereffered = idl_typeDefResolveFully(typeSpec);
    if(idl_typeSpecType(typeDereffered) == idl_tarray)
    {
        subType = idl_typeDefResolveFully(idl_typeArrayActual(idl_typeArray(typeDereffered)));
        if(idl_typeSpecType(subType) == idl_tbasic)
        {
            basic = idl_typeBasicType(idl_typeBasic(subType));
            if(basic == idl_char)
            {
                tmpScope = idl_scopeDup(scope);
                containingElement = idl_scopeElementName(idl_scopeCur (scope));
                idl_scopePop(tmpScope);
                isCatsDefined = idl_catsListItemIsDefined (idl_catsDefDefGet(), tmpScope, containingElement, name);
            }
        }
    }
    return isCatsDefined;
}

os_boolean
idl_catsListItemIsMemberLocated(
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

/* Find the cats list related to the specified typename in
   the specified scope
*/
os_boolean
idl_catsListItemIsDefined (
    idl_catsDef catsDef,
    idl_scope scope,
    const char *typeName,
    const char* itemName)
{
    c_long li;
    c_long si;
    idl_catsMap catsMap;
    c_metaObject typeScope;
    os_boolean isDefined = OS_FALSE;

    if(catsDef)
    {
        li = 0;
        /* check all cats definition list elements */
        while (li < c_iterLength (catsDef->catsList) && !isDefined)
        {
            catsMap = c_iterObject (catsDef->catsList, li);
            if (strcmp(typeName, catsMap->typeName) == 0)
            {
                /* if the typename equals, check if the scope compares */
                if ((idl_scopeStackSize(scope) == 0) &&
                    (catsMap->scope->definedIn == NULL))
                {
                    /* We're in the global scope */

                    /* If no members were defined for this type, then we will
                     * interprete this as a request for all char array
                     * members to be converted to a string internally.
                     */
                    if(strlen(catsMap->catsList) == 0)
                    {
                        isDefined = OS_TRUE;
                    } else
                    {
                        isDefined = idl_catsListItemIsMemberLocated(catsMap->catsList, itemName);
                    }
                }
                if(!isDefined)
                {
                    si = idl_scopeStackSize (scope)-1;
                    typeScope = catsMap->scope;
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
                                     * interprete this as a request for all char array
                                     * members to be converted to a string internally.
                                     */
                                    if(strlen(catsMap->catsList) == 0)
                                    {
                                        isDefined = OS_TRUE;
                                    } else
                                    {
                                        isDefined = idl_catsListItemIsMemberLocated(catsMap->catsList, itemName);
                                    }
                                }
                            }
                        } else
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

/* Set the default cats definition list */
void
idl_catsDefDefSet (
    idl_catsDef catsDef)
{
    idl_catsDefinitions = catsDef;
}

/* Get the default cats definition list, may be null */
idl_catsDef
idl_catsDefDefGet (
    void)
{
    return idl_catsDefinitions;
}

/* This operation scans through the list of items within the 'cats' pragma.
 * For each structure that is identified it will locate the corresponding
 * meta structure. It will then locate each member mentioned in the 'cats'
 * pragma within the member list of said meta structure. It will verify the
 * located member is indeed a character array.
 * It will then proceed to replace the meta data describing the located member
 * with new meta data with as goal to have the new meta data identify the member
 * as a string instead of as a character array.
 * All replaced meta data is stored in out variables to facilitate restore the
 * member list of the meta structure back into it's original configuration.
 * This operation is needed then the meta data of the found structure is
 * converted to XML. As the XML generation code is located in the database and
 * we do not want the database to get knowledge of the 'cats' pragma.
 */
c_iter
idl_catsDefConvertAll(
    idl_catsDef catsDef)
{
    os_uint32 size;
    os_uint32 i;
    idl_catsMap catsMapItem;
    c_structure structure;
    c_iter memberNames;
    os_uint32 memberNamesSize;
    os_uint32 j;
    os_char* memberName;
    os_int32 memberIndex;
    c_member member;
    c_type memberType;
    c_type subType;
    c_member newMember;
    os_uint32* replacedIndex;
    idl_catsDefReplaceInfo replaceData;
    c_iter replaceInfo = NULL;
    c_base base;
    os_char buffer[64];
    c_metaObject found;
    c_metaObject o;
    c_iter charArrayToBeConverted;

    if(catsDef)
    {

        /* Create collections to hold the original members and their respective
         * indexes in the member collection so we can easily restore the meta
         * structure to it's original configuration at a later time.
         */
        replaceInfo = c_iterNew(NULL);
        size = c_iterLength (catsDef->catsList);
        for(i = 0; i < size; i++)
        {
            catsMapItem = c_iterObject (catsDef->catsList, i);
            /* find the matching structure in the meta data */
            structure = idl_catsDefFindMetaStructureResolved(
                catsMapItem->scope,
                catsMapItem->typeName);
            assert(structure);
            replaceData = os_malloc(C_SIZEOF(idl_catsDefReplaceInfo));
            replaceData->structure = structure;
            replaceData->replacedIndexes = c_iterNew(NULL);
            replaceData->replacedMembers = c_iterNew(NULL);
            memberNames = c_splitString(catsMapItem->catsList, ",");
            memberNamesSize = c_iterLength(memberNames);
            charArrayToBeConverted = c_iterNew(NULL);
            if(memberNamesSize == 0)
            {
                os_uint32 membersSize;
                membersSize = c_arraySize(structure->members);
                for(j = 0; j < membersSize; j++)
                {
                    member = c_member(structure->members[j]);
                    memberType = c_typeActualType(c_type(idl_catsDefResolveTypeDef(c_baseObject(c_specifier(member)->type))));
                    if((c_baseObject(memberType)->kind == M_COLLECTION))
                    {
                        subType = c_typeActualType(c_collectionType(memberType)->subType);
                        if(c_baseObject(subType)->kind == M_PRIMITIVE &&
                            c_primitive(subType)->kind == P_CHAR)
                        {
                            /* this is a char array, so we want to convert */
                            c_iterInsert(charArrayToBeConverted, member);
                        }
                    }
                }
            } else
            {
                for(j = 0; j < memberNamesSize; j++)
                {
                    memberName = c_iterTakeFirst(memberNames);
                    memberIndex = idl_catsDefFindMemberIndexByName(
                        structure->members,
                        memberName);
                    if(memberIndex == -1)
                    {
                        printf("FATAL ERROR | #pragma cats: Unable to locate member %s "
                            "within structure %s.\n",
                            memberName, c_metaScopedName(c_metaObject(structure)));
                        exit(-2);
                    }
                    member = structure->members[memberIndex];
                    /* Verify the member is a char array as required */
                    memberType = c_typeActualType(c_type(idl_catsDefResolveTypeDef(c_baseObject(c_specifier(member)->type))));
                    if((c_baseObject(memberType)->kind != M_COLLECTION))
                    {
                        printf("FATAL ERROR | #pragma cats: Member %s within structure "
                            "%s is not a character array as required.\n",
                            memberName, c_metaScopedName(c_metaObject(structure)));
                            assert(0);
                        exit(-2);
                    }
                    subType = c_typeActualType(c_collectionType(memberType)->subType);
                    if(c_baseObject(subType)->kind != M_PRIMITIVE ||
                        c_primitive(subType)->kind != P_CHAR)
                    {
                        printf("FATAL ERROR | #pragma cats: Member %s within structure "
                            "%s is not a character array as required.\n",
                            memberName, c_metaScopedName(c_metaObject(structure)));
                            assert(0);
                        exit(-2);
                    }
                    /* this is a char array , so we want to convert */
                    c_iterInsert(charArrayToBeConverted, member);
                }
            }
            while(c_iterLength(charArrayToBeConverted) > 0)
            {
                member = c_iterTakeFirst(charArrayToBeConverted);
                memberIndex = idl_catsDefFindMemberIndexByName(
                    structure->members,
                    c_specifier(member)->name);
                assert(memberIndex != -1);
                newMember = c_metaDefine(c_metaObject(structure), M_MEMBER);
                base = c_getBase(member);
                c_specifier(newMember)->name = c_stringNew(base, c_specifier(member)->name);
                o = c_metaObject(c_metaDefine(c_metaObject(structure), M_COLLECTION));
                c_collectionType(o)->kind = C_STRING;
                c_collectionType(o)->subType = c_type(c_metaResolve(c_metaObject(structure), "c_char"));
                c_collectionType(o)->maxSize = c_collectionType(idl_catsDefResolveTypeDef(c_baseObject(c_specifier(member)->type)))->maxSize;
                c_metaObject(o)->definedIn = c_metaObject(structure);
                c_metaFinalize(o);
                memset(buffer, 0, 64);
                os_sprintf(buffer, "C_STRING<%d>", c_collectionType(idl_catsDefResolveTypeDef(c_baseObject(c_specifier(member)->type)))->maxSize);
                found = c_metaBind(c_metaObject(structure), &buffer[0], o);
                c_free(o);
                c_specifier(newMember)->type = c_type(found);
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

void
idl_catsDefRestoreAll(
    idl_catsDef catsDef,
    c_iter replaceInfos)
{
    os_uint32 size;
    os_uint32 i;
    idl_catsDefReplaceInfo info;
    os_uint32 j;
    os_uint32 replacedMembersSize;

    assert(replaceInfos);

    if(catsDef)
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
idl_catsDefFindMetaStructureResolved(
    c_metaObject scope,
    const char *typeName)
{
    c_baseObject object;
    c_structure structure;

    object = c_baseObject(c_metaResolve(scope, typeName));

    if(!object)
    {
        printf("FATAL ERROR | #pragma cats: Trying to locate structure '%s' in "
            "scope '%s'. But no such object exists.\n",
            typeName, c_metaScopedName(c_metaObject(scope)));
        exit(-2);
    }
    /* Resolve typedefs */
    idl_catsDefResolveTypeDef(object);
    /* The final object (after typedef resolving) should be a structure */
    if(object->kind != M_STRUCTURE)
    {
        printf("FATAL ERROR | #pragma cats: Trying to locate structure '%s' in "
            "scope '%s'. But the identified object is not a structure.\n",
            typeName, c_metaScopedName(c_metaObject(scope)));
        exit(-2);
    }
    structure = c_structure(object);

    return structure;
}

c_baseObject
idl_catsDefResolveTypeDef(
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
idl_catsDefFindMemberIndexByName(
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
