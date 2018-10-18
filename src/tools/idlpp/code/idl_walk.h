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
#ifndef IDL_WALK_H
#define IDL_WALK_H

#include "c_base.h"
#include "c_typebase.h"
#include "c_iterator.h"

#include "idl_program.h"
#include "idl_tmplExp.h"

void
idl_walkPresetFile(
    const char *fileName);

void
idl_walkPresetModule(
    const char *moduleName);

void
idl_walk(
    c_base base,
    const char *fileName,
    c_iter objects,
    c_bool traceWalk,
    idl_program program);

idl_typeSpec
idl_makeTypeCollection(
    c_collectionType type);

idl_typeSpec
idl_makeTypeSpec(
    c_type type);

void
idl_freeTypeSpec(
    idl_typeSpec typeSpec);

idl_scope
idl_buildScope (
    c_metaObject o);

#endif /* IDL_WALK_H */
