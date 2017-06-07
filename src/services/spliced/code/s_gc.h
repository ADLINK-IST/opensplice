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
#ifndef S_GARBAGECOLLECTOR_H
#define S_GARBAGECOLLECTOR_H

#if defined (__cplusplus)
extern "C" {
#endif

#include "s_types.h"

#define s_garbageCollector(o) ((s_garageCollector)o)

s_garbageCollector
s_garbageCollectorNew(
    spliced daemon);

void
s_garbageCollectorFree(
    s_garbageCollector gc);

void
s_garbageCollectorWaitForActive(
    s_garbageCollector gc);

#if defined (__cplusplus)
}
#endif

#endif /* S_GARBAGECOLLECTOR_H */
