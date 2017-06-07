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
#ifndef DDS_BUFFERREGISTRY_H
#define DDS_BUFFERREGISTRY_H

#include "dds_dcps.h"
#include "dds_dcps_private.h"

typedef struct dds_loanRegistry_s *dds_loanRegistry_t;

dds_loanRegistry_t
dds_loanRegistry_new (
    DDS_TypeSupport typeSupport);

void
dds_loanRegistry_free (
    dds_loanRegistry_t _this);

int
dds_loanRegistry_register (
    dds_loanRegistry_t _this,
    void **buffer,
    uint32_t length);

int
dds_loanRegistry_deregister (
    dds_loanRegistry_t _this,
    void **buffer,
    uint32_t length);

uint32_t
dds_loanRegistry_typeSize (
    dds_loanRegistry_t _this);

DDS_TypeSupportCopyInfo
dds_loanRegistry_copyInfo (
    dds_loanRegistry_t _this);

#endif
