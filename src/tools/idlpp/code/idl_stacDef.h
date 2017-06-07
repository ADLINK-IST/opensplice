/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR PrismTech
 *   Limited, its affiliated companies and licensors. All rights reserved.
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
