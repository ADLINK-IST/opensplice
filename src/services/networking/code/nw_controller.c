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
/* interface */
#include "nw_controller.h"

/* implementation */
#include "string.h"
#include "os.h"
#include "c_base.h"         /* C_CAST   etc */
#include "c_iterator.h"
#include "u_user.h"
#include "v_networkReader.h"
#include "v_networkReaderEntry.h"
#include "nw__confidence.h"
#include "nw_configuration.h"
#include "nw__runnable.h"   /* baseclass */
#include "nw_bridge.h"
#include "nw_partitions.h"
#include "nw_channelUser.h"
#include "nw_channelReader.h"
#include "nw_channelWriter.h"
#include "nw_misc.h"
#include "nw_socketMisc.h" /* Low level function for checking correctness of address */
#include "nw_discovery.h"
#include "nw_report.h"
#include "v_entity.h"      /* for v_entity() */
#include "v_service.h"     /* for v_service() */
#include "v_group.h"       /* for v_group */
#include "v_topic.h"
#include "v_domain.h"
#include "v_event.h"

#define NW_CHANNELUSER_BY_ID(controller, id) (controller->channelUsers[id])
#define NW_SUBSCRIBER_NAME      "Networking subscriber"
#define NW_READER_NAME          "Networking reader"

C_STRUCT(nw_controller) {
    /* My owner */
    u_service service;

    /* Sub admin */
    u_subscriber subscriber;
    u_networkReader reader;
  
    /* Discovery of other nodes */
    nw_discoveryWriter discoveryWriter;
    nw_discoveryReader discoveryReader;
  
    /* Channels for writing and reading data */
    nw_bridge bridge;
    unsigned int nofChannelUsers;
    nw_channelUser *channelUsers /* [nofChannelUsers] */;
    unsigned int nofEntryReaders;
    v_networkId networkId;

    /* Class for administration and lookup of networking partitions */
    nw_partitions partitions;

};

/* --------------------- private -------------------- */

struct readerActionArg {
    nw_controller controller;
    v_group group;
};

static void
onNewGroupReaderAction(
    v_entity e,
    c_voidp arg)
{
    v_networkReader reader = v_networkReader(e);
    struct readerActionArg *actionArg = (struct readerActionArg *)arg;
    v_group group;
    nw_controller controller;
    v_networkReaderEntry entry;   
    unsigned int i;
    v_networkPartitionId networkPartitionId;

    reader = v_networkReader(e);
    actionArg = (struct readerActionArg *)arg;
    controller = actionArg->controller;
    group = actionArg->group;
    
    /* Lookup an entry from the new group to check if we have not been 
     * notified of this before... */
    entry = v_networkReaderLookupEntry(reader, group);
    if (!entry) {
        /* This is the first time we are here
         * Create a new entry myself because the usual subscribe-mechanism
         * does not do this for us (no autoconnect) */
        /* First determine the corresponding networking partition number */
        networkPartitionId = nw_partitionsLookupBestFit(
                                  controller->partitions,
                                  v_partitionName(v_groupPartition(group)),
                                  v_topicName(v_groupTopic(group)));

        if ( networkPartitionId != V_NETWORKPARTITIONID_LOCALHOST ){

            NW_TRACE_3(Test, 3,
                       "Selected partitionId %d for "
                       "partition %s, topic %s",
                       networkPartitionId,
                       v_partitionName(v_groupPartition(group)),
                       v_topicName(v_groupTopic(group)));

            entry = v_networkReaderEntryNew(reader, group,
                                            controller->networkId,
                                            controller->nofEntryReaders,
                                            networkPartitionId);
            
            /* Walk over all channelUsers to notify them on this new group */
            for (i=0; i<controller->nofChannelUsers; i++) {
                nw_channelUserNotifyNewGroup(
                    NW_CHANNELUSER_BY_ID(controller, i), 
                    entry);
                /* For the test, do not output addition of builtin-partitions
                 * because this might happen in any order */
                if (v_partitionName(v_groupPartition(group))[0] != '_') {
                    NW_TRACE_3(Test, 1,
                               "Group added for channel %d, partition = %s, "
                               "topic = %s", i,
                               v_partitionName(v_groupPartition(group)),
                               v_topicName(v_groupTopic(group)));
                }
            }
        } else {
            NW_TRACE_2(Test, 3,
                       "Ignoring partition %s, topic %s",
                       v_partitionName(v_groupPartition(group)),
                       v_topicName(v_groupTopic(group)));
            
            v_groupNotifyAwareness(group, u_serviceGetName(controller->service), FALSE);
        }
    }
}


/* Callback for notification of newly created groups */
static void
onNewGroup(
    v_entity e,
    c_voidp arg)
{
    nw_controller controller = (nw_controller)arg;
    v_service kservice;
    c_iter newGroups;
    v_group group;
    struct readerActionArg actionArg;

    kservice = v_service(e);
    newGroups = v_serviceTakeNewGroups(kservice);
    group = v_group(c_iterTakeFirst(newGroups));
    actionArg.controller = controller;
    while (group) {
        actionArg.group = group;
        u_entityAction(u_entity(controller->reader), onNewGroupReaderAction,
            &actionArg);
        c_free(group);
        group = v_group(c_iterTakeFirst(newGroups));
    }
    c_iterFree(newGroups);
}
            
static c_ulong
nw_controllerOnNewGroup(
    u_dispatcher o,
    c_ulong event,
    c_voidp userData)
{
    u_result result;
    u_service service = u_service(o);

    if (userData &&
        ((event == V_EVENT_NEW_GROUP) || (event == V_EVENT_UNDEFINED))) {
        result = u_entityAction(u_entity(service),onNewGroup,userData);
    }
    return event;
}

static void
fillNewGroups(
    v_entity e,
    c_voidp arg)
{
    v_service service = v_service(e);
    if (arg) { /* arg parameter unused */
    }
    v_serviceFillNewGroups(service);
}


static v_networkId
getNetworkId(
    nw_controller controller)
{
    os_time time;
   
    (void)controller;

    /* NOTE: for now, let network ID be a "random"-number. This number has to be
     *       retrieved from the network/os layer. */

    time = os_timeGet();
    return time.tv_nsec;
}


/* Callback for notification of remotely sent heartbeats */
static void
onNodeStarted(
    v_networkId networkId,
    nw_address address,
    c_time detectedTime,
    unsigned int aliveCount,
    nw_discoveryMsgArg arg)
{
    nw_controller controller = (nw_controller)arg;
    (void)detectedTime;
    
    NW_CONFIDENCE(controller->discoveryReader != NULL);
    NW_CONFIDENCE(controller->discoveryWriter != NULL);

    NW_TRACE_2(Discovery, 1, "Discovered active remote node with id 0x%x, currently %u active nodes known",
        networkId, aliveCount);
    /* Advertise my existence */
    /* Note: this will be p2p in the future */
    nw_discoveryWriterRespondToStartingNode(controller->discoveryWriter);
    /* Tell the bridge about this new node */
    nw_bridgeNotifyNodeStarted(controller->bridge, networkId, address);
    if (aliveCount == 1) {
        /* Now it is useful to send messages to the network, so notify
         * the network reader to switch off its filter */
        u_networkReaderRemoteActivityDetected(controller->reader);
        NW_TRACE(Discovery, 1, "Switched on forwarding to the network");
    }
}


static void
onNodeStopped(
    v_networkId networkId,
    nw_address address,
    c_time detectedTime,
    unsigned int aliveCount,
    nw_discoveryMsgArg arg)
{
    nw_controller controller = (nw_controller)arg;
    (void)detectedTime;
    
    NW_CONFIDENCE(controller->discoveryReader != NULL);
    NW_CONFIDENCE(controller->discoveryWriter != NULL);

    NW_TRACE_2(Discovery, 1, "Node with id 0x%x has stopped, currently %u active nodes known",
        networkId, aliveCount);
    /* Tell the bridge about this stopped node */
    nw_bridgeNotifyNodeStopped(controller->bridge, networkId, address);
    if (aliveCount == 0) {
        u_networkReaderRemoteActivityLost(controller->reader);
        NW_TRACE(Discovery, 1, "Switched off forwarding to the network");
    }
}


static void
onNodeDied(
    v_networkId networkId,
    nw_address address,
    c_time detectedTime,
    unsigned int aliveCount,
    nw_discoveryMsgArg arg)
{
    nw_controller controller = (nw_controller)arg;
    (void)detectedTime;
    
    NW_CONFIDENCE(controller->discoveryReader != NULL);
    NW_CONFIDENCE(controller->discoveryWriter != NULL);

    NW_TRACE_2(Discovery, 1, "Node with id 0x%x has died, currently %u active nodes known",
        networkId, aliveCount);
    /* Tell the bridge about this stopped node */
    nw_bridgeNotifyNodeDied(controller->bridge, networkId, address);
    if (aliveCount == 0) {
        u_networkReaderRemoteActivityLost(controller->reader);
        NW_TRACE(Discovery, 1, "Switched off forwarding to the network");
    }
}

static void
onFatal(void)
{
    
}

/* ------------- Main function of this controller ------------ */

static void
nw_controllerInitializeChannels(
    nw_controller controller,
    nw_onFatalCallBack onFatal,
    c_voidp onFatalUsrData)
{
    unsigned int nofChannelUsers = 0;
    unsigned int nofEntryReaders = 0;
    nw_sendChannel sendChannel;
    nw_receiveChannel receiveChannel;
    int nofChannels = 0;
    int iChannel;
    c_char *channelName;
    nw_bool useDiscovery = FALSE;

    c_iter channelList;
    u_cfElement channel;
    u_cfAttribute attr;
    c_char* channelPath;

    if (controller->bridge) {

#define NW_CHANNEL_PATH NWCF_ROOT(Channel)"[@"NWCF_ATTRIB_enabled"!='false']"

        channelList = nw_configurationGetElements(NW_CHANNEL_PATH);
        if (channelList != NULL) {
            nofChannels = c_iterLength(channelList);
        }

  
        if (nofChannels == 0) {
             controller->channelUsers = NULL;
        } else {
             /* Create channelUsers */
             controller->channelUsers= (nw_channelUser *)os_malloc(
                 2 * nofChannels * 
                 (os_uint32)sizeof(*controller->channelUsers));

            for (iChannel=0; iChannel<nofChannels; iChannel++) {
                channel = u_cfElement(c_iterTakeFirst(channelList));
                attr = u_cfElementAttribute(channel, NWCF_ATTRIB(ChannelName));
                if (attr != NULL) {
                    if (!u_cfAttributeStringValue(attr, &channelName)) {
                        NW_REPORT_ERROR("Controller Initialization", "Error in channel name");
                        /* Memory leak here... */
                        channelName = nw_stringDup("");
                    }
                
                    channelPath = os_malloc(
                        strlen(NWCF_ROOT(Channel)"[@" NWCF_ATTRIB_ChannelName "='']") +
                        strlen(channelName)+1);
                    sprintf(channelPath, NWCF_ROOT(Channel) "[@" NWCF_ATTRIB_ChannelName "='%s']", channelName);

                    sendChannel = nw_bridgeNewSendChannel(controller->bridge, channelPath, onFatal, onFatalUsrData);
                    if (sendChannel) {
                        NW_CHANNELUSER_BY_ID(controller, nofChannelUsers) =
                            (nw_channelUser)nw_channelWriterNew(u_serviceGetName(controller->service),channelPath,
                                         sendChannel, controller->reader);
                        nofChannelUsers++;
                        nofEntryReaders++;
                    }
                    receiveChannel = nw_bridgeNewReceiveChannel(controller->bridge, channelPath, onFatal, onFatalUsrData);
                    if (receiveChannel) {
                        NW_CHANNELUSER_BY_ID(controller, nofChannelUsers) =
                            (nw_channelUser)nw_channelReaderNew(channelPath,
                                      receiveChannel, controller->reader);
                        nofChannelUsers++;
                    }
                    os_free(channelPath);
                    u_cfAttributeFree(attr);
                } else {
                    NW_REPORT_ERROR("Controller Initialization", "Skipping anonymous channel");
                    channelName = NULL;
                }
                u_cfElementFree(channel);
            }
            c_iterFree(channelList);
        }
#undef NW_CHANNEL_PATH
        
        /* Now find an enabled discovery channel */
#define NW_DISCOVERY_PATH NWCF_ROOT(DiscoveryChannel) "[@"NWCF_ATTRIB_enabled"!='false']"

        channelList = nw_configurationGetElements(NW_DISCOVERY_PATH);
        channel = c_iterTakeFirst(channelList);
        if (channel != NULL) {
            useDiscovery = TRUE;
        } else {
            /* Also use discovery if there is no channel at all */
            c_iterFree(channelList);
            channelList = nw_configurationGetElements(NWCF_ROOT(DiscoveryChannel));
            channel = c_iterTakeFirst(channelList);
            if (channel == NULL) {
                useDiscovery = TRUE;
            } else {
                /* There is a discovery channel but it is disabled */
                u_cfElementFree(channel);
            }
        }
        if (useDiscovery) {
            controller->discoveryWriter = nw_discoveryWriterNew(controller->networkId, NW_DISCOVERY_PATH);
            controller->discoveryReader = nw_discoveryReaderNew(controller->networkId, 
               NW_DISCOVERY_PATH, onNodeStarted, onNodeStopped, onNodeDied, controller);
#ifdef NW_LOOPBACK
            if (nw_configurationUseLoopback()) {
                u_networkReaderRemoteActivityDetected(controller->reader);
            }
#endif
            u_cfElementFree(channel);
        }
#undef NW_DISCOVERY_PATH
        
        channel = c_iterTakeFirst(channelList);
        while (channel != NULL) {
            u_cfElementFree(channel);
            channel = c_iterTakeFirst(channelList);
        }
        c_iterFree(channelList);
    }
    controller->nofChannelUsers = nofChannelUsers;
    controller->nofEntryReaders = nofEntryReaders;

    if (!useDiscovery) {
        NW_REPORT_INFO(2, "De-activating discovery functionality, always forwarding data to the network")
        controller->discoveryWriter = NULL;
        controller->discoveryReader = NULL;
        /* Always forward to network */
        u_networkReaderRemoteActivityDetected(controller->reader);
    } else {
        NW_REPORT_INFO(2, "Activating discovery functionality, only forwarding data to the network if other nodes are present")
    }
}


static nw_bool
nw_controllerSplitPartitionTopic(
    const char *expression,
    char **partitionPart,
    char **topicPart)
{
    nw_bool result = FALSE;
    nw_bool incorrectWildcard = FALSE;
    const char *partitionPos;
    os_address partitionLen;
    char *dotPos;
    const char *topicPos;
    unsigned int topicLen;
    char *wildcardPos;
    
    if (expression != NULL) {
        partitionPos = expression;
        dotPos = strchr(expression, '.');
        if (dotPos != NULL) {
            /* Copy partitionpart */
            partitionLen = (os_address)dotPos - (os_address)partitionPos;
            *partitionPart = os_malloc(partitionLen + 1);
            strncpy(*partitionPart, partitionPos, partitionLen);
            (*partitionPart)[partitionLen] = '\0';
            /* Check validity */
            wildcardPos = strchr(*partitionPart, '*');
            if (wildcardPos != NULL) {
                incorrectWildcard = (wildcardPos != *partitionPart) ||
                    (partitionLen != 1);
            }
            if (!incorrectWildcard) {
                /* Copy topicpart */
                topicPos = &(dotPos[1]);
                topicLen = strlen(topicPos);
                *topicPart = os_malloc(topicLen + 1);
                strncpy(*topicPart, topicPos, topicLen);
                (*topicPart)[topicLen] = '\0';
                /* Check validity */
                wildcardPos = strchr(*topicPart, '*');
                if (wildcardPos != NULL) {
                    incorrectWildcard = (wildcardPos != *topicPart) ||
                        (topicLen != 1);
                }
                if (!incorrectWildcard) {
                    result = TRUE;
                } else {
                    os_free(*partitionPart);
                    *partitionPart = NULL;
                    os_free(*topicPart);
                    *topicPart = NULL;
                }
            } else {
                os_free(*partitionPart);
                *partitionPart = NULL;
            }
        }
    }
    
    return result;
}

static void
nw_controllerInitializePartitions(
    nw_controller controller)
{
    c_iter partitionList;
    c_iter mappingList;
    int nofPartitions;
    int iPartition;
    u_cfElement partition;
    u_cfAttribute attrAddress;
    char *partitionAddress;
    u_cfAttribute attrName;
    char *partitionName;
    u_cfAttribute attrConnected;
    c_bool connected;

    int nofMappings;
    int iMapping;
    u_cfElement mapping;
    u_cfAttribute attrExpression;
    char *mappingExpression;
    char *partitionExpression;
    char *topicExpression;
    /* Checking if the address is valid */
    sk_addressType addressType;
    
    controller->partitions = nw_partitionsNew();
    
    /* First get the GlobalPartition aka default partition */
    partitionAddress = NWCF_DEFAULTED_ATTRIB(String, NWCF_ROOT(GlobalPartition),
        NWPartitionAddress, NWCF_DEF(GlobalAddress), NWCF_DEF(GlobalAddress));

    addressType = sk_getAddressType(partitionAddress);
    if (addressType == SK_TYPE_UNKNOWN) {
        NW_REPORT_ERROR_2("initializing network",
            "Incorrect address %s read from configuration, using default %s",
            partitionAddress, NWCF_DEF(GlobalAddress));
        os_free(partitionAddress);
        partitionAddress = nw_stringDup( NWCF_DEF(GlobalAddress));
    }

    nw_partitionsSetGlobalPartition(controller->partitions, partitionAddress);
    os_free(partitionAddress);
    
    partitionList = nw_configurationGetElements(NWCF_ROOT(NWPartition));
    nofPartitions = c_iterLength(partitionList);
    for (iPartition=1; iPartition<=nofPartitions; iPartition++) {
        partition = u_cfElement(c_iterTakeFirst(partitionList));
        attrAddress = u_cfElementAttribute(partition, NWCF_ATTRIB(NWPartitionAddress));
        if (attrAddress != NULL) {
            u_cfAttributeStringValue(attrAddress, &partitionAddress);
            attrName = u_cfElementAttribute(partition, NWCF_ATTRIB(NWPartitionName));
            if (attrName != NULL) {
                u_cfAttributeStringValue(attrName, &partitionName);
            } else {
                partitionName = nw_stringDup(partitionAddress);
            }
            attrConnected = u_cfElementAttribute(partition, NWCF_ATTRIB(Connected));
            if (attrConnected != NULL) {
                u_cfAttributeBoolValue(attrConnected, &connected);
            } else {
                connected = NWCF_DEF(Connected);
            }
            nw_partitionsAddPartition(controller->partitions, iPartition,
                partitionName, partitionAddress, connected);
            NW_TRACE_3(Test, 2, "Read networking partition (%s,%s,%s)",
                partitionName, partitionAddress, (connected?"Connected":"Disconnected"));
        } else {
            NW_REPORT_ERROR("Controller Initialization",
                "Partition contains no address");
        }
    }

    /* Add ignored partitions */
    mappingList = nw_configurationGetElements(NWCF_ROOT(IgnoredPartition));
    nofMappings = c_iterLength(mappingList);
    for (iMapping=1; iMapping<=nofMappings; iMapping++) {
        mapping = u_cfElement(c_iterTakeFirst(mappingList));
        attrExpression = u_cfElementAttribute(mapping, NWCF_ATTRIB_DCPSPartitionTopic);
        if (attrExpression != NULL) {
            u_cfAttributeStringValue(attrExpression, &mappingExpression);
        } else {
            NW_REPORT_ERROR("Controller Initialization",
                "IgnoredPartition contains no DCPS partition-topic expression");
        }
        if (nw_controllerSplitPartitionTopic(mappingExpression,
            &partitionExpression, &topicExpression)) {
            nw_partitionsAddMapping(controller->partitions,
                partitionExpression, topicExpression, NW_LOCALPARTITION_NAME);
        } else {
            NW_REPORT_ERROR_1("Controller Initialization",
                "IgnoredPartition contains invalid partition-topic expression '%s'",
                mappingExpression);
        }
    }

    /* Add all mappings */
    mappingList = nw_configurationGetElements(NWCF_ROOT(PartitionMapping));
    nofMappings = c_iterLength(mappingList);
    for (iMapping=1; iMapping<=nofMappings; iMapping++) {
        mapping = u_cfElement(c_iterTakeFirst(mappingList));
        attrName = u_cfElementAttribute(mapping, NWCF_ATTRIB_NetworkPartition);
        if (attrName != NULL) {
            u_cfAttributeStringValue(attrName, &partitionName);
            attrExpression = u_cfElementAttribute(mapping, NWCF_ATTRIB_DCPSPartitionTopic);
            if (attrExpression != NULL) {
                u_cfAttributeStringValue(attrExpression, &mappingExpression);
            } else {
                NW_REPORT_ERROR("Controller Initialization",
                    "PartitionMapping contains no DCPS partition-topic expression");
            }
            if (nw_controllerSplitPartitionTopic(mappingExpression,
                &partitionExpression, &topicExpression)) {
                nw_partitionsAddMapping(controller->partitions,
                    partitionExpression, topicExpression, partitionName);
            } else {
                NW_REPORT_ERROR_1("Controller Initialization",
                    "PartitionMapping contains invalid partition-topic expression '%s'",
                    mappingExpression);
            }
        } else {
            NW_REPORT_ERROR("Controller Initialization",
                "PartitionMapping contains no network partition name");
        }
    }

}


static void
nw_controllerInitialize(
    nw_controller controller,
    u_service service,
    nw_onFatalCallBack onFatal,
    c_voidp onFatalUsrData)
{
    v_subscriberQos subscriberQos;
        
    controller->service = service;

    nw_controllerInitializePartitions(controller);

    subscriberQos = u_subscriberQosNew(NULL);

    /* Do not autoconnect, but react on newGroup notifications */
    os_free(subscriberQos->partition);
    subscriberQos->partition = NULL;

    controller->subscriber = u_subscriberNew(u_participant(service),
                                             NW_SUBSCRIBER_NAME,
                                             subscriberQos,
                                             TRUE);

    os_free(subscriberQos);

    /* Create a dataReader to read from */
    controller->reader = u_networkReaderNew(controller->subscriber,
                                            NW_READER_NAME, 
					    NULL,
					    FALSE); /*ignoreReliabilityQoS*/
    controller->networkId = getNetworkId(controller);
    controller->bridge = nw_bridgeNew(controller->networkId);
    
    nw_controllerInitializeChannels(controller,onFatal,onFatalUsrData);

     NW_TRACE_1(Discovery, 1, "Networking service has handed out nodeId 0x%x",
         controller->networkId);
}
#undef NW_HEARTBEAT_INTERVAL
#undef NW_DEATH_DETECTION_TIME
#undef NW_SUBSCRIBER_NAME
#undef NW_SUBSCRIBER_PART
#undef NW_READER_NAME

static void
nw_controllerFinalize(
    nw_controller controller)
{
    unsigned int i;
    
    if (controller) {
      
        /* Finalize self */
        if (controller->discoveryWriter != NULL) {
            nw_runnableFree((nw_runnable)controller->discoveryWriter);
        }
        if (controller->discoveryReader != NULL) {
            nw_runnableFree((nw_runnable)controller->discoveryReader);
        }
        /* No more messages to the network */
        u_networkReaderRemoteActivityLost(controller->reader);
        /* Free all channels */
        for (i=0; i<controller->nofChannelUsers; i++) {
            nw_runnableFree((nw_runnable)NW_CHANNELUSER_BY_ID(controller, i));
        }
        os_free(controller->channelUsers);
        nw_bridgeFree(controller->bridge);
        u_networkReaderFree(controller->reader);
        u_subscriberFree(controller->subscriber);
    }
}    


/* ---------------------- public -------------------- */

nw_controller

nw_controllerNew(
    u_service service, 
    nw_controllerListener onFatal, 
    c_voidp usrData)
{
    nw_controller result;

    result = (nw_controller)os_malloc((os_uint32)sizeof(*result));

    if (result != NULL) {
        /* Initialize self */
        nw_controllerInitialize(result, service, onFatal, usrData);
    }
    
    return result;
}


void
nw_controllerFree(
    nw_controller controller)
{
    if (controller) {
        nw_controllerFinalize(controller);
        os_free(controller);
    }
}

void
nw_controllerStart(
    nw_controller controller)
{
    unsigned int i;
    c_ulong mask;
    
    if (controller) {

        /* First start all channelUser threads */
        for (i=0; i<controller->nofChannelUsers; i++) {
            nw_runnableStart((nw_runnable)NW_CHANNELUSER_BY_ID(controller, i));
        }
        
        /* Set the listener of the service */
        u_dispatcherGetEventMask(u_dispatcher(controller->service), &mask);
        u_dispatcherSetEventMask(u_dispatcher(controller->service), 
            mask | V_EVENT_NEW_GROUP);
        u_entityAction(u_entity(controller->service), fillNewGroups, NULL);
        u_dispatcherInsertListener(u_dispatcher(controller->service), 
            nw_controllerOnNewGroup, controller);

        /* Start announcing my existence */
        if (controller->discoveryWriter != NULL) {
            nw_runnableStart((nw_runnable)controller->discoveryWriter);
        }
        /* Start discovering other nodes */
        if (controller->discoveryReader != NULL) {
            nw_runnableStart((nw_runnable)controller->discoveryReader);
        }
    }
}


void
nw_controllerStop(
    nw_controller controller)
{
    unsigned int i;
    c_ulong mask;

    if (controller) {
        /* Let kernel know that no data has to be sent to the network anymore */
        u_networkReaderRemoteActivityLost(controller->reader);

        /* Stop announcinng my existence */
        if (controller->discoveryWriter != NULL) {
            nw_runnableStop((nw_runnable)controller->discoveryWriter);
        }
        /* Stop listening to others'existence */
        if (controller->discoveryReader != NULL) {
            nw_runnableStop((nw_runnable)controller->discoveryReader);
        }


        /* Stop listening */
         u_dispatcherGetEventMask(u_dispatcher(controller->service), &mask);
         u_dispatcherSetEventMask(u_dispatcher(controller->service), 
            mask & ~V_EVENT_NEW_GROUP);
        u_dispatcherRemoveListener(u_dispatcher(controller->service),
            nw_controllerOnNewGroup);
        
        /* Stop all channelUser threads */
        for (i=0; i<controller->nofChannelUsers; i++) {
            nw_runnableStop((nw_runnable)NW_CHANNELUSER_BY_ID(controller, i));
        }
    }
}

void
nw_controllerUpdateHeartbeats(
    nw_controller controller)
{
    /* Wake-up the discoveryReader so it can check the died nodes */
    if (controller->discoveryReader != NULL) {
        nw_discoveryReaderInitiateCheck(controller->discoveryReader);
    }
}


