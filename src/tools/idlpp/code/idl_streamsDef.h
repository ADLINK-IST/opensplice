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
