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
#ifndef IDL_GENPYTHONHELPER_H
#define IDL_GENPYTHONHELPER_H

#include "c_typebase.h"
#include "idl_scope.h"

c_char *idl_pythonId(const char *identifier);
c_char *idl_scopeStackPython(idl_scope scope, const char *scopeSepp, const char *name);
c_char *idl_genPythonLiteralValueImage(c_value literal, c_type type);

#endif /* IDL_GENPYTHONHELPER_H */
