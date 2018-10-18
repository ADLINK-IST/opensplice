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
#ifndef C_MODULE_H
#define C_MODULE_H

#if defined (__cplusplus)
extern "C" {
#endif

#include "c_metabase.h"
#include "c_sync.h"

C_CLASS(c_module);

C_STRUCT(c_module) {
    C_EXTENDS(c_metaObject);
    c_mutex mtx;
    c_scope scope;
};
C_ALIGNMENT_C_STRUCT_TYPE (c_module);

#ifndef NO_META_CAST
#define c_module(o)         (C_CAST(o,c_module))
#endif

#if defined (__cplusplus)
}
#endif

#endif
