/**
 * @fn c_bool v_indexWrite(v_index index, v_message message);
 * @brief Writes the given messages into the specified index.
 * 
 * @return True if the message is succesful inserted and otherwise False.
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

void
v_indexFree(
    v_index _this);

/* These two functions address the lifespanAdmin. This will have to be moved
 * to the dataReader */

void
v_indexRemoveExpiredSamples(
    v_index _this,
    v_readerSampleAction action,
    c_voidp arg);
                         
void
v_indexInsertExpiringSample(
    v_index _this,
    v_dataReaderSample sample);

c_type
v_indexType(
    v_index _this);

c_long
v_indexCount(
    v_index _this);

c_long
v_indexInstanceCount(
    v_index _this);

v_dataReaderInstance
v_indexLookupInstance(
    v_index _this,
    v_message keyTemplate);

void
v_indexRemoveInstance(
    v_index _this,
    v_dataReaderInstance instance);


c_bool
v_indexRead(
    v_index _this,
    v_readerSampleAction action,
    c_voidp arg);

c_bool
v_indexReadInstance(
    v_index _this,
    v_dataReaderInstance instance,
    v_readerSampleAction action,
    c_voidp arg);

c_bool
v_indexReadNextInstance(
    v_index _this,
    v_dataReaderInstance instance,
    v_readerSampleAction action,
    c_voidp arg);

c_bool
v_indexTake(
    v_index _this,
    v_readerSampleAction action,
    c_voidp arg);

c_bool
v_indexTakeInstance(
    v_index _this,
    v_dataReaderInstance instance,
    v_readerSampleAction action,
    c_voidp arg);

c_bool
v_indexTakeNextInstance(
    v_index _this,
    v_dataReaderInstance instance,
    v_readerSampleAction action,
    c_voidp arg);

c_bool
v_indexTakeSample(
    v_index _this,
    v_dataReaderInstance instance,
    v_dataReaderSample sample);

c_bool
v_indexQueryRead(
    v_index _this,
    v_dataReaderQuery drq,
    v_readerSampleAction action,
    c_voidp arg);

c_bool
v_indexQueryReadInstance(
    v_index _this,
    v_dataReaderQuery drq,
    v_dataReaderInstance instance,
    v_readerSampleAction action,
    c_voidp arg);
    
c_bool
v_indexQueryReadNextInstance(
    v_index _this,
    v_dataReaderQuery drq,
    v_dataReaderInstance instance,
    v_readerSampleAction action,
    c_voidp arg);

c_bool
v_indexQueryTake(
    v_index _this,
    v_dataReaderQuery drq,
    v_readerSampleAction action,
    c_voidp arg);

c_bool
v_indexQueryTakeInstance(
    v_index _this,
    v_dataReaderQuery drq,
    v_dataReaderInstance instance,
    v_readerSampleAction action,
    c_voidp arg);

c_bool
v_indexQueryTakeNextInstance(
    v_index _this,
    v_dataReaderQuery drq,
    v_dataReaderInstance instance,
    v_readerSampleAction action,
    c_voidp arg);

void
v_indexWalkSamples(
    v_index _this,
    v_readerSampleAction action,
    c_voidp arg);

void
v_indexUnregisterWriter(
    v_index _this,
    v_gid wGID);

#endif
