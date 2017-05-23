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
#ifndef SD__LIST_H
#define SD__LIST_H

#include "c_base.h"
#include "c_iterator.h"
#include "os_if.h"

#ifdef OSPL_BUILD_CORE
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

C_CLASS(sd_list);

typedef c_bool (*sd_listAction)(void *o, void *arg);
typedef c_bool (*sd_listCompare)(void *o, void *arg);

OS_API sd_list
sd_listNew (
    void);

OS_API void
sd_listFree (
    sd_list list);

OS_API c_bool
sd_listIsEmpty (
    sd_list list);

OS_API void
sd_listInsert (
    sd_list list,
    void *object);

OS_API void
sd_listAppend (
    sd_list list,
    void *object);

OS_API void *
sd_listReadFirst (
    sd_list list);

OS_API void *
sd_listReadLast (
    sd_list list);

OS_API void *
sd_listTakeFirst (
    sd_list list);

OS_API void *
sd_listTakeLast (
    sd_list list);

OS_API void *
sd_listRemove (
    sd_list list,
    void    *object);

OS_API void *
sd_listFindObject (
    sd_list list,
    void *object);

OS_API void *
sd_listFind (
    sd_list list,
    sd_listCompare compare,
    void *arg);

OS_API void
sd_listWalk (
    sd_list list,
    sd_listAction action,
    void *arg);

OS_API c_iter
sd_listIterator (
    sd_list list);

OS_API c_ulong
sd_listSize (
    sd_list list);

OS_API void *
sd_listAt (
    sd_list list,
    c_ulong index);

OS_API c_ulong
sd_listIndexOf (
    sd_list list,
    void *obj);

void
sd_listInsertAt(
    sd_list list,
    void *object,
    c_ulong index);

/*
 * This method inserts 'object' before 'beforeObject'.
 *
 * Precondition: 'beforeObject' is in the list.
 * Postcondition: 'object' is in the list, before 'beforeObject'.
 *
 * example:
 * L = {1, 4, 7, 3}
 *
 * insert(L, 100, 7)
 *
 * L = {1, 4, 100, 7, 3}
 *
 */
void
sd_listInsertBefore(
    sd_list list,
    void *object,
    void *before);

#undef OS_API

#endif /* SD__LIST_H */
