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
    const u_participant _this,
    const v_participant participant,
    const u_domain domain);

u_result
u__participantDeinitW (
    void *_this);

void
u__participantFreeW (
    void *_this);

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
    const u_participant _this);

void
u_participantIncUseCount(
    const u_participant _this);

void
u_participantDecUseCount(
    const u_participant _this);

#undef OS_API

#endif
