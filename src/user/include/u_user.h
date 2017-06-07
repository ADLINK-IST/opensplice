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
#ifndef U_USER_H
#define U_USER_H

/** \file u_user.h
 *  \brief This class provides processes save access to kernel object.
 *
 * The User component provides processes access to kernel objects in a save
 * manner. A kernel object is a shared resource and processes can access and
 * create shared objects in kernel space. Processes must claim shared objects
 * from the kernel before accessing them to assure that they will not be
 * deleted or modified. Processes itself must assure not to terminate without
 * releasing all claimed objects.
 * The User component provides a participant interface to create sessions
 * between one or more kernels. In the User Layer each connected kernel is
 * represented as a Domain object. The User component implements a set of
 * additional interfaces to provides safe acces to all other Domain objects.
 *
 * This includes:
 * - automatic claiming and releasing shared objects for access.
 * - automatic releasing all claimes on process termination.
 * - protecting processes against termination during access of shared resources.
 * - providing an event notification mechanism on shared objects.
 *
 * This file specifies the user class interface. The user class implements
 * the singleton pattern. Before accessing the kernel processes need to
 * call the u_userInitialise method, this will create a process specific user
 * object. The user object provides the mechanism to handle process termination.
 * All other classes in the user component provide interfaces to access kernel
 * objects. These classes use the processes user object to do this in a save
 * manner.
 */

/* Supported methods:
 *
 * u_result u_userInitialise();
 */


#include "u_types.h"
#include "u_observable.h"

#include "u_partitionQos.h"
#include "u_participantQos.h"
#include "u_topicQos.h"
#include "u_writerQos.h"
#include "u_readerQos.h"
#include "u_publisherQos.h"
#include "u_subscriberQos.h"

#include "u_instanceHandle.h"

#include "u_domain.h"
#include "u_participant.h"
#include "u_partition.h"
#include "u_topic.h"

#include "u_publisher.h"
#include "u_writer.h"

#include "u_subscriber.h"
#include "u_reader.h"
#include "u_dataReader.h"
#include "u_query.h"

#include "u_waitset.h"

#include "u_builtin.h"
#include "u_service.h"
#include "u_spliced.h"

#include "u_cfNode.h"
#include "u_cfElement.h"
#include "u_cfAttribute.h"
#include "u_cfData.h"

#if defined (__cplusplus)
extern "C" {
#endif

#ifdef OSPL_BUILD_CORE
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

/** \brief The class constructor.
 *
 * This constructor creates a user object providing a save connections to kernels.
 * Processes must call this method to create a user objects to get save access
 * to any kernel.
 * The constructor is idem-potent so multiple calls will not have any effect,
 * however this call is not MT-save.
 */
OS_API u_result
u_userInitialise(void);

/** \brief Returns the Domain object corresponding to the given Domain Id.
 *
 * The Domain is only returned if the process is attached to the Domain
 * otherwise NULL is returned.
 */
OS_API u_domain
u_userLookupDomain(
    const u_domainId_t id);

/** \brief Returns the Domain Id from the first Domain that is found in
 *         the configuration file specified by the environment symbol OSPL_URI.
 *
 * This operation will return the default domain id (U_DOMAIN_ID_DEFAULT) in
 * case the environment variable OSPL_URI is not set or the provided
 * configuration file is invalid.
 */
OS_API u_domainId_t
u_userGetDomainIdFromEnvUri(void);

/** \brief Returns a generated process name that uniquely identifies the process.
 *
 */
OS_API os_char *
u_userGetProcessName(void);

/* Set the flag U_USER_DELETE_ENTITIES to clean-up all kernel entities. */
#define U_USER_DELETE_ENTITIES  ((os_uint32)(1 << 0))

/* Set the flag U_USER_BLOCK_OPERATIONS to block all calls going in or coming
 * out of the kernel. */
#define U_USER_BLOCK_OPERATIONS ((os_uint32)(1 << 1))

/* Set the flag U_USER_EXCEPTION in order to check the configuration parameter
 * InProcessExceptionHandling. If it is set, u_domainDetach will behave as if
 * U_USER_DELETE_ENTITIES is set. */
#define U_USER_EXCEPTION        ((os_uint32)(1 << 2))

/** \brief Detach the user layer from shared memory
 *
 */
OS_API u_result
u_userDetach(
    _In_ os_uint32 flags);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif
