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
#ifndef MM_ORC_H
#define MM_ORC_H

#include "c_typebase.h"
#include "v_entity.h"

C_CLASS(monitor_orc);
#define monitor_orc(o)     ((monitor_orc)(o))

monitor_orc
monitor_orcNew (
    c_long refCountLimit,
    const char *filterExpression,
    c_bool delta
    );

void
monitor_orcFree (
    monitor_orc o
    );

void
monitor_orcAction (
    v_public entity,
    c_voidp args
    );

#endif /* MM_ORC_H */
