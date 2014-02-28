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
#ifndef IDL_KEYDEF_H
#define IDL_KEYDEF_H

#include "c_metabase.h"
#include "c_typebase.h"
#include "c_iterator.h"
#include "idl_scope.h"

C_CLASS(idl_keyDef);

C_CLASS(idl_keyMap);

C_STRUCT(idl_keyDef) {
    c_iter keyList;
};

C_STRUCT(idl_keyMap) {
    c_metaObject scope;	/* Type scope stack */
    c_char *typeName;	/* Type Name */
    c_char *keyList;	/* Defined key list */
};

idl_keyDef idl_keyDefNew (void);

void idl_keyDefFree (idl_keyDef keyDef);

void idl_keyDefAdd (idl_keyDef keyDef, c_metaObject scope, const char *typeName, const char *keyList);

const c_char *idl_keyResolve (idl_keyDef keyDef, idl_scope scope, const char *typeName);
c_char * idl_keyResolve2 (idl_keyDef keyDef, c_metaObject scope, const char *typeName);

c_bool idl_keyDefIncludesType(idl_keyDef keyDef, const char *typeName);

void idl_keyDefDefSet (idl_keyDef keyDef);

idl_keyDef idl_keyDefDefGet (void);

#endif /* IDL_KEYDEF_H */
