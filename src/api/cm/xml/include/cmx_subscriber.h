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
/**@file api/cm/xml/include/cmx_subscriber.h
 *
 * Represents a subscriber in Splice in XML format.
 */
#ifndef CMX_SUBSCRIBER_H
#define CMX_SUBSCRIBER_H

#include "c_typebase.h"
#include "u_types.h"

#if defined (__cplusplus)
extern "C" {
#endif
#include "os_if.h"

#ifdef OSPL_BUILD_CMXML
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

/**
 * Creates a new subscriber. It is creates by creating its user layer
 * counterpart and serializing it into XML format.
 *
 * An XML publisher looks like:
   @verbatim
   <entity>
       <pointer>...</pointer>
       <handle_index>...</handle_index>
       <handle_serial>..</handle_serial>
       <name>...</name>
       <kind>SUBSCRIBER</kind>
       <enabled>...</enabled>
   </entity>
   @endverbatim
 * @param participant The XML representation of the participant to attach the
 *                    subscriber to.
 * @param name The name for the subscriber.
 * @param qos The qos for the subscriber. If NULL is supplied, the default is
 *            taken.
 * @return The XML representation of the created subscriber or NULL if it could
 *         not be created.
 */
OS_API c_char*     cmx_subscriberNew           (const c_char* participant,
                                                const c_char* name,
                                                const c_char* qos);


OS_API const c_char*    cmx_subscriberBeginAccess   (const c_char* subscriber);

OS_API const c_char*    cmx_subscriberEndAccess     (const c_char* subscriber);


/**
 * @brief Provides access to the data-readers of the subscriber that have
 * data that matches the supplied mask
 *
 * The result of this function is a list of entities in XML format.
 @verbatim
    <list>
        <entity>
            <pointer>...</pointer>
            <handle_index>...</handle_index>
            <handle_serial>..</handle_serial>
            <name>...</name>
            <kind>...</kind>
            <enabled>...</enabled>
            ...
        </entity>
        <entity>
            ...
        </entity>
        ...
    </list>
 @endverbatim
 *
 * @param subscriber The subscriber, which datareaders must be resolved.
 * @param mask Specifies sample states, view states and instance states of the
 *             data in the datareaders that must match.
 * @return A XML list of datareaders that are owned by the supplied subscriber
 *         and match the supplied mask. If the supplied subscriber is not
 *         available (anymore), NULL will be returned.
 */
OS_API c_char*          cmx_subscriberGetDataReaders(const c_char* subscriber,
                                                     u_sampleMask mask);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif /* CMX_SUBSCRIBER*/
