#ifndef IN_COMMONTYPES_H
#define IN_COMMONTYPES_H

#include "c_metabase.h"    /* For c_type */
#include "os_classbase.h"
#include "os_defs.h"
#include "os_if.h"

/**
 * Allow usage of this C code from C++ code.
 */
#if defined (__cplusplus)
extern "C" {
#endif

/* platform specific 'int', may be 32bit or 64 bit */
typedef unsigned int in_uint;
typedef          int in_int;
typedef unsigned int in_partitionId;
typedef os_uchar     in_octet;
typedef long         in_long;
typedef unsigned long in_ulong;

/** os_size_t matching native pointers on that platform */
#define P2UI(_p) ((os_size_t)(_p))
#define UI2P(_u) ((in_octet*)(_u))
#define UI(_i)   ((os_size_t)(_i))
#define UI_DEREF(ptr) (*(os_size_t *)(ptr))
#define OCT(_u) ((_u)&0xff)
#define UI2ENTITYID(_u) { { OCT((_u)>>24), OCT((_u)>>16), OCT((_u)>>8)}, OCT((_u)&0xff) }

#define IN_USHORT_MAX (UINT16_MAX)

#define IN_SHORT_MAX  (INT16_MAX)
#define IN_SHORT_MIN  (INT16_MIN)

#define IN_UINT32_MAX (UINT32_MAX)

#define IN_INT32_MAX  (INT32_MAX)
#define IN_INT32_MIN  (INT32_MIN)

typedef void (*in_onFatalCallBack)(c_voidp usrData);

#define IN_STRING_TERMINATOR ((char)'\0')

OS_CLASS(in_channel);
OS_CLASS(in_channelData);
OS_CLASS(in_channelDataReader);
OS_CLASS(in_channelDataWriter);
OS_CLASS(in_channelReader);
OS_CLASS(in_channelSdp);
OS_CLASS(in_channelSdpReader);
OS_CLASS(in_channelSdpWriter);
OS_CLASS(in_channelWriter);
OS_CLASS(in_plugKernel);

OS_CLASS(in_stream);
OS_CLASS(in_streamPair);
OS_CLASS(in_streamPairIBasic);
OS_CLASS(in_streamReaderCallbackTable);
OS_CLASS(in_streamWriter);
OS_CLASS(in_streamReader);

OS_CLASS(in_transport);
OS_CLASS(in_transportPair);
OS_CLASS(in_transportPairIBasic);
OS_CLASS(in_transportReceiver);
OS_CLASS(in_transportReceiverIBasic); /**  \brief implements abstract interface in_transprotReceiver */
OS_CLASS(in_transportSender);
OS_CLASS(in_transportSenderIBasic);

OS_CLASS(in_abstractSendBuffer);
OS_CLASS(in_abstractReceiveBuffer);
OS_CLASS(in_receiveBuffer); /* implements in_abstractReceiveBuffer */
OS_CLASS(in_sendBuffer); /* implements in_abstractSendBuffer */

OS_CLASS(in_socket);
OS_CLASS(in_socketPartitions);


OS_CLASS(in_address);
OS_CLASS(in_connectivityAdmin);
OS_CLASS(in_connectivityEntityFacade);
OS_CLASS(in_connectivityListener);
OS_CLASS(in_connectivityParticipantFacade);
OS_CLASS(in_connectivityPeerEntity);
OS_CLASS(in_connectivityPeerParticipant);
OS_CLASS(in_connectivityPeerReader);
OS_CLASS(in_connectivityPeerWriter);
OS_CLASS(in_connectivityReaderFacade);
OS_CLASS(in_connectivityWriterFacade);

OS_CLASS(in_controller);
OS_CLASS(in_runnable);

typedef void *in_ddsiParameterToken;

OS_CLASS(in_ddsiSubmessage);
OS_CLASS(in_ddsiSubmessageInfoTimestamp);
OS_CLASS(in_ddsiSerializedData);
OS_CLASS(in_ddsiSubmessageData);
OS_CLASS(in_ddsiDeserializer);
OS_CLASS(in_ddsiParameterHeader);
OS_CLASS(in_ddsiParameterList);
OS_CLASS(in_ddsiParticipantBuiltinTopicData);
OS_CLASS(in_ddsiParticipantProxy);
OS_CLASS(in_ddsiDiscoveredParticipantData);
OS_CLASS(in_ddsiSPDPdiscoveredParticipantData);
OS_CLASS(in_ddsiPublicationBuiltinTopicData);
OS_CLASS(in_ddsiWriterProxy);
OS_CLASS(in_ddsiDiscoveredWriterData);
OS_CLASS(in_ddsiReceiver);
OS_CLASS(in_ddsiSerializer);
OS_CLASS(in_ddsiStreamReaderImpl);
OS_CLASS(in_ddsiStreamWriterImpl);
OS_CLASS(in_ddsiDataFrag);
OS_CLASS(in_ddsiHeartbeat);
OS_CLASS(in_ddsiAckNack);
OS_CLASS(in_ddsiNackFrag);
OS_CLASS(in_ddsiNackFragRequest);
OS_CLASS(in_ddsiSubmessageDeserializer);
OS_CLASS(in_ddsiSubmessageTokenizer);
OS_CLASS(in_ddsiSubscriptionBuiltinTopicData);
OS_CLASS(in_ddsiReaderProxy);
OS_CLASS(in_ddsiDiscoveredReaderData);
OS_CLASS(in_ddsiTopicBuiltinTopicData);
OS_CLASS(in_ddsiDiscoveredTopicData);
OS_CLASS(in_locator);

/* Close the brace that allows the usage of this code in C++. */
#if defined (__cplusplus)
}
#endif

#endif /* IN_COMMONTYPES_H */

