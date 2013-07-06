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

#ifndef D_MESSAGE_H
#define D_MESSAGE_H

#include "u_user.h"
#include "d__types.h"

#if defined (__cplusplus)
extern "C" {
#endif

#define d_message(d)                    ((d_message)(d))

void                d_messageInit               (d_message message,
                                                 d_admin admin);

void                d_messageDeinit             (d_message message);

void                d_messageSetAddressee       (d_message message,
                                                 d_networkAddress addressee);

d_networkAddress    d_messageGetAddressee       (d_message message);

void                d_messageSetSenderAddress   (d_message message,
                                                 d_networkAddress address);

#if defined (__cplusplus)
}
#endif

#endif /* D_MESSAGE_H */
