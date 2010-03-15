#include "in_channel.h"
#include "in_channelSdpWriter.h"
#include "in_report.h"
#include "os_heap.h"
#include "v_state.h"
#include "v_message.h"
#include "in_result.h"
#include "in_connectivityAdmin.h"
#include "Coll_Iter.h"
#include "Coll_List.h"
#include "Coll_Set.h"
#include "u_participant.h"
#include "in_clientMonitor.h"
#include "in_streamWriter.h"
#include "u_dispatcher.h"
#include "v_event.h"
#include "v_service.h"
#include "v_group.h"
#include "v_networkReader.h"
#include "v_partition.h"
#include "v_topic.h"
#include "v_networkReaderEntry.h"
#include "v_writer.h"
#include "v_public.h"
#include "v_dataReader.h"
#include "in_ddsiDefinitions.h"
#include "in_endpointDiscoveryData.h"
#include "in__ddsiReceiver.h"

static os_boolean
in_channelSdpWriterInit(
    in_channelSdpWriter _this,
    in_channelSdp channel,
    in_plugKernel plug,
    in_streamWriter writer,
    in_endpointDiscoveryData discoveryData,
    in_objectKind kind,
    in_objectDeinitFunc deinit);

static void
in_channelSdpWriterDeinit(
    in_object _this);

static void*
in_channelSdpWriterRun(
    in_runnable _this);

static void
in_channelSdpWriterTrigger(
    in_runnable _this);

static void
in_channelSdpWriterParticipantAction (
    in_runnable _this,
    v_state sampleState,
    v_state instanceState,
    v_message msg,
    struct v_participantInfo *data);

static void
in_channelSdpWriterSubscriptionAction (
    in_runnable _this,
    v_state sampleState,
    v_state instanceState,
    v_message msg,
    struct v_subscriptionInfo *data);

static void
in_channelSdpWriterPublicationAction (
    in_runnable _this,
    v_state sampleState,
    v_state instanceState,
    v_message msg,
    struct v_publicationInfo *data);

static void
in_channelSdpWriterSendParticipantsData(
    in_channelSdpWriter _this,
    in_connectivityAdmin connAdmin);

static void
in_channelSdpWriterPeriodicAction (
    in_runnable _this);

static void
in_channelSdpWriterSendParticipantsHeartbeat(
    in_channelSdpWriter _this,
    in_connectivityAdmin connAdmin);

static void
in_channelSdpWriterSendRequestedDiscoveryInformation(
    in_channelSdpWriter _this,
    in_connectivityAdmin connAdmin);

static void
in_channelSdpWriterAcknackHeartbeats(
    in_channelSdpWriter _this,
    in_connectivityAdmin connAdmin);

static in_locator
in_channelSdpWriterGetEntityLocator(
    in_connectivityPeerEntity entity);

static in_locator
in_channelSdpWriterFindDefaultUDPV4Locator(
    in_connectivityPeerEntity entity);

static in_locator
in_channelSdpWriterFindUDPV4Locator(
    Coll_List* locators);

static os_uint32
in_channelSdpWriterOnNewGroup(
    u_dispatcher _this,
    os_uint32 event,
    c_voidp userData);

static void
in_channelSdpWriterOnNewGroupAction(
    v_entity e,
    c_voidp arg);

static void
in_channelSdpWriterProcessGroupEvent(
    v_entity entity,
    void* arg);

static v_networkPartitionId
in_channelSdpWriterGetBestFitPartition(
    in_channelSdpWriter _this,
    const os_char* dcpsPartitionName,
    const os_char* dcpsTopicName);

static void
in_channelSdpWriterFillNewGroups(
    v_entity e,
    c_voidp arg);

struct in_channelSdpWriterGroupActionArg
{
    in_channelSdpWriter writer;
    v_group group;
};

struct in_channelSdpWriterEndpointHasKeyActionArg
{
    v_gid gid;
    os_boolean hasKey; /*Out param*/
};


static void
in_channelSdpWriterEndpointHasKey(
    v_entity entity,
    c_voidp args);

typedef enum in_channelSdpWriterEntityAction_e
{
    ACTION_NEW,
    ACTION_NEW_DELETED,
    ACTION_DELETED
} in_channelSdpWriterEntityAction;

OS_CLASS(in_channelSdpWriterHeartbeatInfo);
OS_STRUCT(in_channelSdpWriterHeartbeatInfo)
{
    OS_STRUCT(in_ddsiEntityId) readerId;
    OS_STRUCT(in_ddsiEntityId) writerId;
    OS_STRUCT(in_ddsiSequenceNumber) firstSN;
    OS_STRUCT(in_ddsiSequenceNumber) lastSN;
    in_ddsiGuidPrefix sourceGuidPrefix;
};

OS_CLASS(in_channelSdpWriterAckNackInfo);
OS_STRUCT(in_channelSdpWriterAckNackInfo)
{
    in_ddsiGuidPrefix sourceGuidPrefix;
    OS_STRUCT(in_ddsiEntityId) readerId;
    OS_STRUCT(in_ddsiSequenceNumber) lastSeqNr;
};

OS_STRUCT(in_channelSdpWriter)
{
    OS_EXTENDS(in_channelWriter);
    OS_STRUCT(in_clientMonitor) monitor;
    in_streamWriter streamWriter;
    in_endpointDiscoveryData discoveryData;
    Coll_List discoveredPeers;
    Coll_List heartbeatEvents;
    Coll_List acknacks;
    os_mutex mutex; /* protects discoveredPeers & ackPeers & heartbeatEvents & acknacks lists */
    in_plugKernel plug;
};

in_channelSdpWriter
in_channelSdpWriterNew(
    in_channelSdp sdp,
    in_plugKernel plug,
    in_streamWriter writer,
    in_endpointDiscoveryData discoveryData)
{
    in_channelSdpWriter _this = NULL;
    os_boolean success;

    assert(plug);
    assert(sdp);
    assert(writer);

    _this = os_malloc(sizeof(OS_STRUCT(in_channelSdpWriter)));
    if(_this != NULL)
    {
        success = in_channelSdpWriterInit(_this, sdp, plug, writer,
            discoveryData, IN_OBJECT_KIND_SDP_WRITER,in_channelSdpWriterDeinit);

        if(!success)
        {
            os_free(_this);
            _this = NULL;
            IN_TRACE_1(Construction, 2, "in_channelSdpWriter creation failed = %x", _this);
        } else
        {
            IN_TRACE_1(Construction, 2, "in_channelSdpWriter created = %x", _this);
        }
    }
    return _this;
}

os_boolean
in_channelSdpWriterInit(
    in_channelSdpWriter _this,
    in_channelSdp channel,
    in_plugKernel plug,
    in_streamWriter writer,
    in_endpointDiscoveryData discoveryData,
    in_objectKind kind,
    in_objectDeinitFunc deinit)
{
    os_boolean success;
    c_time periodic = {1, 0};/* wake up every second */
    os_mutexAttr mutexAttr;
    os_result resultmutex;
    u_participant participant;

    assert(_this);
    assert(kind < IN_OBJECT_KIND_COUNT);
    assert(kind > IN_OBJECT_KIND_INVALID);
    assert(deinit);

    participant = u_participant(in_plugKernelGetService(plug));
    _this->plug = in_plugKernelKeep(plug);

    success = in_channelWriterInit(
        in_channelWriter(_this),
        in_channel(channel),
        kind,
        deinit,
        "in_channelSdpWriter",
        NULL,
        in_channelSdpWriterRun,
        in_channelSdpWriterTrigger);
    if(success)
    {
        _this->streamWriter = in_streamWriterKeep(writer);
        _this->discoveryData = in_endpointDiscoveryDataKeep(discoveryData);
        Coll_List_init(&_this->discoveredPeers);
        Coll_List_init(&_this->heartbeatEvents);
        Coll_List_init(&_this->acknacks);

        mutexAttr.scopeAttr = OS_SCOPE_PRIVATE;
        resultmutex = os_mutexInit(&(_this->mutex), &mutexAttr);
        if(resultmutex != os_resultSuccess)
        {
            success = OS_FALSE;
        }
    }
    if(success)
    {

        in_clientMonitorInit(
            &_this->monitor,
            in_runnable(_this),
            participant,
            periodic,
            in_channelSdpWriterParticipantAction,
            in_channelSdpWriterSubscriptionAction,
            in_channelSdpWriterPublicationAction,
            in_channelSdpWriterPeriodicAction);
    }
    return success;
}

void
in_channelSdpWriterDeinit(
    in_object _this)
{
    u_dispatcher service;
    in_channelSdpWriter w;
    in_connectivityPeerEntity peer;

    w = in_channelSdpWriter(_this);

    assert(_this);
    assert(in_channelSdpWriterIsValid(_this));

    service = u_dispatcher(in_plugKernelGetService(w->plug));
    u_dispatcherRemoveListener(service, in_channelSdpWriterOnNewGroup);

    if (w->discoveryData)
    {
        in_endpointDiscoveryDataFree(w->discoveryData);
    }
    if (w->streamWriter)
    {
        in_streamWriterFree(w->streamWriter);
    }
    if (w->plug)
    {
    	in_plugKernelFree(w->plug);
    }
    if(&w->discoveredPeers)
    {
    	peer = in_connectivityPeerEntity(Coll_List_popBack(&w->discoveredPeers));

    	while(peer)
		{
			in_connectivityPeerEntityFree(peer);
			peer = in_connectivityPeerEntity(Coll_List_popBack(&w->discoveredPeers));
		}

    }
    in_channelWriterDeinit(_this);
    os_mutexDestroy (&(in_channelSdpWriter(_this)->mutex));
}

in_result
in_channelSdpWriterAddPeerEntity(
    in_channelSdpWriter _this,
    in_connectivityPeerEntity entity)
{
    in_result result = IN_RESULT_OK;
    os_uint32 errorCode;

    assert(_this);
    assert(entity);

    IN_TRACE_1(Send, 2, "in_channelSdpWriterAddPeerEntity called for entity %x", entity);

    os_mutexLock(&(_this->mutex));
    errorCode = Coll_List_pushBack(&(_this->discoveredPeers), in_connectivityPeerEntityKeep(entity));
    os_mutexUnlock(&(_this->mutex));
    if(errorCode != COLL_OK)
    {
        result = IN_RESULT_OUT_OF_MEMORY;
    }

    return result;
}

in_result
in_channelSdpWriterAddAckNack(
    in_channelSdpWriter _this,
    in_ddsiAckNack event,
    in_ddsiReceiver receiver)
{
    in_channelSdpWriterAckNackInfo acknack;
    in_result result = IN_RESULT_OK;
    os_uint32 errorCode;

    assert(_this);
    assert(event);
    assert(receiver);

    IN_TRACE_1(Send, 2, "in_channelSdpWriterAddAckNack called for event %x", event);

    acknack = os_malloc(OS_SIZEOF(in_channelSdpWriterAckNackInfo));
    if(acknack)
    {
        memcpy(acknack->sourceGuidPrefix, receiver->sourceGuidPrefix, sizeof(in_ddsiGuidPrefix));
        acknack->readerId = in_ddsiSubmessageAckNack(event)->readerId;
        acknack->lastSeqNr = in_ddsiSubmessageAckNack(event)->readerSNState.bitmapBase;

        os_mutexLock(&(_this->mutex));
        errorCode = Coll_List_pushBack(&_this->acknacks, acknack);
        os_mutexUnlock(&(_this->mutex));
        if(errorCode != COLL_OK)
        {
            result = IN_RESULT_OUT_OF_MEMORY;
        }
    } else
    {
        result = IN_RESULT_OUT_OF_MEMORY;
    }
    return result;
}

in_result
in_channelSdpWriterAddHeartbeatEvent(
    in_channelSdpWriter _this,
    in_ddsiHeartbeat event,
    in_ddsiReceiver receiver)
{
    in_result result = IN_RESULT_OK;
    os_uint32 errorCode;
    in_channelSdpWriterHeartbeatInfo info;

    assert(_this);
    assert(event);
    assert(receiver);

    IN_TRACE_1(Send, 2, "in_channelSdpWriterAddHeartbeatEvent called for event %x", event);
    info = os_malloc(OS_SIZEOF(in_channelSdpWriterHeartbeatInfo));
    if(info)
    {
        info->readerId = in_ddsiSubmessageHeartbeat(event)->readerId;
        info->writerId = in_ddsiSubmessageHeartbeat(event)->writerId;
        info->firstSN = in_ddsiSubmessageHeartbeat(event)->firstSN;
        info->lastSN = in_ddsiSubmessageHeartbeat(event)->lastSN;
        memcpy(info->sourceGuidPrefix, receiver->sourceGuidPrefix, in_ddsiGuidPrefixLength);
        os_mutexLock(&(_this->mutex));
        errorCode = Coll_List_pushBack(&(_this->heartbeatEvents), info);
        os_mutexUnlock(&(_this->mutex));
        if(errorCode != COLL_OK)
        {
            result = IN_RESULT_OUT_OF_MEMORY;
        }
    } else
    {
        result = IN_RESULT_OUT_OF_MEMORY;
    }
    return result;
}

void*
in_channelSdpWriterRun(
    in_runnable runnable)
{
    in_channelSdpWriter _this;
    c_ulong mask = 0;
    u_dispatcher service;

    assert(runnable);

    _this = in_channelSdpWriter(runnable);

    service = u_dispatcher(in_plugKernelGetService(_this->plug));
    u_dispatcherGetEventMask(service, &mask);
    u_dispatcherSetEventMask(service, mask | V_EVENT_NEW_GROUP);
    u_entityAction(u_entity(service), in_channelSdpWriterFillNewGroups, NULL);
    u_dispatcherInsertListener(service, in_channelSdpWriterOnNewGroup, _this);
    in_clientMonitorRun(&_this->monitor);

    return NULL;
}

void
in_channelSdpWriterTrigger(
    in_runnable runnable)
{
    assert(runnable);

    IN_TRACE_1(Send, 2, "in_channelSdpWriterTrigger called %x", runnable);
}

void
in_channelSdpWriterPeriodicAction (
    in_runnable runnable)
{
    in_channelSdpWriter _this;
    in_connectivityAdmin connAdmin;

    assert(runnable);

    IN_TRACE_1(Send, 2, "in_channelSdpWriterPeriodicAction called %x", runnable);

    _this = in_channelSdpWriter(runnable);
    connAdmin = in_connectivityAdminGetInstance();

    in_connectivityAdminLock(connAdmin);

    /* step 1: Send requested discovery information to newly detected peers */
    in_channelSdpWriterSendRequestedDiscoveryInformation(_this, connAdmin);
    /* step 2: send participant heartbeats */
    in_channelSdpWriterSendParticipantsHeartbeat(_this, connAdmin);
    /* step 3: Send acknack for heartbeats */
    in_channelSdpWriterAcknackHeartbeats(_this, connAdmin);
    /* step 4: Send participant data periodically */
    in_channelSdpWriterSendParticipantsData(_this, connAdmin);

    in_connectivityAdminUnlock(connAdmin);
    IN_TRACE_1(Send, 2, "in_channelSdpWriterPeriodicAction call finished for %x", runnable);
}

void
in_channelSdpWriterAcknackHeartbeats(
    in_channelSdpWriter _this,
    in_connectivityAdmin connAdmin)
{
    in_channelSdpWriterHeartbeatInfo event;
    os_uint32 size;
    os_uint32 i;
    in_result result;
    Coll_Set* clientEntities;
    Coll_Iter* iteratorPart;
    in_connectivityParticipantFacade facade;
    in_ddsiGuid partGuid;
    OS_STRUCT(in_ddsiGuid) reader;
    OS_STRUCT(in_ddsiGuid) writer;
    in_ddsiSequenceNumber expectedSN;
    in_connectivityPeerParticipant peerParticipant;
    in_locator destLocator;
    os_uint32 tmpLow;
    OS_STRUCT(in_ddsiEntityId) tmp = UI2ENTITYID(IN_ENTITYID_SEDP_BUILTIN_SUBSCRIPTIONS_READER);
    OS_STRUCT(in_ddsiSequenceNumber) tmpSN;
    OS_STRUCT(in_ddsiSequenceNumberSet) seqSet;
    os_boolean initOk;
    os_boolean canAddSeq = OS_TRUE;

    assert(_this);
    assert(connAdmin);

    os_mutexLock(&(_this->mutex));
    size = Coll_List_getNrOfElements(&(_this->heartbeatEvents));
    IN_TRACE_1(Send, 2, "in_channelSdpWriterAcknackHeartbeats - processing acks for %d received heartbeats", size);
    for(i = 0; i < size; i++)
    {
        event = (in_channelSdpWriterHeartbeatInfo)Coll_List_popBack(&(_this->heartbeatEvents));
        reader.entityId = event->readerId;
        writer.entityId = event->writerId;
        memcpy(writer.guidPrefix, event->sourceGuidPrefix, in_ddsiGuidPrefixLength);
        peerParticipant = in_connectivityAdminGetPeerParticipantUnsafe(connAdmin, event->sourceGuidPrefix);
        if(peerParticipant)
        {
            destLocator = in_channelSdpWriterGetEntityLocator(in_connectivityPeerEntity(peerParticipant));
            if(destLocator)
            {
                if((in_ddsiEntityIdEqual(&event->readerId, &tmp)))
                {
                    expectedSN = in_connectivityPeerParticipantGetLastReaderSNRef(peerParticipant);
                } else
                {
                    expectedSN = in_connectivityPeerParticipantGetLastWriterSNRef(peerParticipant);
                }
                tmpSN = *expectedSN;
                tmpLow = tmpSN.low;
                tmpSN.low++;
                if(tmpSN.low < tmpLow)
                {
                    tmpSN.high++;
                }
                initOk = in_ddsiSequenceNumberSetInit(&seqSet, &tmpSN);
                if(!initOk)
                {
                    IN_REPORT_ERROR("in_channelSdpWriterAcknackHeartbeats", "Initialisation of the sequence number set failed!");
                } else
                {
                    while(in_ddsiSequenceNumberCompare(&(event->lastSN), &tmpSN) != C_LT && canAddSeq)
                    {
                        canAddSeq = in_ddsiSequenceNumberSetAdd(&seqSet, &tmpSN);
                        tmpLow = tmpSN.low;
                        tmpSN.low++;
                        if(tmpSN.low < tmpLow)
                        {
                            tmpSN.high++;
                        }
                    }
                    clientEntities = in_connectivityAdminGetParticipants(connAdmin);
                    iteratorPart = Coll_Set_getFirstElement(clientEntities);
                    while(iteratorPart)
                    {
                        facade = in_connectivityParticipantFacade(Coll_Iter_getObject(iteratorPart));
                        /* Fill out the writer.guidPrefix*/
                        partGuid = in_connectivityEntityFacadeGetGuid(in_connectivityEntityFacade(facade));
                        memcpy(reader.guidPrefix, partGuid->guidPrefix, in_ddsiGuidPrefixLength);
                        result = in_streamWriterAppendAckNack(
                            _this->streamWriter,
                            &reader,
                            &writer,
                            &seqSet,
                            destLocator);
                        if(result != IN_RESULT_OK)
                        {
                            IN_REPORT_ERROR("in_channelSdpWriterAcknackHeartbeats", "Failed to transmit acknack for a heartbeat due to a stream error!");
                        }
                        in_streamWriterFlushSingle(_this->streamWriter, destLocator);
                        iteratorPart = Coll_Iter_getNext(iteratorPart);
                    }
                }
                in_locatorFree(destLocator);
            } else
            {
                IN_REPORT_ERROR("in_channelSdpWriterAcknackHeartbeats", "Unable to find a suitable UDPV4 locator, unable to transmit acknack.");
            }
            in_connectivityPeerParticipantFree(peerParticipant);
        }
        os_free(event);
    }
    os_mutexUnlock(&(_this->mutex));
}

void
in_channelSdpWriterSendRequestedDiscoveryInformation(
    in_channelSdpWriter _this,
    in_connectivityAdmin connAdmin)
{
    in_channelSdpWriterAckNackInfo acknack;
    Coll_List tempList;/* on stack definition */
    os_uint32 size;
    os_uint32 i;
    in_connectivityPeerParticipant peer;
    in_locator locator;
    Coll_Set* clientEntities;
    Coll_Iter* iterator;
    in_result result;
    OS_STRUCT(in_ddsiEntityId) tmp = UI2ENTITYID(IN_ENTITYID_SEDP_BUILTIN_SUBSCRIPTIONS_READER);
    OS_STRUCT(in_ddsiEntityId) tmp2 = UI2ENTITYID(IN_ENTITYID_SEDP_BUILTIN_PUBLICATIONS_READER);

    assert(_this);
    assert(connAdmin);

    os_mutexLock(&(_this->mutex));
    Coll_List_init(&tempList);
    size = Coll_List_getNrOfElements(&_this->acknacks);
    IN_TRACE_1(Send,2,"in_channelSdpWriterSendRequestedDiscoveryInformation: processing %x acknacks", size);
    for(i = 0; i < size; i++)
    {
        acknack = (in_channelSdpWriterAckNackInfo)Coll_List_popBack(&(_this->acknacks));
        peer = in_connectivityAdminGetPeerParticipantUnsafe(connAdmin, acknack->sourceGuidPrefix);
        if(peer)
        {
            /* step 1.1: Get a unicast UDPV4 locator, or if not available the
             * UDPV4 multicast locator, or if that is not available then fall back
             * to the default locators of the owning participant.
             */
            locator = in_channelSdpWriterGetEntityLocator(in_connectivityPeerEntity(peer));
            if(locator)
            {
                Coll_List_pushBack(&tempList, locator);
                if(in_ddsiEntityIdEqual(&acknack->readerId,&tmp))
                {
                    /* step 1.2: Get all local reader entity information, so we can send
                     * this  to the locator found in the previous step
                     */
                    clientEntities = in_connectivityAdminGetReaders(connAdmin);
                    iterator = Coll_Set_getFirstElement(clientEntities);
                    while(iterator)
                    {
                        in_connectivityReaderFacade facade;
                        facade = in_connectivityReaderFacade(Coll_Iter_getObject(iterator));
                        if(in_ddsiSequenceNumberCompare(in_connectivityReaderFacadeGetSequenceNumber(facade), &acknack->lastSeqNr) != C_LT)
                        {
                            result = in_streamWriterAppendReaderData(
                                    in_streamWriter(_this->streamWriter),
                                    acknack->sourceGuidPrefix,
                                    _this->discoveryData,
                                    facade,
                                    &tempList);
                            if(result != IN_RESULT_OK)
                            {
                                IN_REPORT_ERROR("in_channelSdpWriterSendRequestedDiscoveryInformation", "Unable to transmit reader data due to a stream error.");
                            }
                        }
                        iterator = Coll_Iter_getNext(iterator);
                    }
                } else if(in_ddsiEntityIdEqual(&acknack->readerId,&tmp2))
                {
                    /* step 1.3: Get all local writer entity information, so we can send
                     * this  to the locator found in the step 1.1
                     */
                    clientEntities = in_connectivityAdminGetWriters(connAdmin);
                    iterator = Coll_Set_getFirstElement(clientEntities);
                    while(iterator)
                    {
                        in_connectivityWriterFacade facade;
                        facade = in_connectivityWriterFacade(Coll_Iter_getObject(iterator));
                        if(in_ddsiSequenceNumberCompare(in_connectivityWriterFacadeGetSequenceNumber(facade), &acknack->lastSeqNr) != C_LT)
                        {
                            result = in_streamWriterAppendWriterData(
                                    in_streamWriter(_this->streamWriter),
                                    _this->discoveryData,
                                    facade,
                                    &tempList);
                            if(result != IN_RESULT_OK)
                            {
                                IN_REPORT_ERROR("in_channelSdpWriterSendRequestedDiscoveryInformation", "Unable to transmit writer data due to a stream error.");
                            }
                        }
                        iterator = Coll_Iter_getNext(iterator);
                    }
                } else
                {
                    IN_REPORT_ERROR("in_channelSdpWriterSendRequestedDiscoveryInformation", "Unable to match entityId to a known reader or writer, unable to transmit reader/writer data.");
                }
                /* step 1.4: Flush the stream writer */
                in_streamWriterFlush(
                    in_streamWriter(_this->streamWriter),
                    &tempList);
                /* clear the list */
                Coll_List_popBack(&tempList);
                in_locatorFree(locator);
            } else
            {
                IN_REPORT_ERROR("in_channelSdpWriterSendRequestedDiscoveryInformation", "Unable to find a suitable UDPV4 locator, unable to transmit reader/writer data.");
            }
            in_connectivityPeerParticipantFree(peer);
        } else
        {
            IN_REPORT_WARNING("in_channelSdpWriterSendRequestedDiscoveryInformation", "Unable to find a matching peer participant for the received guid. unable to transmit reader/writer data, ignoring request.");
        }
        os_free(acknack);
    }
    os_mutexUnlock(&(_this->mutex));
}

void
in_channelSdpWriterSendParticipantsData(
    in_channelSdpWriter _this,
    in_connectivityAdmin connAdmin)
{
    in_locator locator;
    Coll_List tempList;
    Coll_Set* clientEntities;
    Coll_Iter* iterator;
    in_connectivityParticipantFacade facade;
    in_result result;
    Coll_Set* peers;
    Coll_Iter* peerIterator;
    in_connectivityPeerParticipant peer;
    in_ddsiGuidPrefixRef destGuidPrefix;

    assert(_this);
    assert(connAdmin);

    Coll_List_init(&tempList);
    locator = in_streamWriterGetDataMulticastLocator(_this->streamWriter);
    assert(locator);
    Coll_List_pushBack(&tempList, locator);
    clientEntities = in_connectivityAdminGetParticipants(connAdmin);
    IN_TRACE_1(Send, 2, "in_channelSdpWriterSendParticipantsData - sending data messages for %d participants", Coll_Set_getNrOfElements(clientEntities));
    iterator = Coll_Set_getFirstElement(clientEntities);
    while(iterator)
    {
        facade = in_connectivityParticipantFacade(Coll_Iter_getObject(iterator));
        result = in_streamWriterAppendParticipantData(
                in_streamWriter(_this->streamWriter),
                _this->discoveryData,
                facade,
                &tempList);
        if(result != IN_RESULT_OK)
        {
            IN_REPORT_ERROR("in_channelSdpWriterSendParticipantsData", "Unable to transmit participant data due to a stream error.");
        } else
        {
            peers = in_connectivityAdminGetPeerParticipantsUnsafe(connAdmin);
            peerIterator = Coll_Set_getFirstElement(peers);
            while(peerIterator)
            {                peer = in_connectivityPeerParticipant(Coll_Iter_getObject(peerIterator));
                /* Get a unicast UDPV4 locator, or if not available the
                 * UDPV4 multicast locator, or if that is not available then fall back
                 * to the default locators of the owning participant.
                 */
                locator = in_channelSdpWriterGetEntityLocator(in_connectivityPeerEntity(peer));
                if(locator)
                {
                    destGuidPrefix = in_connectivityPeerParticipantGetGuidPrefix(peer);
                    result = in_streamWriterAppendParticipantMessage(
                        in_streamWriter(_this->streamWriter),
                        facade,
                        destGuidPrefix,
                        locator);
                    if(result != IN_RESULT_OK)
                    {
                        IN_REPORT_ERROR("in_channelSdpWriterSendParticipantsData", "Unable to transmit participant message data due to a stream error.");
                    }
                    in_locatorFree(locator);
                } else
                {
                    IN_REPORT_ERROR("in_channelSdpWriterSendParticipantsData", "Unable to find a suitable UDPV4 locator, unable to transmit participant message data.");
                }
                peerIterator = Coll_Iter_getNext(peerIterator);
            }
        }
        iterator = Coll_Iter_getNext(iterator);

    }
    locator = Coll_List_popBack(&tempList);
    in_locatorFree(locator);
}

void
in_channelSdpWriterSendParticipantsHeartbeat(
    in_channelSdpWriter _this,
    in_connectivityAdmin connAdmin)
{
    in_result result;
    in_ddsiGuidPrefixRef sourceGuidPrefix;
    in_ddsiGuidPrefixRef destGuidPrefix;
    OS_STRUCT(in_ddsiEntityId) pubReaderId = UI2ENTITYID(IN_ENTITYID_SEDP_BUILTIN_PUBLICATIONS_READER);
    OS_STRUCT(in_ddsiEntityId) pubWriterId = UI2ENTITYID(IN_ENTITYID_SEDP_BUILTIN_PUBLICATIONS_WRITER);
    OS_STRUCT(in_ddsiEntityId) subReaderId = UI2ENTITYID(IN_ENTITYID_SEDP_BUILTIN_SUBSCRIPTIONS_READER);
    OS_STRUCT(in_ddsiEntityId) subWriterId = UI2ENTITYID(IN_ENTITYID_SEDP_BUILTIN_SUBSCRIPTIONS_WRITER);
    OS_STRUCT(in_ddsiEntityId) partReaderId = UI2ENTITYID(IN_ENTITYID_P2P_BUILTIN_PARTICIPANT_MESSAGE_READER);
    OS_STRUCT(in_ddsiEntityId) partWriterId = UI2ENTITYID(IN_ENTITYID_P2P_BUILTIN_PARTICIPANT_MESSAGE_WRITER);
    OS_STRUCT(in_ddsiSequenceNumber) nilSN = {0,0};
    OS_STRUCT(in_ddsiSequenceNumber) firstSN = {0,1};
    in_ddsiSequenceNumber lastSN;
    in_locator singleDestination;
    Coll_Set* clientEntities;
    Coll_Iter* iterator;
    in_connectivityParticipantFacade facade;
    in_ddsiGuid tmpGuid;
    static os_uint32 partCount = 0;
    static os_uint32 writerCount = 0;
    static os_uint32 readerCount = 0;
    Coll_Set* participantPeers;
    Coll_Iter* peerIter;
    in_connectivityPeerParticipant peer;

    assert(_this);
    assert(connAdmin);

    clientEntities = in_connectivityAdminGetParticipants(connAdmin);
    IN_TRACE_1(Send, 2, "in_channelSdpWriterSendParticipantsHeartbeat - sending heartbeat messages for %d participant(s)", Coll_Set_getNrOfElements(clientEntities));
    iterator = Coll_Set_getFirstElement(clientEntities);
    while(iterator)
    {
        facade = in_connectivityParticipantFacade(Coll_Iter_getObject(iterator));
        tmpGuid = in_connectivityEntityFacadeGetGuid(in_connectivityEntityFacade(facade));
        sourceGuidPrefix = tmpGuid->guidPrefix;
        participantPeers = in_connectivityParticipantFacadeGetMatchedPeerParticipantsUnsafe(facade);
        peerIter = Coll_Set_getFirstElement(participantPeers);
        while(peerIter)
        {
            peer = in_connectivityPeerParticipant(Coll_Iter_getObject(peerIter));
            destGuidPrefix = in_connectivityPeerParticipantGetGuidPrefix(peer);
            singleDestination = in_channelSdpWriterGetEntityLocator(in_connectivityPeerEntity(peer));
            if(!singleDestination)
            {
                IN_REPORT_ERROR("in_channelSdpWriterSendParticipantsHeartbeat", "Unable to find a suitable UDPV4 locator, unable to transmit heartbeat data.");
            } else
            {
                lastSN = in_connectivityParticipantFacadeGetNrOfWriters(facade);
                if(in_ddsiSequenceNumberCompare(lastSN, &nilSN) == C_GT)
                {
                    writerCount++;
                    /* Send out publication info */
                    result = in_streamWriterAppendHeartbeat(
                        _this->streamWriter,
                        sourceGuidPrefix,
                        destGuidPrefix,
                        &pubReaderId,
                        &pubWriterId,
                        &firstSN,
                        lastSN,
                        writerCount,
                        singleDestination);
                    if(result != IN_RESULT_OK)
                    {
                         IN_REPORT_ERROR("in_channelSdpWriterSendParticipantsHeartbeat", "Unable to transmit heartbeat for publications reader/writer due to a stream error.");
                    }
                }
                lastSN = in_connectivityParticipantFacadeGetNrOfReaders(facade);
                if(in_ddsiSequenceNumberCompare(lastSN, &nilSN) == C_GT)
                {
                    readerCount++;
                    /* Send out subscription info */
                    result = in_streamWriterAppendHeartbeat(
                        _this->streamWriter,
                        sourceGuidPrefix,
                        destGuidPrefix,
                        &subReaderId,
                        &subWriterId,
                        &firstSN,
                        lastSN,
                        readerCount,
                        singleDestination);
                    if(result != IN_RESULT_OK)
                    {
                         IN_REPORT_ERROR("in_channelSdpWriterSendParticipantsHeartbeat", "Unable to transmit heartbeat for subscriptions reader/writer due to a stream error.");
                    }
                }
                partCount++;
                /* Send out publication info */
                result = in_streamWriterAppendHeartbeat(
                    _this->streamWriter,
                    sourceGuidPrefix,
                    destGuidPrefix,
                    &partReaderId,
                    &partWriterId,
                    &firstSN,
                    &nilSN,
                    partCount,
                    singleDestination);

                if(result == IN_RESULT_OK)
                {
                	in_streamWriterFlushSingle(_this->streamWriter, singleDestination);
                } else
                {
                     IN_REPORT_ERROR(
                    	"in_channelSdpWriterSendParticipantsHeartbeat",
                    	"Unable to transmit heartbeat for P2P participant message reader/writer due to a stream error.");
                }
                in_locatorFree(singleDestination);
            }
            peerIter = Coll_Iter_getNext(peerIter);
        }
        iterator = Coll_Iter_getNext(iterator);
    }

}

/*******************************************************************************
 ******************** operations for handling discovery info *******************
 ******************************************************************************/

in_channelSdpWriterEntityAction
in_channelSdpWriterDetermineEntityAction(
    v_state sampleState,
    v_state instanceState)
{
    in_channelSdpWriterEntityAction action;

    if(v_stateTest(instanceState, L_DISPOSED))
    {
        if((v_stateTest(sampleState, L_NEW) ))
        {
            action = ACTION_NEW_DELETED;
        } else
        {
            action = ACTION_DELETED;
        }
    } else if((v_stateTest(sampleState, L_NEW)) || v_stateTest(sampleState, L_READ))
    {
        action = ACTION_NEW;
    } else
    {
    	action = ACTION_DELETED;
    }

    return action;
}

in_locator
in_channelSdpWriterGetEntityLocator(
    in_connectivityPeerEntity entity)
{
    Coll_List* locators;
    in_locator locator = NULL;

    assert(entity);

    /* step 1: Try to find a suitable unicast locator */
    locators = in_connectivityPeerEntityGetUnicastLocators(entity);
    locator = in_channelSdpWriterFindUDPV4Locator(locators);
    /* step 2: if we were unable to locate a suitable unicast locator, try to
     * find a suitable multicast locator
     */
    if(!locator)
    {
        IN_TRACE(Send, 2, "in_channelSdpWriterGetEntityLocator - No unicast locator found, falling back to multicast");
        locators = in_connectivityPeerEntityGetMulticastLocators(entity);
        locator = in_channelSdpWriterFindUDPV4Locator(locators);
    }
    /* if no unicast or multicast locator could be found, then fall back to the
     * default locator registered with the owning participant
     */
    if(!locator)
    {
        IN_TRACE(Send, 2, "in_channelSdpWriterGetEntityLocator - No multicast locator found, falling back to default locators");
        locator = in_channelSdpWriterFindDefaultUDPV4Locator(entity);
    }

    return locator;
}

in_locator
in_channelSdpWriterFindUDPV4Locator(
    Coll_List* locators)
{
    Coll_Iter* iterator;
    in_locator locator = NULL;
    in_locator tmpLocator;

    assert(locators);

    /* try to find a suitable locator */
    iterator = Coll_List_getFirstElement(locators);
    while(iterator && !locator)
    {
        tmpLocator = in_locator(Coll_Iter_getObject(iterator));
        if(in_locatorGetKind(tmpLocator) == IN_LOCATOR_KIND_UDPV4)
        {
            if(!locator)
            {
                locator = tmpLocator;
            }
        }
        iterator = Coll_Iter_getNext(iterator);
    }
    if(locator)
    {
        locator = in_locatorKeep(locator);
    }
    return locator;
}

in_locator
in_channelSdpWriterFindDefaultUDPV4Locator(
    in_connectivityPeerEntity entity)
{
    in_locator locator = NULL;
    in_ddsiGuid guid;
    in_connectivityAdmin connAdmin;
    in_connectivityPeerParticipant peerParticipant = NULL;
    in_objectKind kind;
    Coll_List* locators;

    assert(entity);

    /* step 1: locate the corresponding peer participant for the peer entity */
    connAdmin = in_connectivityAdminGetInstance();
    kind = in_objectGetKind(in_object(entity));
    switch (kind)
    {
        case IN_OBJECT_KIND_PEER_PARTICIPANT:
            peerParticipant = in_connectivityPeerParticipantKeep(entity);
            break;
        case IN_OBJECT_KIND_PEER_WRITER:
            guid = in_connectivityPeerWriterGetGuid((in_connectivityPeerWriter)entity);
            peerParticipant = in_connectivityAdminGetPeerParticipantUnsafe(connAdmin, guid->guidPrefix);
            break;
        case IN_OBJECT_KIND_PEER_READER:
            guid = in_connectivityPeerReaderGetGuid((in_connectivityPeerReader)entity);
            peerParticipant = in_connectivityAdminGetPeerParticipantUnsafe(connAdmin, guid->guidPrefix);
            break;
        default:
            assert(FALSE);
    }
    /* step 2: If we located the peer participant, then fetch the default unicast
     * locator, or if thats not available try to find the default multicast locator
     */
    if(peerParticipant)
    {
        locators = in_connectivityPeerParticipantGetDefaultUnicastLocators(peerParticipant);
        locator = in_channelSdpWriterFindUDPV4Locator(locators);
        if(!locator)
        {
            locators = in_connectivityPeerParticipantGetDefaultMulticastLocators(peerParticipant);
            locator = in_channelSdpWriterFindUDPV4Locator(locators);
        }
        in_connectivityPeerParticipantFree(peerParticipant);
    }
    return locator;
}

void
in_channelSdpWriterParticipantAction (
    in_runnable _this,
    v_state sampleState,
    v_state instanceState,
    v_message msg,
    struct v_participantInfo *data)
{
    in_result result = IN_RESULT_OK;
    in_connectivityAdmin connAdmin;
    in_connectivityParticipantFacade facade;
    in_channelSdpWriterEntityAction action;
    in_channelSdpWriter sdpWriter;

    assert(_this);
    assert(msg);
    assert(data);

    IN_TRACE_3(Send,2,"in_channelSdpWriterParticipantAction %x (%x,%x)",_this, data->key.systemId,data->key.localId);

    sdpWriter = in_channelSdpWriter(_this);
    action = in_channelSdpWriterDetermineEntityAction(sampleState, instanceState);
    connAdmin = in_connectivityAdminGetInstance();
    /* step 1: if participant is new or modified then add/update the participant
     */
    if(action == ACTION_NEW)
    {
        result = in_connectivityAdminAddParticipant(connAdmin, data);
    }
    /* step 2: notified all interest parties of the participant status, however
     * if the participant was already deleted, then we wont inform this at all
     * and just ignore it
     */
    if(action != ACTION_NEW_DELETED && result == IN_RESULT_OK)
    {

        facade = in_connectivityAdminGetParticipant(connAdmin, data);
        if(facade)
        {
            in_result result;
            Coll_List tempList;
            os_uint32 errorCode;
            in_locator locator;

            Coll_List_init(&tempList);
            locator = in_streamWriterGetDataMulticastLocator(sdpWriter->streamWriter);
            errorCode = Coll_List_pushBack(&tempList, locator);
            if(errorCode != COLL_OK)
            {
                IN_REPORT_ERROR("in_channelSdpWriterParticipantAction", "Unable to transmit participant data, out of memory error when adding locator to locator list.");
            } else
            {
                IN_TRACE_3(Send,2,"in_channelSdpWriterParticipantAction - sending participant data %x (%x,%x)",_this, data->key.systemId,data->key.localId);
                result = in_streamWriterAppendParticipantData(
                        in_streamWriter(sdpWriter->streamWriter),
                        sdpWriter->discoveryData,
                        facade,
                        &tempList);
                if(result != IN_RESULT_OK)
                {
                    IN_REPORT_ERROR("in_channelSdpWriterParticipantAction", "Unable to transmit participant data due to a stream error.");
                }
                in_streamWriterFlush(
                    in_streamWriter(sdpWriter->streamWriter),
                    &tempList);
                /* clear the list */
                locator = in_locator(Coll_List_popBack(&tempList));
                in_locatorFree(locator);
            }
            in_connectivityParticipantFacadeFree(facade);
        } /* else no one interested */
    }
    /* step 3: if the participant was deleted, we can remove it from the
     * connectivity admin after we notified all interested parties
     */
    if(action == ACTION_DELETED)
    {
        result = in_connectivityAdminRemoveParticipant(connAdmin, data);
    }
}

void
in_channelSdpWriterSubscriptionAction (
    in_runnable _this,
    v_state sampleState,
    v_state instanceState,
    v_message msg,
    struct v_subscriptionInfo *data)
{
    in_result result;
    in_connectivityAdmin connAdmin;
    in_channelSdpWriterEntityAction action;
    in_channelSdpWriter sdpWriter;
    struct in_channelSdpWriterEndpointHasKeyActionArg arg;

    assert(_this);
    assert(msg);
    assert(data);

    IN_TRACE_3(Send,2,"in_channelSdpWriterSubscriptionAction %x (%x,%x)",_this, data->key.systemId,data->key.localId);

    sdpWriter = in_channelSdpWriter(_this);
    action = in_channelSdpWriterDetermineEntityAction(sampleState, instanceState);
    connAdmin = in_connectivityAdminGetInstance();
    if(action == ACTION_NEW)
    {
        /* step 1: if participant is new or modified then add/update the participant
         */
        arg.gid = data->key;
        arg.hasKey = OS_FALSE;

        u_entityAction(u_entity(in_plugKernelGetService(sdpWriter->plug)),
                in_channelSdpWriterEndpointHasKey, &arg);
        result = in_connectivityAdminAddReader(connAdmin, data, arg.hasKey);

        if(result != IN_RESULT_OK)
        {
            IN_REPORT_ERROR("in_channelSdpWriterSubscriptionAction", "Unable to add a newly detected reader to the connectvity administration.");
        }
    } else if(action == ACTION_DELETED)
    {
        /* step 2: if the participant was deleted, we can remove it from the
         * connectivity admin after we notified all interested parties
         */
        result = in_connectivityAdminRemoveReader(connAdmin, data);
        if(result != IN_RESULT_OK)
        {
            IN_REPORT_ERROR("in_channelSdpWriterSubscriptionAction", "Unable to remove a reader to the connectvity administration.");
        }
    }
}

void
in_channelSdpWriterPublicationAction (
    in_runnable _this,
    v_state sampleState,
    v_state instanceState,
    v_message msg,
    struct v_publicationInfo *data)
{
    in_result result;
    in_connectivityAdmin connAdmin;
    in_channelSdpWriterEntityAction action;
    in_channelSdpWriter sdpWriter;
    struct in_channelSdpWriterEndpointHasKeyActionArg arg;

    assert(_this);
    assert(msg);
    assert(data);

    IN_TRACE_3(Send,2,"in_channelSdpWriterPublicationAction %x (%x,%x)",_this, data->key.systemId,data->key.localId);
    IN_TRACE_3(Send,2,"in_channelSdpWriterPublicationAction %x part(%x,%x)",_this,data->participant_key.systemId,data->participant_key.localId);

    sdpWriter = in_channelSdpWriter(_this);
    action = in_channelSdpWriterDetermineEntityAction(sampleState, instanceState);
    connAdmin = in_connectivityAdminGetInstance();

    if(action == ACTION_NEW)
    {
        /* step 1: if participant is new or modified then add/update the participant
         */
        arg.gid = data->key;
        arg.hasKey = OS_FALSE;

        u_entityAction(u_entity(in_plugKernelGetService(sdpWriter->plug)),
                in_channelSdpWriterEndpointHasKey, &arg);
        result = in_connectivityAdminAddWriter(connAdmin, data, arg.hasKey);

        if(result != IN_RESULT_OK)
        {
            IN_REPORT_ERROR("in_channelSdpWriterSubscriptionAction", "Unable to add a newly detected writer to the connectvity administration.");
        }
    } else if(action == ACTION_DELETED)
    {
        /* step 2: if the participant was deleted, we can remove it from the
         * connectivity admin after we notified all interested parties
         */
        result = in_connectivityAdminRemoveWriter(connAdmin, data);
        if(result != IN_RESULT_OK)
        {
            IN_REPORT_ERROR("in_channelSdpWriterSubscriptionAction", "Unable to remove a writer to the connectvity administration.");
        }
    }
}

/*******************************************************************************
 ********************* operations for handling new groups **********************
 ******************************************************************************/

os_uint32
in_channelSdpWriterOnNewGroup(
    u_dispatcher _this,
    os_uint32 event,
    c_voidp userData)
{
    u_result result;
    u_service service = u_service(_this);

    assert(_this);
    assert(userData);

    IN_TRACE_2(Send, 3, "in_channelSdpWriterOnNewGroup %x (event %d)", _this, event);

    if (userData && ((event == V_EVENT_NEW_GROUP) || (event == V_EVENT_UNDEFINED)))
    {
        result = u_entityAction(
            u_entity(service),
            in_channelSdpWriterOnNewGroupAction,
            userData);
    }
    return event;
}

void
in_channelSdpWriterOnNewGroupAction(
    v_entity e,
    c_voidp arg)
{
    in_channelSdpWriter _this;
    v_service kservice;
    c_iter newGroups;
    v_group group;
    struct in_channelSdpWriterGroupActionArg actionArg;

    assert(e);
    assert(arg);

    _this = in_channelSdpWriter(arg);

    kservice = v_service(e);
    newGroups = v_serviceTakeNewGroups(kservice);

    IN_TRACE_1(Send, 4, "Number of new groups to be processed is: %d", c_iterLength(newGroups));

    group = v_group(c_iterTakeFirst(newGroups));
    actionArg.writer = _this;
    while (group)
    {
        actionArg.group = group;
        u_entityAction(
            u_entity(in_plugKernelGetNetworkReader(_this->plug)),
            in_channelSdpWriterProcessGroupEvent,
            &actionArg);
        c_free(group);
        group = v_group(c_iterTakeFirst(newGroups));
    }
    c_iterFree(newGroups);
}

void
in_channelSdpWriterProcessGroupEvent(
    v_entity entity,
    void* arg)
{
    in_channelSdpWriter _this;
    v_networkReader reader;
    v_group group;
    v_networkReaderEntry entry;

    assert(entity);
    assert(arg);

    _this = ((struct in_channelSdpWriterGroupActionArg *)arg)->writer;
    reader = v_networkReader(entity);
    group = ((struct in_channelSdpWriterGroupActionArg *)arg)->group;

    /* step 1: Look up the group, maybe it already exists */
    entry = v_networkReaderLookupEntry(reader, group);
    if(!entry)
    {
        v_networkPartitionId networkPartitionId;
        os_char* serviceName;

        /* step 2: Lookup the best partition ID */
        networkPartitionId = in_channelSdpWriterGetBestFitPartition(
            _this,
            v_partitionName(v_groupPartition(group)),
            v_topicName(v_groupTopic(group)));
        /* Step 3: Create a new entry within the network reader */
        entry = v_networkReaderEntryNew(
            reader,
            group,
            V_NETWORKID_DDSI,
            1,/* must be 1 to allow finalizing the entry, see next step */
            networkPartitionId);
        /* Step 4: Notify the entry to finalize itself, should rewrite the
         * entry to finalize itself upon creation. This code is a tad.. weird
         * now...
         */
        serviceName = u_serviceGetName(in_plugKernelGetService(_this->plug));
        v_networkReaderEntryNotifyConnected(entry, serviceName);
        os_free(serviceName);
    }
    else
    {
        IN_REPORT_ERROR("in_channelSdpWriterProcessGroupEvent", "Unable to lookup the network entry in the networkReader.");
    }
}

v_networkPartitionId
in_channelSdpWriterGetBestFitPartition(
    in_channelSdpWriter _this,
    const os_char* dcpsPartitionName,
    const os_char* dcpsTopicName)
{
    /* TODO select best partition */
    return 1;
}

static void
in_channelSdpWriterFillNewGroups(
    v_entity e,
    c_voidp arg)
{
    v_service service;

    service = v_service(e);
    /* arg parameter unused */

    v_serviceFillNewGroups(service);
}

static void
in_channelSdpWriterEndpointHasKey(
    v_entity entity,
    c_voidp arg)
{
    v_public public;
    v_topic topic;
    c_long size;
    struct in_channelSdpWriterEndpointHasKeyActionArg* result;

    result = (struct in_channelSdpWriterEndpointHasKeyActionArg*)arg;
    public = v_gidClaim(result->gid, v_object(entity)->kernel);

    if(public)
    {
        switch(v_object(public)->kind)
        {
        case K_DATAREADER:
            topic = v_dataReaderGetTopic(v_dataReader(public));
            break;
        case K_WRITER:
            topic = v_writer(public)->topic;
            break;
        default:
            assert(FALSE);
            topic = NULL;
            break;
        }

        if(topic)
        {
           size = c_arraySize(v_topicMessageKeyList(topic));

           if(size)
           {
               result->hasKey = OS_TRUE;
           }
        }
        v_gidRelease(result->gid, v_object(entity)->kernel);
    }
    return;
}
