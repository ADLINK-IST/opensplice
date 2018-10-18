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
#ifndef DDS_OBJMANAG_H
#define DDS_OBJMANAG_H

#include "dds_dcps.h"
#include "sac_common.h"


void *
DDS__header (
    void *object);

DDS_unsigned_long
DDS__refCount(
    void *object);

DDS_ReturnCode_t
DDS_keep (
    void *object);

DDS_ReturnCode_t
DDS__free (
    void *object);

DDS_char *
DDS_string_dup (
    const DDS_char *src);

DDS_char *
DDS_string_dup_no_spaces (
    const DDS_char *src);

DDS_octet *
DDS_octetSeq_allocbuf (
    DDS_unsigned_long len);

#endif
