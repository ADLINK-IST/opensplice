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
#ifndef IDL_STACDEF_H
#define IDL_STACDEF_H

#include "c_metabase.h"
#include "c_iterator.h"

#include "idl_program.h"
#include "idl_scope.h"

C_CLASS(idl_stacDef);

C_CLASS(idl_stacMap);

C_STRUCT(idl_stacDef)
{
    c_iter stacList;
};

C_STRUCT(idl_stacMap)
{
    /* Type scope stack */
    c_metaObject scope;
    /* Type Name identifying the structure*/
    c_char *typeName;
    /* Defined stac list, i.e. the structure members */
    c_char *stacList;
};


idl_stacDef
idl_stacDefNew (
    void);

void
idl_stacDefFree (
    idl_stacDef stacDef);

void idl_stacDefAdd (
    idl_stacDef stacDef,
    c_metaObject scope,
    const char *typeName,
    const char *stacList);

c_bool
idl_isStacDefFor(
    c_metaObject scope,
    c_char *typeName,
    c_char *key);

void idl_stacDefDefSet (
    idl_stacDef stacDef);

idl_stacDef
idl_stacDefDefGet (
    void);

os_boolean
idl_stacDef_isStacDefined(
    idl_scope scope,
    const char *name,
    idl_typeSpec typeSpec,
    idl_typeSpec* baseStringTypeDereffered);

void
idl_stacDefRestoreAll(
    idl_stacDef stacDef,
    c_iter replaceInfos);

c_iter
idl_stacDefConvertAll(
    idl_stacDef stacDef);

#endif /* IDL_STACDEF_H */
