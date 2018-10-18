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
#ifndef __V__NAMESPACE_H__
#define __V__NAMESPACE_H__

#include "c_typebase.h"
#include "c_iterator.h"
#include "v_kernel.h"

struct v_nameSpacePartitionTopic {
    char *partition;
    char *topic;
};

struct v_nameSpace {
    char *name;
    c_iter partitionTopics;
    /* policy */
};

c_iter /* v_nameSpace */
v__nameSpaceCollect (
    v_kernel kernel);

c_bool
v__nameSpaceIsIn (
    struct v_nameSpace *ns,
    char *partition,
    char *topic);

void
v__nameSpaceFree (
    struct v_nameSpace *ns);

#endif /* __V__NAMESPACE_H__ */
