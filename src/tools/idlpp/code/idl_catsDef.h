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
#ifndef IDL_CATSDEF_H
#define IDL_CATSDEF_H

#include "c_metabase.h"
#include "c_iterator.h"

#include "idl_program.h"
#include "idl_scope.h"

C_CLASS(idl_catsDef);

C_CLASS(idl_catsMap);

C_STRUCT(idl_catsDef)
{
    c_iter catsList;
};

C_STRUCT(idl_catsMap)
{
    /* Type scope stack */
    c_metaObject scope;
    /* Type Name identifying the structure*/
    c_char *typeName;
    /* Defined cats list, i.e. the structure members */
    c_char *catsList;
};


idl_catsDef
idl_catsDefNew (
    void);

void
idl_catsDefFree (
    idl_catsDef catsDef);

void idl_catsDefAdd (
    idl_catsDef catsDef,
    c_metaObject scope,
    const char *typeName,
    const char *catsList);

c_bool
idl_isCatsDefFor(
    c_metaObject scope,
    c_char *typeName,
    c_char *key);

void idl_catsDefDefSet (
    idl_catsDef catsDef);

idl_catsDef
idl_catsDefDefGet (
    void);

os_boolean
idl_catsDef_isCatsDefined(
    idl_scope scope,
    const char *name,
    idl_typeSpec typeSpec);

void
idl_catsDefRestoreAll(
    idl_catsDef catsDef,
    c_iter replaceInfos);

c_iter
idl_catsDefConvertAll(
    idl_catsDef catsDef);

#endif /* IDL_CATSDEF_H */
