/*
 *                         Vortex OpenSplice
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
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
