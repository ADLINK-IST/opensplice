/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2013 PrismTech
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

#ifdef OSPL_BUILD_CORE
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
    u_domain domain);

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

u_result
u_participantAddPublisher (
    u_participant _this,
    u_publisher publisher);

u_result
u_participantRemovePublisher (
    u_participant _this,
    u_publisher publisher);

u_result
u_participantAddSubscriber (
    u_participant _this,
    u_subscriber subscriber);

u_result
u_participantRemoveSubscriber (
    u_participant _this,
    u_subscriber subscriber);

u_result
u_participantAddTopic (
    u_participant _this,
    u_topic topic);

u_result
u_participantRemoveTopic (
    u_participant _this,
    u_topic topic);

#undef OS_API

#endif
