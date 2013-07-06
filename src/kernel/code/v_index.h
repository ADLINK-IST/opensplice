/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2013 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE 
 *
 *   for full copyright notice and license terms. 
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

typedef void (*v_indexNewAction)(v_index index, v_topic topic, c_voidp arg);

v_index
v_indexNew(
    v_dataReader reader,
    q_expr _from,
    v_indexNewAction action,
    c_voidp arg);

#endif
