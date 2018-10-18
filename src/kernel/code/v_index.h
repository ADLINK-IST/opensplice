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
#ifndef V_INDEX_H
#define V_INDEX_H

#include "v_kernel.h"
#include "v__dataReader.h"
#include "v_dataReaderSample.h"

#define v_index(o) (C_CAST(o,v_index))

#define v_indexKeyList(_this) \
        c_tableKeyList(v_index(_this)->objects)

#define v_indexSourceKeyList(_this) \
        (c_keep(v_index(_this)->sourceKeyList))

#define v_indexMessageKeyList(_this) \
        (c_keep(v_index(_this)->messageKeyList))

#define v_indexTopic(o) \
         (v_index(o)->entry ? \
          v_dataReaderEntryTopic(v_index(o)->entry) : NULL )

#define v_indexDataReader(_this) \
        v_dataReader(v_index(_this)->reader)

typedef c_bool (*v_indexNewAction)(v_index index, v_topic topic, c_voidp arg);

v_index
v_indexNew(
    v_dataReader reader,
    q_expr _from,
    v_indexNewAction action,
    c_voidp arg);

#endif
