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
#ifndef IDL_GENCHELPER_H
#define IDL_GENCHELPER_H

#include "c_base.h"

#include "idl_scope.h"
#include "idl_program.h"

c_char *idl_cId(const char *identifier);

c_char *idl_scopeStackC(idl_scope scope, const char *scopeSepp, const char *name);

c_char *idl_corbaCTypeFromTypeSpec(idl_typeSpec typeSpec);

c_char *idl_genCLiteralValueImage(c_value literal, c_type type);

/* clean definitions */
void idl_definitionClean(void);

/* add a definition within the specified scope */
void idl_definitionAdd(char *class, char *name);

/* return 1 if the definition already exists within the specified scope
 * return 0 if the definition does not exists within the specified scope
 */
int idl_definitionExists(char *class, char *name);

c_char *idl_genCConstantGetter(void);

#endif /* IDL_GENCHELPER_H */
