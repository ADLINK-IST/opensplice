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

#ifndef D_MESSAGE_H
#define D_MESSAGE_H

#include "u_user.h"
#include "d__types.h"

#if defined (__cplusplus)
extern "C" {
#endif

/* Macro to check if the message uses time stamps beyond 2038.
 * The check consist of checking bit 30 of the nanoseconds field of the message */
#define IS_Y2038READY(d)                ((c_bool)(((d_message(d)->productionTimestamp.nanoseconds) & (1u << 30)) == (1u << 30)))

#define d_message(d)                    ((d_message)(d))

void                d_messageInit                 (d_message message,
                                                   d_admin admin);

void                d_messageDeinit               (d_message message);

void                d_messageSetAddressee         (d_message message,
                                                   d_networkAddress addressee);

d_networkAddress    d_messageGetAddressee         (d_message message);

void                d_messageSetSenderAddress     (d_message message,
                                                   d_networkAddress address);

c_bool              d_messageHasCapabilitySupport (d_message message);

#if defined (__cplusplus)
}
#endif

#endif /* D_MESSAGE_H */
