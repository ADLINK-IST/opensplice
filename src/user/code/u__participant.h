/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2009 PrismTech 
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE 
 *
 *   for full copyright notice and license terms. 
 *
 */
#ifndef U__PARTICIPANT_H
#define U__PARTICIPANT_H

#include "u_participant.h"
/* exporting some functions from this header file is only needed, since cmxml 
 * uses these functions 
 */
#include "os_if.h"

#ifdef OSPL_BUILD_USER
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

/** \brief The class initializer.
 *
 * The class initializer is called by the contructor of the class or a 
 * initializer of one of its derived classes.
 * It should not be used in any other
 * manner.
 *
 * \param _this The participant to operate on.
 * \return U_RESULT_OK on a succesful operation or<br>
 *         U_RESULT_ILL_PARAM if the specified participant is incorrect.
 */
u_result
u_participantInit(
    u_participant _this,
    u_kernel kernel);

/** \brief The class deinitializer.
 *
 * The class deinitializer is called by the destructor of the class or a 
 * deinitializer of one of its derived classes. It should not be used in any
 * other manner.
 *
 * \param _this The participant to operate on.
 * \return U_RESULT_OK on a succesful operation or<br>
 *         U_RESULT_ILL_PARAM if the specified participant is incorrect.
 */
u_result
u_participantDeinit (
    u_participant _this);

/** \brief Provides safe access to the kernel entity.
 *
 * This method protects the process from termination by increasing the
 * processes protect count and returns the shared memory reference of
 * the kernel entity associated to the given participant.
 * When access is no longer required the entity must be released by the
 * u_participantRelease method.
 * 
 * \param _this The participant to operate on.
 * \param p     The placeholder for the kernel participant reference.
 * \return U_RESULT_OK on a succesful operation or<br>
 *         U_RESULT_ILL_PARAM if the specified participant is incorrect.
 *         U_RESULT_INTERNAL_ERROR if the specified participant is has no
 *                                 associated kernel entity.
 */
u_result
u_participantClaim(
    u_participant _this,
    v_participant *p);

/** \brief Releases the previously claimed entity.
 *
 * This method decreases the processes protect count and unprotects the
 * process from termination when the couter reaches zero.
 * The previously retrieved reference to the kernel entity should not be
 * used anymore after this call since it may not have a valid value anymore.
 * 
 * \param _this The participant to operate on.
 * \return U_RESULT_OK on a succesful operation or<br>
 *         U_RESULT_ILL_PARAM if the specified participant is incorrect.
 */
u_result
u_participantRelease(
    u_participant _this);

/** \brief Retrieves the kernel associated to the given participants.
 *
 * \param _this The participant to operate on.
 * \return U_RESULT_OK on a succesful operation or
 *         U_RESULT_ILL_PARAM if the specified participant is incorrect.
 */
OS_API u_kernel
u_participantKernel(
    u_participant _this);

/** \brief Detaches the participant from the associated kernel
 *
 * This method will free the associated kernel participant and detach
 * from the associated kernel.
 * After this call the participant is of no longer use.
 *
 * \param _this The participant to operate on.
 * \return U_RESULT_OK on a succesful operation or
 *         U_RESULT_ILL_PARAM if the specified participant is incorrect.
 */
u_result
u_participantDetach(
    u_participant _this);

#undef OS_API

#endif
