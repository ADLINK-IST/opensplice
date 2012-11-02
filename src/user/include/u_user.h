/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2011 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE
 *
 *   for full copyright notice and license terms.
 *
 */
#ifndef U_USER_H
#define U_USER_H

/** \file u_user.h
 *  \brief This class provides processes save access to kernel object.
 *
 * The User component provides proceses access to kernel objects in a save
 * manner. A kernel object is a shared resource and processes can access and
 * create these shared objects. Processes must claim shared objects from a
 * kernel before accessing them to assure that the kernel will not delete or
 * modify the objects. Processes itself must assure not to terminate without
 * releasing all claimed objects.
 * The User component provides a set of classes to access kernel objects in
 * a save manner. The User component can handle multiple kernels via the
 * participant class. A participant object defines a kernel connection.
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
 * u_result u_userDetach();
 * u_result u_userProtect();
 * u_result u_userUnprotect();
 */


#if defined (__cplusplus)
extern "C" {
#endif

#include "u_types.h"
#include "u_time.h"

#include "u_partitionQos.h"
#include "u_participantQos.h"
#include "u_topicQos.h"
#include "u_writerQos.h"
#include "u_readerQos.h"
#include "u_publisherQos.h"
#include "u_subscriberQos.h"

#include "u_dispatcher.h"
#include "u_instanceHandle.h"

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

#include "u_service.h"
#include "u_spliced.h"
#include "u_serviceManager.h"

#include "u_cfNode.h"
#include "u_cfElement.h"
#include "u_cfAttribute.h"
#include "u_cfData.h"
#include "os_if.h"

#ifdef OSPL_BUILD_USER
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
u_userInitialise();

/** \brief The class destructor.
 *
 * The destructor disconnects all objects created in the scope of the given
 * user and the user object is deleted. All disconnected objects are not
 * deleted ,the process itself must delete these objects.
 * Calling methods of any disconnected object will result into a error.
 */
OS_API u_result
u_userDetach ();

/** \brief Increases the refCount of the object only within the lifespan of the process.
 */
OS_API c_object
u_userKeep(
    c_object o);

/** \brief Decreases the refCount of the object.
 */
OS_API void
u_userFree (
    c_object o);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif
