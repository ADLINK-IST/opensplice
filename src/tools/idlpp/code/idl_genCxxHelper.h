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
#ifndef IDL_GENCXXHELPER_H
#define IDL_GENCXXHELPER_H

#include "c_typebase.h"

#include "idl_scope.h"
#include "idl_program.h"

c_char *idl_cxxId(const char *identifier);

c_char *idl_scopeStackCxx(idl_scope scope, const char *scopeSepp, const char *name);

c_char *idl_corbaCxxTypeFromTypeSpec(idl_typeSpec typeSpec);

c_char *idl_genCxxConstantGetter(void);

const c_char* idl_isocppCxxStructMemberSuffix(void);

#endif /* IDL_GENCXXHELPER_H */
