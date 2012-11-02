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
/* interface */
#include "nw_plugNetwork.h"

/* implementation */
#include "os_abstract.h"
#include "os_heap.h"
#include "nw__confidence.h"
#include "nw__plugReceiveChannel.h" /* for constructor */
#include "nw__plugSendChannel.h"    /* for constructor */
#include "nw__plugPartitions.h"     /* for constructor and setters */
#include "nw_plugChannel.h"
#include "nw_report.h"
#include "nw_configuration.h"
#include "nw_misc.h"          /* for nw_stringDup */
/* For helper function */
#include "nw_socketMisc.h"
#include "ut_crc.h"

#define NW_PLUG_PROTOCOL_VERSION (1U)
#define NW_MAX_CHANNEL_ID    (84U) /* resulting in 42 configurable channels for the networking service */

#define NW_CHANNEL_ID_IS_VALID(plugNetwork,id)  (id < plugNetwork->nofChannels)
#define NW_CHANNEL_BY_ID(plugNetwork,id)        (plugNetwork->channels[id])
#define NW_USERDATA_ID_IS_VALID(plugNetwork,id) (id < plugNetwork->nofPaths)
#define NW_USERDATA_PATH_BY_ID(plugNetwork,id)  (plugNetwork->pathUserData[id].pathName)
#define NW_USERDATA_DATA_BY_ID(plugNetwork,id)  (plugNetwork->pathUserData[id].userData)

typedef unsigned char nw_protocolVersion;

NW_CLASS(nw_pathUserData);
NW_STRUCT(nw_pathUserData) {
    char *pathName;
    nw_userData userData;
};


NW_STRUCT(nw_plugNetwork) {
    nw_networkId networkId;
    nw_protocolVersion protocolVersion;          /* For version checking   */
    nw_plugPartitions partitions;                /* networking partitions */
    nw_seqNr nofChannels;                        /* Nr of channels created */
    nw_seqNr nofCreatedChannels;
    nw_plugChannel *channels;                    /* [nofChannels] */
    nw_seqNr nofPaths;
    ut_crc partitionCrc;						 /* partitionId crc hash object */
    NW_STRUCT(nw_pathUserData) *pathUserData;    /* [nofChannels] */
};

/* Global variable to implemente singleton mechanism */
static nw_plugNetwork G_nw_plugIncarnation = NULL;
static os_uint32   G_nw_nofPlugIncarnations = 0;

/* NetworkPlug class operations */

/* Private functions */

static nw_userData *
nw_plugNetworkGetOrCreateUserDataPtr(
    nw_plugNetwork network,
    const char *pathName)
{
    nw_userData *result = NULL;
    nw_seqNr i;
    
    for (i=0; (i<network->nofPaths) && (result == NULL); i++) {
        if (strcmp(pathName, NW_USERDATA_PATH_BY_ID(network, i)) == 0) {
            /* Pathname found, so return pointer to userdata */
            result = &(NW_USERDATA_DATA_BY_ID(network, i));
        }
    }
    
    if (result == NULL) {
        NW_USERDATA_PATH_BY_ID(network, network->nofPaths) = nw_stringDup(pathName);
        result = &(NW_USERDATA_DATA_BY_ID(network, network->nofPaths));
        network->nofPaths++;
    }
    
    return result;
}

static void
nw_plugNetworkInitializePartitions(
    nw_plugNetwork plugNetwork)
{
    c_iter partitionList;
    nw_plugPartitions partitions;
    nw_partitionId nofPartitions;
    nw_partitionId partitionId;
    u_cfElement partitionElement;
    u_cfAttribute attrConnected;
    c_bool connected;
    u_cfAttribute attrCompression;
    c_bool compression;
    u_cfAttribute attrAddress;
    u_cfAttribute attrName;
    char *partitionAddress;
    char *partitionName;
    sk_addressType addressType;
    
    u_cfAttribute attrSecurityPolicy;
    char *securityPolicy;
    
    u_cfAttribute attrMulticastTimeToLive;
    c_ulong multicastTimeToLive;

    partitionList = nw_configurationGetElements(NWCF_ROOT(NWPartition));
    nofPartitions = (nw_partitionId)c_iterLength(partitionList) + 1 /* Do not forget the default partition */;
    partitions = nw_plugPartitionsNew(nofPartitions);
    plugNetwork->partitions = partitions;

    /* First get the GlobalPartition aka default partition */
    partitionAddress = NWCF_DEFAULTED_ATTRIB(String, NWCF_ROOT(GlobalPartition),
        NWPartitionAddress, NWCF_DEF(GlobalAddress), NWCF_DEF(GlobalAddress));
        
    /* might return NULL */ 
    securityPolicy = NWCF_DEFAULTED_ATTRIB(String, NWCF_ROOT(GlobalPartition),
        NWSecurityPolicy, NWCF_DEF(NWSecurityPolicy), NWCF_DEF(NWSecurityPolicy));

    multicastTimeToLive = NWCF_DEFAULTED_ATTRIB(ULong, NWCF_ROOT(GlobalPartition),
            MulticastTimeToLive, NWCF_DEF(MulticastTimeToLive), NWCF_DEF(MulticastTimeToLive));

    addressType = sk_getAddressType(partitionAddress);
    if (addressType == SK_TYPE_UNKNOWN) {
        os_free(partitionAddress);
        partitionAddress = nw_stringDup( NWCF_DEF(GlobalAddress));
    }


    nw_plugPartitionsSetDefaultPartition(partitions, partitionAddress,securityPolicy,
    		ut_crcCalculate(plugNetwork->partitionCrc,"DefaultPartition",strlen("DefaultPartition")),
    		multicastTimeToLive);

    os_free(partitionAddress);
    os_free(securityPolicy);
    
    for (partitionId=1; partitionId<=(nofPartitions-1); partitionId++) {
        partitionElement = u_cfElement(c_iterTakeFirst(partitionList));
        attrConnected = u_cfElementAttribute(partitionElement, NWCF_ATTRIB_Connected);
        if (attrConnected != NULL) {
            u_cfAttributeBoolValue(attrConnected,  &connected);
	    u_cfAttributeFree(attrConnected);
        } else {
            connected = NWCF_DEF_Connected;
        }
        attrCompression = u_cfElementAttribute(partitionElement, NWCF_ATTRIB_Compression);
        if (attrCompression != NULL) {
            u_cfAttributeBoolValue(attrCompression,  &compression);
	    u_cfAttributeFree(attrCompression);
        } else {
            compression = NWCF_DEF_Compression;
        }
        attrMulticastTimeToLive = u_cfElementAttribute(partitionElement, NWCF_ATTRIB(MulticastTimeToLive));
        if (attrMulticastTimeToLive != NULL) {
            u_cfAttributeULongValue(attrMulticastTimeToLive, &multicastTimeToLive);
            u_cfAttributeFree(attrMulticastTimeToLive);
        } else {
            multicastTimeToLive = NWCF_DEF(MulticastTimeToLive);
        }

#ifdef OSPL_NO_ZLIB
        if (compression) {
            NW_REPORT_WARNING("Channel setup", "Compression configured but not enabled in this build");
        }
#endif  /* OSPL_NO_ZLIB */
        attrAddress = u_cfElementAttribute(partitionElement, NWCF_ATTRIB_NWPartitionAddress);
        if (attrAddress != NULL) {
            u_cfAttributeStringValue(attrAddress, &partitionAddress);
	    u_cfAttributeFree(attrAddress);

	    attrSecurityPolicy = u_cfElementAttribute(partitionElement, 
						      NWCF_ATTRIB(NWSecurityPolicy));
	    
	    securityPolicy = NULL; 	    
	    if (attrSecurityPolicy!=NULL) {
		u_cfAttributeStringValue(attrSecurityPolicy, &securityPolicy);
		u_cfAttributeFree(attrSecurityPolicy);
	    }

	    attrName = u_cfElementAttribute(partitionElement, NWCF_ATTRIB_NWPartitionName);
        if (attrName != NULL) {
            u_cfAttributeStringValue(attrName, &partitionName);
        }
        u_cfAttributeFree(attrName);

            nw_plugPartitionsSetPartition(
                partitions,
                partitionId,
                partitionAddress,
                securityPolicy,
                ut_crcCalculate (
                    plugNetwork->partitionCrc,
                    (void *)partitionName,
                    strlen(
                       (char *)partitionName
                    )
                ),
                connected,
                compression,
                multicastTimeToLive
            );


        os_free(partitionAddress);
	    os_free(securityPolicy);
        } else {
            NW_REPORT_ERROR_1("plugNetwork initialization",
                "Partition %d contains no address", partitionId);
        }
	
	u_cfElementFree(partitionElement);
    }

    c_iterFree(partitionList);    
}

static nw_plugNetwork
nw_plugNetworkNew(
    nw_networkId networkId)
{
    nw_plugNetwork result;
    nw_seqNr nofChannels = NW_MAX_CHANNEL_ID;
    os_uint32 i;
    
    result = (nw_plugNetwork)os_malloc((os_uint32)sizeof(*result));
    
    if (result != NULL) {
        result->networkId = networkId;
        result->protocolVersion = (nw_protocolVersion)NW_PLUG_PROTOCOL_VERSION;
        result->nofCreatedChannels = 0;
        result->nofPaths = 0;
        result->partitionCrc = ut_crcNew(UT_CRC_KEY);
        
        /* Initialize channeltable */
        result->channels = (nw_plugChannel *)os_malloc(
            nofChannels * (os_uint32)sizeof(*result->channels));
        if (result->channels != NULL) {    
            for (i=0; i<nofChannels; i++) {
                NW_CHANNEL_BY_ID(result, i) = NULL;
            }
            result->nofChannels = nofChannels;
        } else {
            result->nofChannels = 0;
        }
        
        /* Initialize userData per path */
        result->pathUserData = (NW_STRUCT(nw_pathUserData) *)os_malloc(
            result->nofChannels * (os_uint32)sizeof(*result->pathUserData));
        if (result->pathUserData != NULL) {    
            for (i=0; i<result->nofChannels; i++) {
                NW_USERDATA_PATH_BY_ID(result, i) = NULL;
                NW_USERDATA_DATA_BY_ID(result, i) = NULL;
            }
        }
        
        /* Initialize newtorking partitions */
        nw_plugNetworkInitializePartitions(result);
    }
    
    return result;
}


static void
nw_plugNetworkFree(
    nw_plugNetwork network)
{
    nw_seqNr i;
    
    if (network != NULL) {
        /* Free policies and hashes */
        for (i = 0; i < network->nofChannels; i++) {
            nw_plugChannelFree(NW_CHANNEL_BY_ID(network, i));
        }
        os_free(network->channels);
        for (i = 0; i < network->nofPaths; i++) {
            os_free(NW_USERDATA_PATH_BY_ID(network, i));
        }
        os_free(network->pathUserData);
        os_free(network);
    }
}


/* Public */

nw_plugNetwork
nw_plugNetworkIncarnate(
    nw_networkId networkId)
{
    if (G_nw_plugIncarnation == NULL) {
        G_nw_plugIncarnation = nw_plugNetworkNew(networkId);
    } else {
        NW_CONFIDENCE(networkId == G_nw_plugIncarnation->networkId);
    }
    
    G_nw_nofPlugIncarnations++;
    NW_TRACE_1(Construction, 4, "Incarnated network, currently %u incarnations active",
        G_nw_nofPlugIncarnations);
    return G_nw_plugIncarnation;   
}


void
nw_plugNetworkExcarnate(
    nw_plugNetwork network)
{
    NW_CONFIDENCE(network);
     
    if (network) {
        G_nw_nofPlugIncarnations--;
        NW_TRACE_1(Destruction, 4, "Excarnated network, currently %u incarnations active",
            G_nw_nofPlugIncarnations);
        if (G_nw_nofPlugIncarnations == 0U) {
            NW_TRACE(Destruction, 3, "plugNetwork reference count dropped to zero, "
                "destroying");
            nw_plugNetworkFree(G_nw_plugIncarnation);
            G_nw_plugIncarnation = NULL;
        }
    }
}


static nw_plugChannel
nw_plugNetworkNewChannel(
    nw_plugNetwork network,
    nw_communicationKind communication,
    const char *pathName,
    nw_onFatalCallBack onFatal,
    c_voidp onFatalUsrData)
{
    nw_plugChannel result = NULL;
    nw_userData *userDataPtr;
    
    NW_CONFIDENCE(network);
    NW_CONFIDENCE(NW_CHANNEL_ID_IS_VALID(network, network->nofCreatedChannels));
    
    if ((network != NULL) &&
        NW_CHANNEL_ID_IS_VALID(network, network->nofCreatedChannels)) {
        
        userDataPtr = nw_plugNetworkGetOrCreateUserDataPtr(network, pathName);
        switch (communication) {
            case NW_COMM_RECEIVE:
                result = (nw_plugChannel)nw_plugReceiveChannelNew(
                    network->nofCreatedChannels, network->networkId,
                    network->partitions, userDataPtr, pathName,onFatal,onFatalUsrData);
            break;
            case NW_COMM_SEND:
                result = (nw_plugChannel)nw_plugSendChannelNew(
                    network->nofCreatedChannels, network->networkId,
                    network->partitions, userDataPtr, pathName,onFatal,onFatalUsrData);
            break;
        }
        if (result != NULL) {
            NW_CHANNEL_BY_ID(network, network->nofCreatedChannels) = result;
            network->nofCreatedChannels++;
        }
    }
    
    return result;
}

nw_plugChannel
nw_plugNetworkNewReceiveChannel(
    nw_plugNetwork network,
    const char *pathName,
    nw_onFatalCallBack onFatal,
    c_voidp onFatalUsrData)
{
    return nw_plugNetworkNewChannel(network, NW_COMM_RECEIVE, pathName,onFatal,onFatalUsrData);
}

nw_plugChannel
nw_plugNetworkNewSendChannel(
    nw_plugNetwork network,
    const char *pathName,
    nw_onFatalCallBack onFatal,
    c_voidp onFatalUsrData)
{
    return nw_plugNetworkNewChannel(network, NW_COMM_SEND, pathName,onFatal,onFatalUsrData);
}

nw_seqNr
nw_plugNetworkGetNetworkId(
    nw_plugNetwork network)
{
    return network->networkId;
}

nw_seqNr       
nw_plugNetworkGetMaxChannelId(
    nw_plugNetwork network)
{
    return network->nofChannels;
}



/* ------------- protected network to host conversion functions ------------- */

#ifdef PA_LITTLE_ENDIAN

typedef unsigned char nw_charArray[sizeof(nw_seqNr)];

nw_seqNr
nw_plugByteSwap(
    nw_seqNr value)
{
    union valPtr {
        nw_seqNr  value;
        nw_charArray array;
    } srcUnion, dstUnion;
    unsigned char *srcPtr;
    unsigned char *dstPtr;
    os_uint32 i;
    os_uint32 len;

    srcUnion.value = value;
    srcPtr = srcUnion.array;
    dstPtr = dstUnion.array;

    len = (os_uint32)sizeof(nw_seqNr);
    for (i = 0; i < len; i++) {
        dstPtr[i] = srcPtr[len - i - 1];
    }

    return dstUnion.value;
}

#endif
