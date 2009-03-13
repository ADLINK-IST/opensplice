
#ifndef D_MESSAGE_H
#define D_MESSAGE_H

#include "u_user.h"
#include "d__types.h"

#if defined (__cplusplus)
extern "C" {
#endif

#define d_message(d)                    ((d_message)(d))

void        d_messageInit               (d_message message,
                                         d_admin admin);

void        d_messageDeinit             (d_message message);

void        d_messageSetAddressee       (d_message message,
                                         d_networkAddress addressee);

void        d_messageSetSenderAddress   (d_message message,
                                         d_networkAddress address);

#if defined (__cplusplus)
}
#endif

#endif /* D_MESSAGE_H */
