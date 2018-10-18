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

#ifndef DDS__TIME_H
#define DDS__TIME_H

DDS_Duration_t
dds_duration_to_sac(
    dds_duration_t d);

dds_duration_t
dds_duration_from_sac(
    DDS_Duration_t d);

DDS_Time_t
dds_time_to_sac(
    dds_time_t t);

dds_duration_t
dds_delta_from_now(
    dds_time_t n);

#endif /* DDS__TIME_H */
