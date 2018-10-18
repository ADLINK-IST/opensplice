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

#ifndef V__COLLECTION_H
#define V__COLLECTION_H

#if defined (__cplusplus)
extern "C" {
#endif

#include "v_collection.h"

void
v_collectionInit (
    _Inout_ v_collection _this,
    _In_opt_z_ const c_char *name);

void
v_collectionDeinit (
    v_collection _this);

#if defined (__cplusplus)
}
#endif

#endif /* V__COLLECTION_H */
