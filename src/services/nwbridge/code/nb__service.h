/*
 *                         Vortex OpenSplice
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
 *
 *   Licensed under the ADLINK Software License Agreement Rev 2.7 2nd October
 *   2014 (the "License"); you may not use this file except in compliance with
 *   the License.
 *   You may obtain a copy of the License at:
 *                      $OSPL_HOME/LICENSE
 *
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */
#ifndef NB__SERVICE_H
#define NB__SERVICE_H

#include "nb_service.h"
#include "nb__object.h"

#include "NetworkingBridge.h"

#include "u_participant.h"

#if defined (__cplusplus)
extern "C" {
#endif

#ifndef NDEBUG
#define nb_service(o) (assert(nb__objectKind(nb_object(o)) == NB_OBJECT_SERVICE), (nb_service)(o))
#else
#define nb_service(o) ((nb_service)(o))
#endif

#define                 nb_serviceFree(s) nb_objectFree(s)

const os_char*          nb_serviceName(nb_service _this) __nonnull_all__
                                                         __attribute_returns_nonnull__;

u_service               nb_serviceService(nb_service _this) __nonnull_all__
                                                            __attribute_returns_nonnull__;

#define                 nb_serviceParticipant(_this) u_participant(nb_serviceService(_this))

/**
 * \brief This operation will change the state of the service AND write a new service status topic
 *
 * \param _this The nwbridge service object to operate on
 * \param exception This exception struct will be filled if an error occurs. Caller must check this struct by
 *                  performing a R_Exception_PROPAGATE directly after this call or by using the
 *                  r_exceptionIsRaised(...) functionality.
 * \param state The new state to apply
 */
c_bool                  nb__serviceChangeState(nb_service _this,
                                               ServiceState state) __nonnull_all__;

/* \brief Return a reference to the service participant (not a copy).
 *
 * This function returns a reference to the service participant. This reference may not
 * be freed. It also may not be used once the reference to the owning nwbridge service object
 * is released or once the service has been deinited (aka when r_objectIsAlive return OS_FALSE)
 *
 * \param _this The nwbridge service object to operate on'
 */
/* u_participant   nb_serviceParticipant(nb_service _this); */

#if defined (__cplusplus)
}
#endif

#endif /* NB__SERVICE_H */
