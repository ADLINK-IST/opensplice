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
#ifndef DDS_OPENSPLICE_STATUSUTILS_H
#define DDS_OPENSPLICE_STATUSUTILS_H

#include "ccpp.h"
#include "u_user.h"


/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */


namespace DDS {
namespace OpenSplice {
namespace Utils {


/*
 * Flag groups
 */
#define SAMPLE_STATE_FLAGS \
        (DDS::READ_SAMPLE_STATE | \
         DDS::NOT_READ_SAMPLE_STATE)

#define VIEW_STATE_FLAGS \
        (DDS::NEW_VIEW_STATE | \
         DDS::NOT_NEW_VIEW_STATE)

#define INSTANCE_STATE_FLAGS \
        (DDS::ALIVE_INSTANCE_STATE | \
         DDS::NOT_ALIVE_DISPOSED_INSTANCE_STATE | \
         DDS::NOT_ALIVE_NO_WRITERS_INSTANCE_STATE)

#define statesMask(sample_states, view_states, instance_states) \
        ((sample_states & SAMPLE_STATE_FLAGS) | \
         ((view_states & VIEW_STATE_FLAGS) << 2) | \
         ((instance_states & INSTANCE_STATE_FLAGS) << 4))

/*
 * Validations
 */
#define sampleStateMaskIsValid(mask) \
        ((DDS::Boolean)((mask == DDS::ANY_SAMPLE_STATE) || \
                       ((mask & ~SAMPLE_STATE_FLAGS) == 0)))

#define viewStateMaskIsValid(mask) \
        ((DDS::Boolean)((mask == DDS::ANY_VIEW_STATE) || \
                       ((mask & ~VIEW_STATE_FLAGS) == 0)))

#define instanceStateMaskIsValid(mask) \
        ((DDS::Boolean)((mask == DDS::ANY_INSTANCE_STATE) || \
                       ((mask & ~INSTANCE_STATE_FLAGS) == 0)))

#define statesMaskIsValid(sample_states, view_states, instance_states) \
        ((DDS::Boolean) (sampleStateMaskIsValid(sample_states)) && \
                        (viewStateMaskIsValid(view_states)) && \
                        (instanceStateMaskIsValid(instance_states)))

/* If length is unlimited, but release is true, maxSamples equals the
 * maximum for the sequence (scdds2032). */
#define realMaxSamples(max_samples, seq) \
    (((max_samples == DDS::LENGTH_UNLIMITED) && (seq.release())) ? seq.maximum() : max_samples)

/*
 * Status conversions
 */
DDS::StatusMask vEventMaskToStatusMask (const v_eventMask vMask, const v_kind vKind);
v_eventMask vEventMaskFromStatusMask (const DDS::StatusMask mask);

ReturnCode_t copyStatusOut( const v_incompatibleQosInfo   &from,  OfferedIncompatibleQosStatus   &to);
ReturnCode_t copyStatusOut( const v_incompatibleQosInfo   &from,  RequestedIncompatibleQosStatus &to);
ReturnCode_t copyStatusOut( const v_deadlineMissedInfo    &from,  RequestedDeadlineMissedStatus  &to);
ReturnCode_t copyStatusOut( const v_sampleRejectedInfo    &from,  SampleRejectedStatus           &to);
ReturnCode_t copyStatusOut( const v_livelinessChangedInfo &from,  LivelinessChangedStatus        &to);
ReturnCode_t copyStatusOut( const v_topicMatchInfo        &from,  SubscriptionMatchedStatus      &to);
ReturnCode_t copyStatusOut( const v_sampleLostInfo        &from,  SampleLostStatus               &to);
ReturnCode_t copyStatusOut( const v_inconsistentTopicInfo &from,  InconsistentTopicStatus        &to);
ReturnCode_t copyStatusOut( const v_deadlineMissedInfo    &from,  OfferedDeadlineMissedStatus    &to);
ReturnCode_t copyStatusOut( const v_livelinessLostInfo    &from,  LivelinessLostStatus           &to);
ReturnCode_t copyStatusOut( const v_topicMatchInfo        &from,  PublicationMatchedStatus       &to);
/* Not yet found in user layer or kernel (not properly searched yet anyway).
ReturnCode_t copyStatusOut(const gapi_allDataDisposedTopicStatus &from,     AllDataDisposedTopicStatus &to);
ReturnCode_t copyStatusOut(const gapi_sampleRejectedStatusKind &from,       SampleRejectedStatusKind &to);
*/



} /* end namespace Utils */
} /* end namespace OpenSplice */
} /* end namespace DDS */

#endif /* DDS_OPENSPLICE_STATUSUTILS_H */


