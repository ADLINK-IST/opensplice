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
/****************************************************************
 * Interface definition for OS layer user clock service         *
 ****************************************************************/

/** \file os_usrClock.h
 *  \brief User Clock management
 *
 * os_usrClock.h provides abstraction to user clock management functions.
 * It allows to initialize, deinitialize and specify the user clock module.
 * 
 * Calling os_userClockStart() and os_userClockStop() influences the behavior
 * of the os_time function os_timeGet().
 */

#ifndef OS_USRCLOCK_H
#define OS_USRCLOCK_H

#include "os_defs.h"

#include "os_if.h"

#ifdef OSPL_BUILD_OS
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

/** \brief Start a user clock
 *
 * Starts a user defined clock which will be used as a time base for
 * DCPS. The os_timeGet function will use that new time base.
 *
 * Precondition:
 * - userClockModule
 *   Path name of the user clock library, which is either a fully specified
 *   path (e.g. for UNIX and VxWorks), or only the file name which will be searched
 *   for in the library search path (e.g. UNIX).
 *   if NULL or an empty string, no user library will be loaded (e.g. for VxWorks
 *   when the user clock support is pre-loaded). On UNIX like platforms this will
 *   lead to failure of this function.
 * - startName
 *   Name of the start function.
 *   When specified as NULL, it is assumed that no start function is required.
 *   When specified as empty string, the default name clockStart is assumed.
 * - stopName
 *   Name of the stop function.
 *   When specified as NULL, it is assumed that no stop function is required.
 *   When specified as empty string, the default name clockStop is assumed.
 * - getName
 *   Name of the function to get the time.
 *   When specified as NULL, this operation will fail.
 *   When specified as empty string, the default name clockGet is assumed.
 *
 * Possible Results:
 * - returns os_resultSuccess if
 *     The user clock library is loaded when required,
 *     References to the required user clock functions are resolved,
 *     The user clock start function when specified is executed without returning an error status
 * - returns os_resultFail if
 *     The user clock library can not be loaded when required
 *     The required user clock functions can not be resolved.
 *     The user clock start function returned an error status.
 */
OS_API os_result
os_userClockStart(
    char *userClockModule,
    char *startName,
    char *stopName,
    char *getName);

/** \brief Start a user clock
 *
 * Stops the installed user defined clock which was used as a time base for
 * DCPS. The os_timeGet function will use its default time base again.
 *
 * Possible Results:
 * - returns os_resultSuccess if
 *     The installed user clock is successfully stopped.
 * - returns os_resultFail if
 *     The user clock stop function returned an error ststus.
 */
OS_API os_result
os_userClockStop(void);

#undef OS_API

#endif /* OS_USRCLOCK_H */
