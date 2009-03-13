#ifndef NW__PLUGCHANNEL_H
#define NW__PLUGCHANNEL_H

#include "nw_plugChannel.h"
#include "nw_plugPartitions.h"
#include "nw_plugInterChannel.h"
#include "nw_socket.h"


#define nw_plugChannel(o) ((nw_plugChannel)(o))
/* Class definition needed because inheriting classes need to know include
 * this */

NW_CLASS(nw_messageBox);
NW_STRUCT(nw_plugChannel) {
    /* Identity of this channel */
    char *name;
    nw_seqNr Id;
    /* Identity of the node */
    nw_seqNr nodeId;
    /* Sending or receiving channel */
    nw_communicationKind communication;
    /* Partition administration, read only */
    nw_plugPartitions partitions;
    /* Qos offered */
    nw_reliabilityKind reliabilityOffered;
    nw_priorityKind priorityOffered;
    nw_latencyBudget latencyBudgetOffered;
    /* The socket abstraction to read from or write to */
    nw_socket socket;
    /* Size of a single fragment */
    nw_length fragmentLength;
    /* Pointer to object for inter-channel communication */
    nw_plugInterChannel interChannelComm;
    /* Threadsafe message box for posting events */
    nw_messageBox messageBox; 
    /* Callback in case of an fatal error */
    nw_onFatalCallBack onFatal;
    c_voidp onFatalUsrData;
};    
  
/* Protected functions for descendants */

/* Getters (read only) */
#define nw__plugChannelGetName(channel)               (channel)->name
#define nw__plugChannelGetNodeId(channel)             (channel)->nodeId
#define nw__plugChannelGetCommunication(channel)      (channel)->communication
#define nw__plugChannelGetReliabilityOffered(channel) (channel)->reliabilityOffered
#define nw__plugChannelGetPriorityOffered(channel)    (channel)->priorityOffered
#define nw__plugChannelGetSocket(channel)             (channel)->socket
#define nw__plugChannelGetFragmentLength(channel)     (channel)->fragmentLength
#define nw__plugChannelGetInterChannelComm(channel)   (channel)->interChannelComm

void nw_plugChannelInitialize(
         nw_plugChannel channel,
         nw_seqNr seqNr,
         nw_networkId nodeId,
         nw_communicationKind communication,
         nw_plugPartitions partitions,
         nw_userData *userDataPtr,
         const char *pathName,
         nw_onFatalCallBack onFatal,
         c_voidp onFatalUsrData);

void nw_plugChannelFinalize(
         nw_plugChannel channel);
         
typedef enum nw_messageBoxMessageType_e {
    NW_MBOX_UNDEFINED,
    NW_MBOX_NODE_STARTED,
    NW_MBOX_NODE_STOPPED,
    NW_MBOX_NODE_DIED
} nw_messageBoxMessageType;

nw_bool nw_plugChannelProcessMessageBox(
         nw_plugChannel channel,
         nw_networkId *networkId /* out */,
         nw_address *address,
         nw_messageBoxMessageType *messageType /* out */);

void    nw_plugChannelGetPartition(
         nw_plugChannel channel,
         nw_partitionId partitionId,
         nw_bool *found,
         nw_partitionAddress *partitionAddress,
         nw_bool *connected);

#endif /* NW__PLUGCHANNEL_H */

