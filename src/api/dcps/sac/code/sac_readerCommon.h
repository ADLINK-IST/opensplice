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
#ifndef DDS_READERCOMMON_H
#define DDS_READERCOMMON_H

#include "dds_dcps.h"
#include "sac_common.h"

/* If length is unlimited, but release is true, maxSamples equals the
 * maximum for the sequence (scdds2032). */
#ifdef OPENSPLICE_V7
#define REAL_MAX_SAMPLES(max_samples, info_seq) \
(((max_samples == DDS_LENGTH_UNLIMITED) && info_seq->_release) ? (DDS_long)(info_seq->_maximum) : max_samples)
#else
#define REAL_MAX_SAMPLES(max_samples, info_seq) \
(((max_samples == DDS_LENGTH_UNLIMITED) && info_seq->_release && info_seq->_maximum > 0) ? (DDS_long)(info_seq->_maximum) : max_samples)
#endif

struct flushCopyArg {
    _DataReader reader;
    _DDS_sequence data_seq;
    DDS_SampleInfoSeq *info_seq;
    os_uint32 seqIndex;
};

void
DDS_ReaderCommon_samples_flush_copy(
    void              *sample,
    cmn_sampleInfo     sampleInfo,
    void              *arg);

DDS_ReturnCode_t
DDS_ReaderCommon_check_read_args(
    _DDS_sequence data_seq,
    DDS_SampleInfoSeq *info_seq,
    const DDS_long max_samples);


#endif /* DDS_READERCOMMON_H */
