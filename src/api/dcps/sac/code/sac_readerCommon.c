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

#include "dds_dcps.h"
#include "dds_dcps_private.h"
#include "sac_common.h"
#include "sac_loanRegistry.h"
#include "sac_genericCopyOut.h"
#include "sac_readerCommon.h"
#include "v_dataReaderSample.h"
#include "sac_report.h"

static void
_ReaderCommon_info_copy(cmn_sampleInfo srcInfo, DDS_SampleInfo *dst) {
    dst->sample_state                = (DDS_SampleStateKind)  srcInfo->sample_state;
    dst->view_state                  = (DDS_ViewStateKind)    srcInfo->view_state;
    dst->instance_state              = (DDS_InstanceStateKind)srcInfo->instance_state;
    dst->valid_data                  = (DDS_boolean)          srcInfo->valid_data;
    dst->instance_handle             = (DDS_InstanceHandle_t) srcInfo->instance_handle;
    dst->publication_handle          = (DDS_InstanceHandle_t) srcInfo->publication_handle;
    dst->disposed_generation_count   = (DDS_long)             srcInfo->disposed_generation_count;
    dst->no_writers_generation_count = (DDS_long)             srcInfo->no_writers_generation_count;
    dst->sample_rank                 = (DDS_long)             srcInfo->sample_rank;
    dst->generation_rank             = (DDS_long)             srcInfo->generation_rank;
    dst->absolute_generation_rank    = (DDS_long)             srcInfo->absolute_generation_rank;
    dst->source_timestamp.sec        = (DDS_long)             OS_TIMEW_GET_SECONDS(srcInfo->source_timestamp);
    dst->source_timestamp.nanosec    = (DDS_unsigned_long)    OS_TIMEW_GET_NANOSECONDS(srcInfo->source_timestamp);
    dst->reception_timestamp.sec     = (DDS_long)             OS_TIMEW_GET_SECONDS(srcInfo->reception_timestamp);
    dst->reception_timestamp.nanosec = (DDS_unsigned_long)    OS_TIMEW_GET_NANOSECONDS(srcInfo->reception_timestamp);
}

DDS_ReturnCode_t
DDS_ReaderCommon_check_read_args(
    _DDS_sequence data_seq,
    DDS_SampleInfoSeq *info_seq,
    const DDS_long max_samples)
{
    DDS_ReturnCode_t result = DDS_RETCODE_OK;

    if (!DDS_sequence_is_valid(data_seq)) {
        result = DDS_RETCODE_BAD_PARAMETER;
        SAC_REPORT(result, "Invalid sequence data_seq");
    }
    if (!DDS_sequence_is_valid((_DDS_sequence)info_seq)) {
        result = DDS_RETCODE_BAD_PARAMETER;
        SAC_REPORT(result, "Invalid sequence info_seq");
    }
    if (max_samples < -1) {
        result = DDS_RETCODE_BAD_PARAMETER;
        SAC_REPORT(result, "Invalid value for max_samples (%d)",
                    max_samples);
    }
    if (result != DDS_RETCODE_OK) {
        return result;
    }
    /* Rule 1 : Both sequences must have equal len,
     * max_len and owns properties.
     */
    if ( (data_seq->_length  != info_seq->_length)  ||
         (data_seq->_maximum != info_seq->_maximum) ||
         (data_seq->_release != info_seq->_release) )
    {
        result = DDS_RETCODE_PRECONDITION_NOT_MET;
        SAC_REPORT(result, "Mismatch between data_seq (%d,%d,%s) and info_seq (%d,%d,%s)",
                    data_seq->_length, data_seq->_maximum, data_seq->_release?"TRUE":"FALSE",
                    info_seq->_length, info_seq->_maximum, info_seq->_release?"TRUE":"FALSE");
    }
    /* Rule 4: When max_len > 0, then own must be true.
     */
    if ( (info_seq->_maximum > 0) &&
         (!info_seq->_release) )
    {
        result = DDS_RETCODE_PRECONDITION_NOT_MET;
        SAC_REPORT(result, "Invalid info_seq: _maximum = %d and _release = %s",
                    info_seq->_maximum, info_seq->_release?"TRUE":"FALSE");
    }
    /* Rule 5: when max_samples != LENGTH_UNLIMITED,
     * then the following condition needs to be met:
     * maxSamples <= max_len
     */
    if ( (info_seq->_maximum > 0) &&
         (((DDS_unsigned_long)max_samples) > info_seq->_maximum) &&
         (max_samples != DDS_LENGTH_UNLIMITED) )
    {
        result = DDS_RETCODE_PRECONDITION_NOT_MET;
        SAC_REPORT(result, "Invalid size info_seq: _maximum = %d and max_samples = %d",
                    info_seq->_maximum, max_samples);
    }
    if (result != DDS_RETCODE_OK) {
        return result;
    }
    /* In all other cases, the provided sequences are valid.
     */
    if (REAL_MAX_SAMPLES(max_samples, info_seq) == 0) {
        /* Optimization to avoid going into the kernel. */
        return DDS_RETCODE_NO_DATA;
    }
    return DDS_RETCODE_OK;
}


void
DDS_ReaderCommon_samples_flush_copy(
    void * sample,
    cmn_sampleInfo sampleInfo,
    void *arg)
{
    struct flushCopyArg *a = (struct flushCopyArg *)arg;
    _DataReader reader = a->reader;
    _DDS_sequence dataSeq = a->data_seq;
    DDS_SampleInfoSeq *infoSeq = a->info_seq;
    DDS_unsigned_long typeSize;
    C_STRUCT(DDS_dstInfo) dstInfo;
    void *dst;

    assert(infoSeq->_length == dataSeq->_length);

    typeSize = DDS_LoanRegistry_typeSize(reader->loanRegistry);
    dst = &((char*)dataSeq->_buffer)[a->seqIndex*typeSize];
    if (reader->copy_cache != NULL) {
        dstInfo.dst         = dst;
        dstInfo.buf         = dataSeq->_buffer;
        dstInfo.copyProgram = reader->copy_cache;
        reader->copy_out(sample, &dstInfo);
    } else {
        reader->copy_out(sample, dst);
    }

    _ReaderCommon_info_copy(sampleInfo, &(infoSeq->_buffer[a->seqIndex]));
    a->seqIndex++;
    dataSeq->_length = a->seqIndex;
    infoSeq->_length = a->seqIndex;
}
