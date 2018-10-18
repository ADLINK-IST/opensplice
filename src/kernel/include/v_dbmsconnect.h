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
#ifndef V_DBMSCONNECT_H
#define V_DBMSCONNECT_H

#include "kernelModule.h"
#include "v_participantQos.h"

#if defined (__cplusplus)
extern "C" {
#endif
#include "os_if.h"

#ifdef OSPL_BUILD_CORE
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

/**
 * \brief The <code>v_dbmsconnect</code> cast method.
 *
 * This method casts an object to a <code>v_dbmsconnect</code> object.
 * Before the cast is performed, if compiled with the NDEBUG flag not set,
 * the type of the object is checked to be <code>v_dbmsconnect</code> or
 * one of its subclasses.
 */
#define v_dbmsconnect(o) (C_CAST(o,v_dbmsconnect))

/**
 * \brief The <code>v_dbmsconnect</code> constructor.
 *
 * This method creates a record and replay service participant.
 *
 * Currently this class is part of the kernel but as being a pluggable service
 * This definition should be part of the service itself.
 *
 * \param manager        the kernel service manager
 * \param name           the participant name of the service
 * \param extStateName   the name by which the service state is identified.
 *                       the state will be made accessible via this name.
 * \param qos            the service participant QoS policy values.
 *
 * \return <code>NULL</code> if construction fails, otherwise
 *         a reference to a newly instantiated service object.
 */
OS_API v_dbmsconnect
v_dbmsconnectNew(
    v_kernel kernel,
    const c_char *name,
    const c_char *extStateName,
    v_participantQos qos,
    c_bool enable);

/**
 * \brief The <code>v_dbmsconnect</code> destructor.
 *
 * This method destroys the record and replay service participant.
 *
 * \param service  the service to be destroyed.
 *
 */
OS_API void
v_dbmsconnectFree(
    v_dbmsconnect service);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif /* V_DBMSCONNECT_H */
