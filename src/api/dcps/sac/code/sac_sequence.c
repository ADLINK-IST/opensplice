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

#include <string.h>

#include "dds_dcps.h"
#include "dds_dcps_private.h"
#include "sac_common.h"
#include "sac_objManag.h"


_DDS_sequence
DDS_sequence_malloc (
    void)
{
    return DDS_alloc(sizeof(struct DDS_sequence_s), (DDS_deallocatorType) DDS_sequence_free);
}

DDS_ReturnCode_t
DDS_sequence_free (
    _DDS_sequence sequence)
{
    if (sequence != NULL) {
        if (sequence->_release) {
            DDS_free(sequence->_buffer);
        }
    }
    return DDS_RETCODE_OK;
}

void
DDS_sequence_clean (
    _DDS_sequence sequence)
{
    if (sequence != NULL) {
        if (sequence->_release) {
            DDS_free (sequence->_buffer);
        }
        sequence->_buffer = NULL;
        sequence->_maximum = 0;
        sequence->_length = 0;
        sequence->_release = FALSE;
    }
}

void *
DDS_sequence_allocbuf (
    DDS_deallocatorType dealloactor,
    DDS_unsigned_long len,
    DDS_unsigned_long count)
{
    void *buffer;
    DDS_unsigned_long *bufcount;

    if (count > 0) {
        buffer = DDS__malloc (dealloactor, sizeof(DDS_unsigned_long), len * count);
        bufcount = DDS__header (buffer);
        *bufcount = count;
    } else {
        buffer = NULL;
    }

    return buffer;
}

void
DDS_sequence_replacebuf (
    _DDS_sequence sequence,
    void *(*allocbuf)(DDS_unsigned_long len),
    DDS_unsigned_long count)
{
    if (count > sequence->_maximum) {
        DDS_sequence_clean(sequence);
    }
    if (sequence->_buffer == NULL) {
        sequence->_buffer = allocbuf(count);
        sequence->_maximum = count;
        sequence->_length = 0;
        sequence->_release = TRUE;
    }
}

void
DDS_sequence_set_release (
    void *sequence,
    DDS_boolean release)
{
    _DDS_sequence seq = (_DDS_sequence)sequence;

    seq->_release = release;
}

DDS_boolean
DDS_sequence_get_release (
    void *sequence)
{
    _DDS_sequence seq = (_DDS_sequence)sequence;

    return seq->_release;
}

DDS_sequence_octet *
DDS_sequence_octet__alloc (
    void)
{
   return (DDS_sequence_octet *)DDS_sequence_malloc();
}

DDS_octet *
DDS_sequence_octet_allocbuf (
    DDS_unsigned_long len)
{
    return (DDS_octet *)
        DDS_sequence_allocbuf (NULL, sizeof(DDS_octet), len);
}

DDS_sequence_DDS_InstanceHandle_t *
DDS_sequence_DDS_InstanceHandle_t__alloc (
    void)
{
    return (DDS_sequence_DDS_InstanceHandle_t *)DDS_sequence_malloc ();
}

DDS_InstanceHandle_t *
DDS_sequence_DDS_InstanceHandle_t_allocbuf (
    DDS_unsigned_long len)
{
    return (DDS_InstanceHandle_t *)
        DDS_sequence_allocbuf (NULL, sizeof(DDS_InstanceHandle_t), len);
}

static DDS_ReturnCode_t
DDS_sequence_string_freebuf (void *buffer)
{
    DDS_unsigned_long *count = (DDS_unsigned_long *)DDS__header (buffer);
    DDS_string *b = (DDS_string *)buffer;
    DDS_unsigned_long i;
    for (i = 0; i < *count; i++) {
        DDS_string_clean (&b[i]);
    }
    return DDS_RETCODE_OK;
}

DDS_sequence_string *
DDS_sequence_string__alloc (void)
{
    return (DDS_sequence_string *)DDS_sequence_malloc();
}

DDS_string *
DDS_sequence_string_allocbuf (DDS_unsigned_long len)
{
    return (DDS_string *)
        DDS_sequence_allocbuf((DDS_deallocatorType) DDS_sequence_string_freebuf,
                              sizeof(DDS_string), len);
}

DDS_sequence_DDS_QosPolicyCount *
DDS_sequence_DDS_QosPolicyCount__alloc (
    void)
{
    return (DDS_sequence_DDS_QosPolicyCount *)DDS_sequence_malloc();
}

DDS_QosPolicyCount *
DDS_sequence_DDS_QosPolicyCount_allocbuf (
    DDS_unsigned_long len)
{
    return (DDS_QosPolicyCount *)
        DDS_sequence_allocbuf(NULL, sizeof(DDS_Topic), len);
}

DDS_sequence_DDS_Topic *
DDS_sequence_DDS_Topic__alloc (
    void)
{
    return (DDS_sequence_DDS_Topic *)DDS_sequence_malloc();
}

DDS_Topic *
DDS_sequence_DDS_Topic_allocbuf (
    DDS_unsigned_long len)
{
    return (DDS_Topic *)
        DDS_sequence_allocbuf(NULL, sizeof(DDS_Topic), len);
}

DDS_sequence_DDS_Condition *
DDS_sequence_DDS_Condition__alloc (
    void)
{
    return (DDS_sequence_DDS_Condition *)DDS_sequence_malloc();
}

DDS_Condition *
DDS_sequence_DDS_Condition_allocbuf (
    DDS_unsigned_long len)
{
    return (DDS_Condition *)
        DDS_sequence_allocbuf(NULL, sizeof(DDS_Condition), len);
}

DDS_sequence_DDS_DataReader *
DDS_sequence_DDS_DataReader__alloc (
    void)
{
    return (DDS_sequence_DDS_DataReader *)DDS_sequence_malloc();
}

DDS_DataReader *
DDS_sequence_DDS_DataReader_allocbuf (
    DDS_unsigned_long len)
{
    return (DDS_DataReader *)
        DDS_sequence_allocbuf(NULL, sizeof(DDS_DataReader), len);
}

DDS_sequence_DDS_SampleStateKind *
DDS_sequence_DDS_SampleStateKind__alloc (
    void)
{
    return (DDS_sequence_DDS_SampleStateKind *)DDS_sequence_malloc();
}

DDS_SampleStateKind *
DDS_sequence_DDS_SampleStateKind_allocbuf (
    DDS_unsigned_long len)
{
    return (DDS_SampleStateKind *)
        DDS_sequence_allocbuf(NULL, sizeof(DDS_SampleStateKind), len);
}

DDS_sequence_DDS_ViewStateKind *
DDS_sequence_DDS_ViewStateKind__alloc (
    void)
{
    return (DDS_sequence_DDS_ViewStateKind *)DDS_sequence_malloc();
}

DDS_ViewStateKind *
DDS_sequence_DDS_ViewStateKind_allocbuf (
    DDS_unsigned_long len)
{
    return (DDS_ViewStateKind *)
        DDS_sequence_allocbuf(NULL, sizeof(DDS_ViewStateKind), len);
}

DDS_sequence_DDS_InstanceStateKind *
DDS_sequence_DDS_InstanceStateKind__alloc (
    void)
{
    return (DDS_sequence_DDS_InstanceStateKind *)DDS_sequence_malloc();
}

DDS_InstanceStateKind *
DDS_sequence_DDS_InstanceStateKind_allocbuf (
    DDS_unsigned_long len)
{
    return (DDS_InstanceStateKind *)
        DDS_sequence_allocbuf(NULL, sizeof(DDS_InstanceStateKind), len);
}

DDS_sequence_DDS_SampleInfo *
DDS_sequence_DDS_SampleInfo__alloc (
    void)
{
    return (DDS_sequence_DDS_SampleInfo *)DDS_sequence_malloc();
}

DDS_SampleInfo *
DDS_sequence_DDS_SampleInfo_allocbuf (
    DDS_unsigned_long len)
{
    return( DDS_SampleInfo *)
        DDS_sequence_allocbuf(NULL, sizeof(DDS_SampleInfo), len);
}

/*****************************************************************/

DDS_octet *
DDS_octetSeq_allocbuf (
    DDS_unsigned_long len)
{
    return (DDS_octet *)
        DDS_sequence_allocbuf (NULL, sizeof(DDS_octet), len);
}

DDS_InstanceHandleSeq *
DDS_InstanceHandleSeq__alloc (
    void)
{
    return (DDS_InstanceHandleSeq *)DDS_sequence_malloc ();
}

DDS_InstanceHandle_t *
DDS_InstanceHandleSeq_allocbuf (
    DDS_unsigned_long len)
{
    return (DDS_InstanceHandle_t *)
        DDS_sequence_allocbuf(NULL, sizeof(DDS_InstanceHandle_t), len);
}

DDS_QosPolicyCountSeq *
DDS_QosPolicyCountSeq__alloc (
    void)
{
    return (DDS_QosPolicyCountSeq *)DDS_sequence_malloc();
}

DDS_QosPolicyCount *
DDS_QosPolicyCountSeq_allocbuf (
    DDS_unsigned_long len)
{
    return (DDS_QosPolicyCount *)
        DDS_sequence_allocbuf(NULL, sizeof(DDS_QosPolicyCount), len);
}

DDS_TopicSeq *
DDS_TopicSeq__alloc (
    void)
{
    return (DDS_TopicSeq *)DDS_sequence_malloc();
}

DDS_Topic *
DDS_TopicSeq_allocbuf (
    DDS_unsigned_long len)
{
    return (DDS_Topic *)
        DDS_sequence_allocbuf(NULL, sizeof(DDS_Topic), len);
}

DDS_DataReaderSeq *
DDS_DataReaderSeq__alloc (
    void)
{
    return (DDS_DataReaderSeq *)DDS_sequence_malloc();
}

DDS_DataReader *
DDS_DataReaderSeq_allocbuf (
    DDS_unsigned_long len)
{
    return (DDS_DataReader *)
        DDS_sequence_allocbuf(NULL, sizeof(DDS_DataReader), len);
}

DDS_ConditionSeq *
DDS_ConditionSeq__alloc (
    void)
{
    return (DDS_ConditionSeq *)DDS_sequence_malloc();
}

DDS_Condition *
DDS_ConditionSeq_allocbuf (
    DDS_unsigned_long len)
{
    return (DDS_Condition *)
        DDS_sequence_allocbuf(NULL, sizeof(DDS_Condition), len);
}

DDS_SampleStateSeq *
DDS_SampleStateSeq__alloc (
    void)
{
    return (DDS_SampleStateSeq *)DDS_sequence_malloc();
}

DDS_SampleStateKind *
DDS_SampleStateSeq_allocbuf (
    DDS_unsigned_long len)
{
    return (DDS_SampleStateKind *)
        DDS_sequence_allocbuf(NULL, sizeof(DDS_SampleStateKind), len);
}

DDS_ViewStateSeq *
DDS_ViewStateSeq__alloc (
    void)
{
    return (DDS_ViewStateSeq *)DDS_sequence_malloc();
}

DDS_ViewStateKind *
DDS_ViewStateSeq_allocbuf (
    DDS_unsigned_long len)
{
    return (DDS_ViewStateKind *)
        DDS_sequence_allocbuf(NULL, sizeof(DDS_ViewStateKind), len);
}

DDS_InstanceStateSeq *
DDS_InstanceStateSeq__alloc (
    void)
{
    return (DDS_InstanceStateSeq *)DDS_sequence_malloc();
}

DDS_InstanceStateKind *
DDS_InstanceStateSeq_allocbuf (
    DDS_unsigned_long len)
{
    return (DDS_InstanceStateKind *)
        DDS_sequence_allocbuf(NULL, sizeof(DDS_InstanceStateKind), len);
}

DDS_SampleInfoSeq *
DDS_SampleInfoSeq__alloc (
    void)
{
    return (DDS_SampleInfoSeq *)DDS_sequence_malloc();
}

DDS_SampleInfo *
DDS_SampleInfoSeq_allocbuf (
    DDS_unsigned_long len)
{
    return (DDS_SampleInfo *)
        DDS_sequence_allocbuf(NULL, sizeof(DDS_SampleInfo), len);
}
