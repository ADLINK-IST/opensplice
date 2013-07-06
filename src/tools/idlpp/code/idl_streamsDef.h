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
#ifndef IDL_STREAMSDEF_H
#define IDL_STREAMSDEF_H

#include "c_metabase.h"
#include "c_typebase.h"
#include "c_iterator.h"
#include "idl_scope.h"

C_CLASS(idl_streamsDef);

C_CLASS(idl_streamsMap);

C_STRUCT(idl_streamsDef)
{
    c_iter streamsList;
};

C_STRUCT(idl_streamsMap)
{
    /* Type scope stack */
    c_metaObject scope;
    /* Type Name identifying the structure */
    c_char *typeName;
};

idl_streamsDef
idl_streamsDefNew(
    void);

void
idl_streamsDefDefSet(
    idl_streamsDef streamsDef);

idl_streamsDef
idl_streamsDefDefGet(
    void);

void
idl_streamsDefAdd(
    idl_streamsDef streamsDef,
    c_metaObject scope,
    const char *typeName);

void
idl_streamsDefFree(
    idl_streamsDef streamsDef);

c_bool
idl_streamsResolve(
    idl_streamsDef streamsDef,
    idl_scope scope,
    const char *typeName);

#endif /* IDL_STREAMSDEF_H */
