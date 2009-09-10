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
#ifndef NW_SOCKET_H
#define NW_SOCKET_H

#include "os_time.h"

typedef struct nw_socket_s *nw_socket;
typedef os_uchar   sk_bool;
typedef os_ushort  sk_portNr;     /* Internally converted into n_port_t */
typedef os_uint32  sk_length;     /* Internally converted into size_t */
typedef os_uint32  sk_address;    /* Networking address */
typedef os_uint32  sk_partitionId; /* Networking partition */

typedef enum sk_addressType_e {
    SK_TYPE_UNKNOWN,
    SK_TYPE_LOOPBACK,
    SK_TYPE_BROADCAST,
    SK_TYPE_MULTICAST
} sk_addressType;


nw_socket   nw_socketSendNew(
                const char *defaultAddress,
                sk_portNr portNr,
                sk_bool supportsControl,
                const char *pathName);

nw_socket   nw_socketReceiveNew(
                const char *defaultAddress,
                sk_portNr portNr,
                sk_bool supportsControl,
                const char *pathName);

void        nw_socketFree(
                nw_socket sock);
                

sk_bool     nw_socketLoopsback(
                nw_socket sock);

                
sk_address  nw_socketPrimaryAddress(
                nw_socket sock);

sk_address  nw_socketBrodcastAddress(
                nw_socket sock);

sk_address  nw_socketDataAddress(
                nw_socket sock);

os_int      nw_socketPrimaryAddressCompare(
                nw_socket sock,
                sk_address toCompare);
                
                

void        nw_socketAddPartition(
                nw_socket sock,
                sk_partitionId partitionId,
                const char *addressString,
                sk_bool connected);

sk_length   nw_socketSendData(
                nw_socket sock,
                void *buffer,
                sk_length length);

sk_length   nw_socketSendDataTo(
                nw_socket sock,
                sk_address receiverAddress,
                void *buffer,
                sk_length length);

sk_length   nw_socketSendDataToPartition(
                nw_socket sock,
                sk_partitionId partitionId,
                void *buffer,
                sk_length length);
                

sk_length   nw_socketSendControlTo(
                nw_socket sock,
                sk_address receiverAddress,
                void *buffer,
                sk_length length);

sk_length   nw_socketReceive(
                nw_socket sock,
                sk_address *senderAddress,
                void *buffer,
                sk_length length,
                os_time *timeOut);
os_int      nw_socketBind(
                nw_socket sock);
                       
#endif /* NW_SOCKET_H */

