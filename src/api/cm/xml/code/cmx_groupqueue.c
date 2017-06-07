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
#include "cmx__groupqueue.h"
#include "v_groupQueue.h"
#include <stdio.h>
#include "os_stdlib.h"

c_char*
cmx_groupQueueInit(
    v_groupQueue entity)
{
    assert(C_TYPECHECK(entity, v_groupQueue));
    OS_UNUSED_ARG(entity);
    
    return (c_char*)(os_strdup("<kind>GROUPQUEUE</kind>"));
}
