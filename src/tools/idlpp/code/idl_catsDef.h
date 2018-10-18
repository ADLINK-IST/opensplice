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
