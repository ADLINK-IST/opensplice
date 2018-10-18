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

#ifndef C__COLLECTION_H
#define C__COLLECTION_H

#include "c_collection.h"

#if defined (__cplusplus)
extern "C" {
#endif

extern const size_t c_listSize;
extern const size_t c_setSize;
extern const size_t c_bagSize;
extern const size_t c_tableSize;
extern const size_t c_querySize;

c_array c_keyList          (c_table c);
void    c_collectionInit   (c_base base);
void    c_queryOptimize    (c_query _this);

#if defined (__cplusplus)
}
#endif

#endif /* C__COLLECTION_H */
