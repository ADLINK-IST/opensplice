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

#include "gapi.h"

gapi_type
gapi_typeActualType(gapi_type typeBase)
{
    return c_typeActualType(typeBase);
}

c_metaKind
gapi_metaData_baseObjectKind(gapi_baseObject objBase)
{
    return c_baseObjectKind(objBase);
}

gapi_type
gapi_metaData_specifierType(gapi_specifier specBase)
{
    return c_specifierType(specBase);
}

const gapi_char *
gapi_metaData_specifierName(gapi_specifier specBase)
{
    return c_specifierName(specBase);
}

gapi_long
gapi_metaData_typeSize(gapi_type typeBase)
{
    return c_typeSize(typeBase);
}

gapi_long
gapi_metaData_enumerationCount(gapi_enumeration enumBase)
{
    return c_enumerationCount(enumBase);
}

c_primKind
gapi_metaData_primitiveKind(gapi_primitive primBase)
{
    return c_primitiveKind(primBase);
}

c_collKind
gapi_metaData_collectionTypeKind(gapi_collectionType collBase)
{
    return c_collectionTypeKind(collBase);
}

gapi_long
gapi_metaData_collectionTypeMaxSize(gapi_collectionType collBase)
{
    return c_collectionTypeMaxSize(collBase);
}

gapi_type
gapi_metaData_collectionTypeSubType(gapi_collectionType collBase)
{
    return c_collectionTypeSubType(collBase);
}

gapi_long
gapi_metaData_structureMemberCount(gapi_structure structBase)
{
    return c_structureMemberCount(structBase);
}

gapi_member
gapi_metaData_structureMember(gapi_structure structBase, c_long index)
{
    return c_structureMember(structBase, index);
}

gapi_type
gapi_metaData_memberType(gapi_member memberBase)
{
    return c_memberType(memberBase);
}

gapi_unsigned_long
gapi_metaData_memberOffset(gapi_member memberBase)
{
    return c_memberOffset(memberBase);
}

gapi_long
gapi_metaData_unionUnionCaseCount(gapi_union unionBase)
{
    return c_unionUnionCaseCount(unionBase);
}

gapi_unionCase
gapi_metaData_unionUnionCase(gapi_union unionBase, c_long index)
{
    return c_unionUnionCase(unionBase, index);
}

gapi_type
gapi_metaData_unionCaseType(gapi_unionCase caseBase)
{
    return c_unionCaseType(caseBase);
}
