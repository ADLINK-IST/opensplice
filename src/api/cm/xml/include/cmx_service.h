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
/**@file api/cm/xml/include/cmx_service.h
 * Represents a service in Splice in XML format.
 */
#ifndef CMX_SERVICE_H
#define CMX_SERVICE_H

#include "c_typebase.h"

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
 * @brief Provides access to the state of the supplied service.
 * 
 * The state looks like:
 @verbatim
    <entity>
            <pointer>...</pointer>
            <handle_index>...</handle_index>
            <handle_serial>..</handle_serial>
            <name>...</name>
            <kind>SERVICESTATE</kind>
            <enabled>...</enabled>
            <statename>...</statename>
            <state>...</state>
        </entity>
 @endverbatim
 * - 'statename' contains the name of the state
 * - 'state' contains the actual state of the service and is one of the
 *   following:
 *      -# NONE
 *      -# INITIALISING
 *      -# OPERATIONAL
 *      -# TERMINATING
 *      -# TERMINATED
 *      -# DIED
 * @param The service where to resolve the state from.
 * @return The state of the supplied service. If the service is not available 
 *         (anymore), NULL is returned.
 */
OS_API c_char*         cmx_serviceGetState     (const c_char* service);

/**
 * @brief Applied the supplied state on the supplied service.
 * 
 * @param service The service to apply the state on.
 * @param state The state that must be applied to the supplied servive.
 * @return Whether or not the state change succeeded. When succeeded:
 *         @verbatim<result>OK</result>@endverbatim is returned,
 *         @verbatim<result>FAILED</result>@endverbatim otherwise.
 * 
 * @todo Actual implementation of the state change.
 */
OS_API const c_char*   cmx_serviceSetState     (const c_char* service,
                                                const c_char* state);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif /* CMX_SERVICE_H */
