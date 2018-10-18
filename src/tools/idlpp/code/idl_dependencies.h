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
#ifndef IDL_DEPENDENCIES_H
#define IDL_DEPENDENCIES_H

#include "c_typebase.h"

C_CLASS(idl_dep);

idl_dep idl_depNew (void);

void idl_depFree (const idl_dep dep);

void idl_depDefInit (void);

idl_dep idl_depDefGet (void);

void idl_depDefExit (void);

void idl_depAdd (const idl_dep dep, const char *basename);

c_char *idl_depGet (const idl_dep dep, c_ulong index);

c_ulong idl_depLength (const idl_dep dep);

#endif /* IDL_DEPENDENCIES */
