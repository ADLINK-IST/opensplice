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
#ifndef U_PUBLISHER_H
#define U_PUBLISHER_H

/** \file u_publisher.h
 * \brief The publisher class is responsible for data distribution.
 *
 * The data can be written to a publisher via associated typed u_writer objects.
 * A publisher will distribute the data to all partitions associated to the publisher.
 * The publisher also offers methods to control the distribution of data.
 * The distribution can be suspended or performed in a coherent manner.
 *
 * This class specifies object that implement a proxy to kernel publishers
 * and offers methods implementing the functionality as described above.
 */

/** Supported methods
 * u_publisher u_publisherNew           (u_participant p, const os_char *name, u_publisherQos qos);
 * u_result    u_publisherGetQos        (u_publisher _this, u_publisherQos *qos);
 * u_result    u_publisherSetQos        (u_publisher _this, u_publisherQos qos);
 * u_result    u_publisherSuspend       (u_publisher _this);
 * u_result    u_publisherResume        (u_publisher _this);
 * u_result    u_publisherCoherentBegin (u_publisher _this);
 * u_result    u_publisherCoherentEnd   (u_publisher _this);
*/

#include "u_types.h"

#if defined (__cplusplus)
extern "C" {
#endif

#ifdef OSPL_BUILD_CORE
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

/** \brief The u_publisher cast method.
 *
 * In comparison with kernel classes this cast method will not perform
 * runtime type checking due to the lack of type information.
 */
#define u_publisher(p) \
        ((u_publisher)u_objectCheckType(u_object(p), U_PUBLISHER))

/** \brief The class constructor.
 *
 * The constructor will create a kernel publisher and publisher proxy within
 * the scope of the specified participant.
 *
 * \param _this the participant that must be specified.
 * \param name an optional name that can be used to identify a publisher.
 * \param qos an optional parameter to specify publisher specific quality of
 *        service settings.
 * \return the created publisher proxy on a succesful operation or NULL
 *         if the operation failed.
 */
OS_API u_publisher
u_publisherNew (
    const u_participant _this,
    const os_char *name,
    const u_publisherQos qos,
    u_bool enable);

OS_API u_result
u_publisherGetQos (
    const u_publisher _this,
    u_publisherQos *qos);

OS_API u_result
u_publisherSetQos (
    const u_publisher _this,
    const u_publisherQos qos);

/** \brief Suspend data distribution until resume is called.
 *
 * This method will stop data distribution but processes can still write
 * data via the data writers until the publisher internal buffer has no
 * more space left to store messages.
 * Then the QoS setting determine whether writing data will block or
 * that old data will be overwritten.
 *
 * \param _this The publisher this method will operate on.
 * \return U_RESULT_OK
 */
OS_API u_result
u_publisherSuspend (
    const u_publisher _this);

/** \brief The resume method: resumes previously suspended data distribution.
 *
 * This method will resume data distribution.
 *
 * \param _this The publisher this method will operate on.
 * \return U_RESULT_OK
 */
OS_API u_result
u_publisherResume (
    const u_publisher _this);

/** \brief Set the beginning of a coherent set of data to be distributed.
 *
 * This method is used in combination with u_publisherCoherentEnd,
 * both specifying the beginning and the end of a coherent set of data
 * to be distributed.
 * Data written between these calls are written in a coherent manner
 * according to the coherency scope that is specified by the publishers QoS.
 *
 * \param _this The publisher this method will operate on.
 * \return U_RESULT_OK
 */
OS_API u_result
u_publisherCoherentBegin (
    const u_publisher _this);

/** \brief Set the end of a coherent set of data to be distributed.
 *
 * This method is used in combination with u_publisherCoherentBegin,
 * both specifying the beginning and the end of a coherent set of data
 * to be distributed.
 * Data written between these calls are written in a coherent manner
 * according to the coherency scope that is specified by the publishers QoS.
 *
 * \param _this The publisher this method will operate on.
 * \return U_RESULT_OK
 */
OS_API u_result
u_publisherCoherentEnd (
    const u_publisher _this);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif
