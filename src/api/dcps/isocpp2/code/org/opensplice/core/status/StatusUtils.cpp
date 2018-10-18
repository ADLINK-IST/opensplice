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
#include <org/opensplice/core/status/StatusUtils.hpp>


/*
 * Status conversions
 */
dds::core::status::StatusMask
org::opensplice::core::utils::vEventMaskToStatusMask (
    const v_eventMask vMask,
    const v_kind      vKind)
{
    dds::core::status::StatusMask mask;

    switch(vKind) {
    case K_TOPIC:
    case K_TOPIC_ADAPTER:
        if (vMask & V_EVENT_INCONSISTENT_TOPIC) {
            mask << dds::core::status::StatusMask::inconsistent_topic();
        }
        if (vMask & V_EVENT_ALL_DATA_DISPOSED) {
            mask << dds::core::status::StatusMask::all_data_disposed_topic();
        }
    break;
    case K_SUBSCRIBER:
        if (vMask & V_EVENT_ON_DATA_ON_READERS) {
            mask << dds::core::status::StatusMask::data_on_readers();
        }
    break;
    case K_WRITER:
        if (vMask & V_EVENT_LIVELINESS_LOST) {
            mask << dds::core::status::StatusMask::liveliness_lost();
        }
        if (vMask & V_EVENT_OFFERED_DEADLINE_MISSED) {
            mask << dds::core::status::StatusMask::offered_deadline_missed();
        }
        if (vMask & V_EVENT_OFFERED_INCOMPATIBLE_QOS) {
            mask << dds::core::status::StatusMask::offered_incompatible_qos();
        }
        if (vMask & V_EVENT_PUBLICATION_MATCHED) {
            mask << dds::core::status::StatusMask::publication_matched();
        }
    break;
    case K_READER:
    case K_GROUPQUEUE:
    case K_DATAREADER:
    case K_GROUPSTREAM:
        if (vMask & V_EVENT_SAMPLE_REJECTED) {
            mask << dds::core::status::StatusMask::sample_rejected();
        }
        if (vMask & V_EVENT_LIVELINESS_CHANGED) {
            mask << dds::core::status::StatusMask::liveliness_changed();
        }
        if (vMask & V_EVENT_REQUESTED_DEADLINE_MISSED) {
            mask << dds::core::status::StatusMask::requested_deadline_missed();
        }
        if (vMask & V_EVENT_REQUESTED_INCOMPATIBLE_QOS) {
            mask << dds::core::status::StatusMask::requested_incompatible_qos();
        }
        if (vMask & V_EVENT_SUBSCRIPTION_MATCHED) {
            mask << dds::core::status::StatusMask::subscription_matched();
        }
        if (vMask & V_EVENT_DATA_AVAILABLE) {
            mask << dds::core::status::StatusMask::data_available();
        }
        if (vMask & V_EVENT_SAMPLE_LOST) {
            mask << dds::core::status::StatusMask::sample_lost();
        }
    break;
    case K_PARTICIPANT:
    case K_PUBLISHER:
    case K_DOMAIN:
    case K_KERNEL:
    break;
    default:
        assert(FALSE);
    }

    return mask;
}

v_eventMask
org::opensplice::core::utils::vEventMaskFromStatusMask (
    const dds::core::status::StatusMask& mask)
{
    v_eventMask vMask = 0;
    if ((mask & dds::core::status::StatusMask::inconsistent_topic()).any()) {
        vMask |= V_EVENT_INCONSISTENT_TOPIC;
    }
    if ((mask & dds::core::status::StatusMask::liveliness_lost()).any()) {
        vMask |= V_EVENT_LIVELINESS_LOST;
    }
    if ((mask & dds::core::status::StatusMask::offered_deadline_missed()).any()) {
        vMask |= V_EVENT_OFFERED_DEADLINE_MISSED;
    }
    if ((mask & dds::core::status::StatusMask::offered_incompatible_qos()).any()) {
        vMask |= V_EVENT_OFFERED_INCOMPATIBLE_QOS;
    }
    if ((mask & dds::core::status::StatusMask::data_on_readers()).any()) {
        vMask |= V_EVENT_ON_DATA_ON_READERS;
    }
    if ((mask & dds::core::status::StatusMask::sample_lost()).any()) {
        vMask |= V_EVENT_SAMPLE_LOST;
    }
    if ((mask & dds::core::status::StatusMask::data_available()).any()) {
        vMask |= V_EVENT_DATA_AVAILABLE;
    }
    if ((mask & dds::core::status::StatusMask::sample_rejected()).any()) {
        vMask |= V_EVENT_SAMPLE_REJECTED;
    }
    if ((mask & dds::core::status::StatusMask::liveliness_changed()).any()) {
        vMask |= V_EVENT_LIVELINESS_CHANGED;
    }
    if ((mask & dds::core::status::StatusMask::requested_deadline_missed()).any()) {
        vMask |= V_EVENT_REQUESTED_DEADLINE_MISSED;
    }
    if ((mask & dds::core::status::StatusMask::requested_incompatible_qos()).any()) {
        vMask |= V_EVENT_REQUESTED_INCOMPATIBLE_QOS;
    }
    if ((mask & dds::core::status::StatusMask::publication_matched()).any()) {
        vMask |= V_EVENT_PUBLICATION_MATCHED;
    }
    if ((mask & dds::core::status::StatusMask::subscription_matched()).any()) {
        vMask |= V_EVENT_SUBSCRIPTION_MATCHED;
    }
    if ((mask & dds::core::status::StatusMask::all_data_disposed_topic()).any()) {
        vMask |= V_EVENT_ALL_DATA_DISPOSED;
    }
    return vMask;
}
