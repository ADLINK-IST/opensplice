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
#ifndef SAC_LOANREGISTRY_H
#define SAC_LOANREGISTRY_H

#include "dds_dcps.h"
#include "sac_common.h"

DDS_LoanRegistry
DDS_LoanRegistry_new (
    DDS_TypeSupport typeSupport);

DDS_ReturnCode_t
DDS_LoanRegistry_free (
    DDS_LoanRegistry _this);

DDS_ReturnCode_t
DDS_LoanRegistry_register (
    DDS_LoanRegistry _this,
    _DDS_sequence data_seq,
    DDS_SampleInfoSeq *info_seq,
    DDS_long length);

DDS_ReturnCode_t
DDS_LoanRegistry_deregister (
    DDS_LoanRegistry _this,
    _DDS_sequence data_seq,
    DDS_SampleInfoSeq *info_seq);

DDS_boolean
DDS_LoanRegistry_is_loan (
    DDS_LoanRegistry _this,
    void *data_buffer,
    void *info_buffer);

DDS_boolean
DDS_LoanRegistry_is_empty (
    DDS_LoanRegistry _this);

DDS_unsigned_long 
DDS_LoanRegistry_typeSize (
    DDS_LoanRegistry _this);

#endif
