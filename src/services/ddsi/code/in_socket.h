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
#ifndef IN_SOCKET_H
#define IN_SOCKET_H

#include "os_time.h"
#include "in_locator.h"
#include "in__configChannel.h"

in_socket
in_socketDuplexNew(
    in_configChannel configChannel,
    os_boolean supportsControl);

/**  \brief constructor */
in_socket   in_socketSendNew(
				in_configChannel configChannel,
                os_boolean supportsControl);

/**  \brief constructor */
in_socket   in_socketReceiveNew(
				in_configChannel configChannel,
                os_boolean supportsControl);
/** narrower */
#define in_socket(_o) \
    ((in_socket)_o)

/** increment refcount */
#define in_socketKeep(_s) \
    in_socket(in_objectKeep(in_object(_s)))

/** */
void        in_socketFree(
                in_socket sock);

/** \brief */
os_boolean     in_socketSupportsControl(
				in_socket sock);
/** */
os_boolean     in_socketLoopsback(
                in_socket sock);

/** \brief return IPv4/IPv6 address format
 *
 * Use in_addressIsIPv4Compatible to differ between IPv4 and IPv6 */
in_address
in_socketPrimaryAddress(
                in_socket sock);

/** \brief return IPv4/IPv6 address format
 *
 * Use in_addressIsIPv4Compatible to differ between IPv4 and IPv6 */
in_address
in_socketMulticastAddress(
				in_socket sock);

/** \brief return IPv4/IPv6 address format
 *
 * Use in_addressIsIPv4Compatible to differ between IPv4 and IPv6 */
in_address
in_socketBrodcastAddress(
                in_socket sock);


/** \brief The data stream locator.
 *
 * \return NULL in case unicast has been disabled */
in_locator
in_socketGetUnicastDataLocator(
    in_socket sock);

/** \brief The data stream locator.
 *
 * \return NULL in case multicast has been disabled */
in_locator
in_socketGetMulticastDataLocator(
    in_socket sock);

/** \brief Support for control stream as optional feature
 *
 * NULL in case no support for control stream */
in_locator
in_socketGetUnicastControlLocator(
    in_socket sock);

/** \brief Support for control stream as optional feature
 *
 * NULL in case no support for control stream */
in_locator
in_socketGetMulticastControlLocator(in_socket sock);


in_long   in_socketSendDataTo(
                in_socket sock,
                in_locator receiver,
                void *buffer,
                os_size_t length);

in_long   in_socketSendControlTo(
				in_socket sock,
				in_locator receiver,
				void *buffer,
				os_size_t length);


in_long   in_socketReceive(
                in_socket sock,
                in_locator sendLocator,
                void *buffer,
                os_size_t length,
                os_boolean *isControl,
                const os_time *timeOut);


#endif /* IN_SOCKET_H */

