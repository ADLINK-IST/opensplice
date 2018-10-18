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
#ifndef MM_TRC_H
#define MM_TRC_H

#include "c_typebase.h"
#include "v_entity.h"

C_CLASS(monitor_trc);
#define monitor_trc(o)     ((monitor_trc)(o))

typedef enum {
    NO_ORDERING,
    ORDER_BY_COUNT, 
    ORDER_BY_SIZE, 
    ORDER_BY_TOTAL
} orderKind;

monitor_trc
monitor_trcNew (
    c_long objectCountLimit,
    const char *filterExpression,
    orderKind oKind,
    int orderCount,
    c_bool delta);

void
monitor_trcFree (
    monitor_trc o
    );

void
monitor_trcAction (
    v_public entity,
    c_voidp args
    );

#endif /* MM_TRC_H */
