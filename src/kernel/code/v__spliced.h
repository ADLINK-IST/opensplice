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

#ifndef V__SPLICED_H
#define V__SPLICED_H

#if defined (__cplusplus)
extern "C" {
#endif

#include "v_spliced.h"

v_spliced
v_splicedNew (
    v_kernel kernel,
    c_bool enable);

void
v_splicedInit (
    v_spliced _this,
    c_bool enable);

void
v_splicedDeinit (
    v_spliced _this);

void
v_splicedHeartbeat (
    v_spliced _this);

void
v_splicedCheckHeartbeats (
    v_spliced _this);

c_bool
v_splicedPublicationMatchCount(
    v_spliced _this,
    v_object scope,
    v_gid wgid,
    c_ulong *count);

typedef v_result (*publicationInfoAction)(struct v_publicationInfo *, void *arg);

v_result
v_splicedLookupPublicationInfo(
    v_spliced _this,
    v_gid wgid,
    publicationInfoAction action,
    void *arg);

#if defined (__cplusplus)
}
#endif

#endif /* V__SPLICED_H */
