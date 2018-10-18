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

#ifndef V__PARTITION_H
#define V__PARTITION_H

#include "v_partition.h"

#define v_partitionInterest(o) (C_CAST(o,v_partitionInterest))

c_bool
v_partitionExpressionIsAbsolute(
    const c_char* expression);

c_bool
v_partitionStringMatchesExpression(
    const c_char* string,
    const c_char* expression);

v_partitionInterest
v_partitionInterestNew (
    v_kernel kernel,
    const char *partitionExpression);

#endif
