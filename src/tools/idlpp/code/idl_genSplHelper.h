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
#ifndef IDL_GENSPLHELPER_H
#define IDL_GENSPLHELPER_H

#include "c_typebase.h"

#include "idl_program.h"

enum idl_boundsCheckFailKind {
    CASE,
    DISCRIMINATOR,
    MEMBER,
    ELEMENT
};

void idl_boundsCheckFail (enum idl_boundsCheckFailKind kind, idl_scope scope, idl_typeSpec type, const c_char* name);

void idl_boundsCheckFailNull (enum idl_boundsCheckFailKind kind, idl_scope scope, idl_typeSpec type, const c_char* name);

void idl_memoryAllocFailed (idl_scope scope, idl_typeSpec type, const c_char* name, c_long indent);

void idl_printIndent (c_long indent);

c_char *idl_typeFromTypeSpec (const idl_typeSpec typeSpec);

c_char *idl_scopedTypeName (const idl_typeSpec typeSpec);

c_char *idl_scopedSplTypeName (const idl_typeSpec typeSpec);

c_char *idl_scopedTypeIdent (const idl_typeSpec typeSpec);

c_char *idl_scopedSplTypeIdent (const idl_typeSpec typeSpec);

#endif /* IDL_GENSPLHELPER_H */
