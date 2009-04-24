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
#include "v_domain.h"
#include "v_topic.h"
#include "v_networkReaderEntry.h"
#include "in_ddsiDefinitions.h"
#include "in_endpointDiscoveryData.h"
#include "in__ddsiReceiver.h"

static os_boolean
in_channelSdpWriterInit(
    in_channelSdpWriter _this,
    in_channelSdp channel,
    in_objectKind kind,
    in_objectDeinitFunc deinit,
    u_participant participant,
    in_streamWriter writer,
    u_networkReader reader,
    in_endpointDiscoveryData discoveryData);

static void
in_channelSdpWriterDeinit(
    in_object _this);

static void*
in_channelSdpWriterRun(
    in_runnable _this);

static void
in_channelSdpWriterTrigger(
    in_runnable runnable);

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

static void
in_channelSdpWriterSendAcks(
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

v_networkPartitionId
in_channelSdpWriterGetBestFitPartition(
    in_channelSdpWriter _this,
    const os_char* dcpsPartitionName,
    const os_char* dcpsTopicName);

static void
fillNewGroups(v_entity e,
              c_voidp arg);

struct in_channelSdpWriterGroupActionArg
{
    in_channelSdpWriter writer;
    v_group group;
};

typedef enum in_channelSdpWriterEntityAction_e
{
    ACTION_NEW,
    ACTION_NEW_DELETED,
    ACTION_DELETED
} in_channelSdpWriterEntityAction;

OS_CLASS(in_channelSdpWriterHeartbeatInfo);
OS_STRUCT(in_channelSdpWriterHeartbeatInfo)
{
    in_locator destination;
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
    Coll_List ackPeers;
    Coll_List heartbeatEvents;
    Coll_List acknacks;
    os_mutex mutex; /* protects discoveredPeers & ackPeers & heartbeatEvents & acknacks lists */
    u_service service;
    u_networkReader reader;
};

in_channelSdpWriter
in_channelSdpWriterNew(
    in_channelSdp sdp,
    u_participant participant,
    in_streamWriter writer,
    u_networkReader reader,
    in_endpointDiscoveryData discoveryData)
{
    in_channelSdpWriter _this = NULL;

    _this = os_malloc(sizeof(OS_STRUCT(in_channelSdpWriter)));
    if(_this != NULL)
    {
        os_boolean success;

        success = in_channelSdpWriterInit(
            _this, sdp,
            IN_OBJECT_KIND_SDP_WRITER,
            in_channelSdpWriterDeinit,
            participant,
            writer,
            reader,
            discoveryData);
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
    in_objectKind kind,
    in_objectDeinitFunc deinit,
    u_participant participant,
    in_streamWriter writer,
    u_networkReader reader,
    in_endpointDiscoveryData discoveryData)
{
    os_boolean success;
    c_time periodic = {1, 0};
    os_mutexAttr mutexAttr;
    os_result resultmutex;

    assert(_this);
    assert(kind < IN_OBJECT_KIND_COUNT);
    assert(kind > IN_OBJECT_KIND_INVALID);
    assert(deinit);

    _this->reader = reader;
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
        Coll_List_init(&_this->ackPeers);
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
    if(success)
    {
        _this->service = u_service(participant);
    }
    return success;
}

void
in_channelSdpWriterDeinit(
    in_object _this)
{
    assert(_this);
    assert(in_channelSdpWriterIsValid(_this));

    u_dispatcherRemoveListener(
        u_dispatcher(in_channelSdpWriter(_this)->service),
        in_channelSdpWriterOnNewGroup);

    if (in_channelSdpWriter(_this)->discoveryData)
    {
        in_endpointDiscoveryDataFree(in_channelSdpWriter(_this)->discoveryData);
    }
    if (in_channelSdpWriter(_this)->streamWriter)
    {
        in_streamWriterFree(in_channelSdpWriter(_this)->streamWriter);
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

    IN_TRACE_1(Send, 2, "in_channelSdpWriterAddPeerEntity called %x", entity);

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

    acknack = os_malloc(OS_SIZEOF(in_channelSdpWriterAckNackInfo));

    memcpy(acknack->sourceGuidPrefix,
             receiver->sourceGuidPrefix,
             sizeof(in_ddsiGuidPrefix));
    acknack->readerId = in_ddsiSubmessageAckNack(event)->readerId;
    acknack->lastSeqNr = in_ddsiSubmessageAckNack(event)->readerSNState.bitmapBase;

    os_mutexLock(&(_this->mutex));
    errorCode = Coll_List_pushBack(&_this->acknacks, acknack);
    os_mutexUnlock(&(_this->mutex));
    if(errorCode != COLL_OK)
    {
        result = IN_RESULT_OUT_OF_MEMORY;
    }
    return result;
}



in_result
in_channelSdpWriterAddPeerEntityForAcking(
    in_channelSdpWriter _this,
    in_discoveredPeer entity)
{
    in_result result = IN_RESULT_OK;
    os_uint32 errorCode;
    in_discoveredPeer tmp;

    assert(_this);
    assert(entity);

    IN_TRACE_1(Send, 2, "in_channelSdpWriterAddPeerEntityForAcking called %x", entity);

    /* We need to create a copy of the in_discoveredPeer object, because it is
     * not a reference counted object.
     */
    tmp = os_malloc(OS_SIZEOF(in_discoveredPeer));
    memcpy(tmp->writerGuidPrefix,
             entity->writerGuidPrefix,
             sizeof(in_ddsiGuidPrefix));
    tmp->writerId = entity->writerId;
    tmp->readerId = entity->readerId;
    tmp->sequenceNumber = entity->sequenceNumber;
    tmp->discoveredPeerEntity = entity->discoveredPeerEntity;

    os_mutexLock(&(_this->mutex));
    errorCode = Coll_List_pushBack(&_this->ackPeers, tmp);
    os_mutexUnlock(&(_this->mutex));
    if(errorCode != COLL_OK)
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

    IN_TRACE_1(Send, 2, "in_channelSdpWriterAddHeartbeatEvent called %x", event);
    info = os_malloc(OS_SIZEOF(in_channelSdpWriterHeartbeatInfo));
    if(info)
    {
        info->readerId = in_ddsiSubmessageHeartbeat(event)->readerId;
        info->writerId = in_ddsiSubmessageHeartbeat(event)->writerId;
        info->firstSN = in_ddsiSubmessageHeartbeat(event)->firstSN;
        info->lastSN = in_ddsiSubmessageHeartbeat(event)->lastSN;
        memcpy(info->sourceGuidPrefix, receiver->sourceGuidPrefix, in_ddsiGuidPrefixLength);
        if(Coll_List_getNrOfElements(&receiver->unicastReplyLocatorList) > 0)
        {
            info->destination = in_locatorKeep(in_locator(Coll_List_getObject(&receiver->unicastReplyLocatorList, 0)));
        } else if(Coll_List_getNrOfElements(&receiver->multicastReplyLocatorList) > 0)
        {
            info->destination = in_locatorKeep(in_locator(Coll_List_getObject(&receiver->multicastReplyLocatorList, 0)));
        } else
        {
            assert(FALSE);
            /* TODO report error */
        }
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

    assert(runnable);

    _this = in_channelSdpWriter(runnable);

    u_dispatcherGetEventMask(u_dispatcher(_this->service), &mask);
    u_dispatcherSetEventMask(u_dispatcher(_this->service), mask | V_EVENT_NEW_GROUP);
    u_entityAction(u_entity(_this->service), fillNewGroups, NULL);

    u_dispatcherInsertListener(
        u_dispatcher(_this->service),
        in_channelSdpWriterOnNewGroup,
        _this);

    in_clientMonitorRun(&_this->monitor);
    return NULL;
}

void
in_channelSdpWriterTrigger(
    in_runnable runnable)
{
    in_channelSdpWriter _this;

    assert(runnable);

    IN_TRACE_1(Send, 2, "in_channelSdpWriterTrigger called %x", runnable);

    _this = in_channelSdpWriter(runnable);
    /* TODO tbd, currently not needed */
}

void
in_channelSdpWriterPeriodicAction (
    in_runnable runnable)
{
    in_channelSdpWriter _this;
    in_connectivityAdmin connAdmin;

    assert(runnable);

    IN_TRACE_1(Send, 2, ">>> in_channelSdpWriterPeriodicAction called %x", runnable);

    _this = in_channelSdpWriter(runnable);
    connAdmin = in_connectivityAdminGetInstance();

    in_connectivityAdminLock(connAdmin);

    /* step 1: ack received discovery information */
    in_channelSdpWriterSendAcks(_this, connAdmin);
    /* step 2: Send requested discovery information to newly detected peers */
    in_channelSdpWriterSendRequestedDiscoveryInformation(_this, connAdmin);
    /* step 3: send participant heartbeats */
    in_channelSdpWriterSendParticipantsHeartbeat(_this, connAdmin);
    /* step 4: Send acknack for heartbeats */
    in_channelSdpWriterAcknackHeartbeats(_this, connAdmin);
    /* step 4: Send participant data periodically */
    in_channelSdpWriterSendParticipantsData(_this, connAdmin);

    in_connectivityAdminUnlock(connAdmin);
    IN_TRACE_1(Send, 2, "<<< in_channelSdpWriterPeriodicAction call finished for %x", runnable);
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

    assert(_this);

    os_mutexLock(&(_this->mutex));
    size = Coll_List_getNrOfElements(&(_this->heartbeatEvents));
    IN_TRACE_1(Send, 2, ">>> in_channelSdpWriterAcknackHeartbeats - processing acks for %d received heartbeats", size);
    for(i = 0; i < size; i++)
    {
        OS_STRUCT(in_ddsiEntityId) tmp = UI2ENTITYID(IN_ENTITYID_SEDP_BUILTIN_SUBSCRIPTIONS_READER);
        OS_STRUCT(in_ddsiSequenceNumber) nilSeq = {0,0};

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
                OS_STRUCT(in_ddsiSequenceNumber) tmpSN;
                OS_STRUCT(in_ddsiSequenceNumberSet) seqSet;
                os_boolean initOk;
                os_boolean canAddSeq = OS_TRUE;

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
                        IN_TRACE_2(Send, 2, "in_channelSdpWriterAcknackHeartbeats - @@@@@@@@@@@@@ %d, %d", seqSet.bitmapBase.low,seqSet.bitmapBase.high);
                        result = in_streamWriterAppendAckNack(
                            _this->streamWriter,
                            &reader,
                            &writer,
                            &seqSet,
                            destLocator);
                        if(result != IN_RESULT_OK)
                        {
                            IN_TRACE(Send, 2, "in_channelSdpWriterAcknackHeartbeats - Sending ACKNACK failed.");
                            /* TODO report error */
                        }
                        in_streamWriterFlushSingle(_this->streamWriter, destLocator);
                        //in_locatorFree(destLocator);
                        iteratorPart = Coll_Iter_getNext(iteratorPart);
                    }
                }
            } else
            {
                IN_TRACE(Send, 2, "in_channelSdpWriterAcknackHeartbeats - Unable to find a suitable UDPV4 locator, unable to transmit acknack.");
            }
        }
        in_locatorFree(event->destination);
        os_free(event);
    }
    os_mutexUnlock(&(_this->mutex));
    IN_TRACE_1(Send, 2, "<<< in_channelSdpWriterAcknackHeartbeats - processed acks for %d received heartbeats", size);
}


void
in_channelSdpWriterSendAcks(
    in_channelSdpWriter _this,
    in_connectivityAdmin connAdmin)
{
    os_uint32 size;
    os_uint32 i;
    in_discoveredPeer peer;
    Coll_Iter* iteratorPart;
    Coll_Set* clientEntities;
    OS_STRUCT(in_ddsiGuid) readerId;
    OS_STRUCT(in_ddsiGuid) writerId;
    in_connectivityParticipantFacade facade;
    in_ddsiGuid partGuid;
    in_locator dest;
    in_result result;
    OS_STRUCT(in_ddsiSequenceNumber) nilSeq = {0,1};

    assert(_this);
    os_mutexLock(&(_this->mutex));
    size = Coll_List_getNrOfElements(&_this->ackPeers);
    IN_TRACE_1(Send, 2, ">>> in_channelSdpWriterSendAcks - processing acks for %d entities", size);
    for(i = 0; i < size; i++)
    {
        OS_STRUCT(in_ddsiEntityId) partReaderId = UI2ENTITYID(IN_ENTITYID_P2P_BUILTIN_PARTICIPANT_MESSAGE_READER);
        OS_STRUCT(in_ddsiEntityId) partWriterId = UI2ENTITYID(IN_ENTITYID_P2P_BUILTIN_PARTICIPANT_MESSAGE_WRITER);
        assert(0);//should not come here

        peer = (in_discoveredPeer)Coll_List_popBack(&_this->ackPeers);

        /* Set readerId and writerId values, the readerId.guidPrefix will be
         * set per participantFacade.
         */
        readerId.entityId = partReaderId; //*(peer->readerId);
        writerId.entityId = partWriterId; //*(peer->writerId);
        memcpy(writerId.guidPrefix, peer->writerGuidPrefix, in_ddsiGuidPrefixLength);
        /* Now iterate over all known participant facades, for each found
         * facade send the acknack message
         */
        clientEntities = in_connectivityAdminGetParticipants(connAdmin);
        iteratorPart = Coll_Set_getFirstElement(clientEntities);
        while(iteratorPart)
        {
            facade = in_connectivityParticipantFacade(Coll_Iter_getObject(iteratorPart));
            IN_TRACE_1(Send, 2, ">>> in_channelSdpWriterSendAcks - processing acks for %p facade", facade);
            /* Fill out the readerId.guidPrefix*/
            partGuid = in_connectivityEntityFacadeGetGuid(in_connectivityEntityFacade(facade));
            memcpy(readerId.guidPrefix, partGuid->guidPrefix, in_ddsiGuidPrefixLength);
            /* Fetch the destination locator */
            dest = in_channelSdpWriterGetEntityLocator(peer->discoveredPeerEntity);
            if(dest)
            {
                OS_STRUCT(in_ddsiSequenceNumberSet) seqSet;
                os_boolean initOk;
                IN_TRACE_1(Send, 2, ">>> in_channelSdpWriterSendAcks - processing acks for %p destxxx", dest);


                initOk = in_ddsiSequenceNumberSetInit(&seqSet, &nilSeq);
                if(!initOk)
                {
                    IN_REPORT_ERROR("in_channelSdpWriterSendAcks", "Initialisation of the sequence number set failed!");
                } else
                {
                    IN_TRACE_1(Send, 2, ">>> in_channelSdpWriterSendAcks - processing acks for %p dest - actually sendingxxx", dest);
                    result = in_streamWriterAppendAckNack(
                        _this->streamWriter,
                        &readerId,
                        &writerId,
                        &seqSet,
                        dest);
                    if(result != IN_RESULT_OK)
                    {
                        IN_TRACE(Send, 2, "Sending ACKNACK failed.");
                        /* TODO report error */
                    }
                }
            } else {
                IN_TRACE(Send, 2, "Unable to find a suitable UDPV4 locator, unable to transmit acknack.");
            }
            iteratorPart = Coll_Iter_getNext(iteratorPart);
        }
        /* We need to free the peer object, as we popped it from the list */
        os_free(peer);
    }
    os_mutexUnlock(&(_this->mutex));
    IN_TRACE_1(Send, 2, "<<< in_channelSdpWriterSendAcks - processed acks for %d entities", size);
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

    os_mutexLock(&(_this->mutex));
    Coll_List_init(&tempList);
    size = Coll_List_getNrOfElements(&_this->acknacks);
    IN_TRACE_1(Send,2,">>> in_channelSdpWriterSendRequestedDiscoveryInformation: processing %x acknacks", size);
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
                OS_STRUCT(in_ddsiEntityId) tmp = UI2ENTITYID(IN_ENTITYID_SEDP_BUILTIN_SUBSCRIPTIONS_READER);
                OS_STRUCT(in_ddsiEntityId) tmp2 = UI2ENTITYID(IN_ENTITYID_SEDP_BUILTIN_PUBLICATIONS_READER);

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
                        //if(in_ddsiSequenceNumberCompare(in_connectivityReaderFacadeGetSequenceNumber(facade), &acknack->lastSeqNr) != C_LT)
                       // {
                            result = in_streamWriterAppendReaderData(
                                    in_streamWriter(_this->streamWriter),
                                    acknack->sourceGuidPrefix,
                                    _this->discoveryData,
                                    facade,
                                    &tempList);
                            if(result != IN_RESULT_OK)
                            {
                                IN_TRACE(Send,2,"in_channelSdpWriterSendRequestedDiscoveryInformation: sending reader data failed");
                                /* TODO report error */
                            }
                       // }
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
                                IN_TRACE(Send,2,"in_channelSdpWriterSendRequestedDiscoveryInformation: sending writer data failed");
                                /* TODO report error */
                            }
                        }
                        iterator = Coll_Iter_getNext(iterator);
                    }
                } else
                {
                    IN_TRACE(Send, 2, "in_channelSdpWriterSendRequestedDiscoveryInformation: Unable to match entityId, unable transmit reader/writer data.");
                }
                /* step 1.4: Flush the stream writer */
                in_streamWriterFlush(
                    in_streamWriter(_this->streamWriter),
                    &tempList);
                /* clear the list */
                Coll_List_popBack(&tempList);
            } else {
                IN_TRACE(Send, 2, "in_channelSdpWriterSendRequestedDiscoveryInformation: Unable to find a suitable UDPV4 locator, unable to transmit reader/writer data.");
            }
        } else {
            IN_TRACE(Send, 2, "in_channelSdpWriterSendRequestedDiscoveryInformation: Unable to find a matching peer participant for the received guid. unable to transmit reader/writer data.");
        }
        os_free(acknack);
    }
    os_mutexUnlock(&(_this->mutex));
    IN_TRACE_1(Send,2,"<<< in_channelSdpWriterSendRequestedDiscoveryInformation: processed %x acknacks", size);
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

    assert(_this);

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

        } else
        {
            Coll_Set* peers;
            Coll_Iter* peerIterator;

            peers = in_connectivityAdminGetPeerParticipantsUnsafe(connAdmin);
            peerIterator = Coll_Set_getFirstElement(peers);
            while(peerIterator)
            {
                in_connectivityPeerParticipant peer;
                in_locator locator;

                peer = in_connectivityPeerParticipant(Coll_Iter_getObject(peerIterator));
                /* Get a unicast UDPV4 locator, or if not available the
                 * UDPV4 multicast locator, or if that is not available then fall back
                 * to the default locators of the owning participant.
                 */
                locator = in_channelSdpWriterGetEntityLocator(in_connectivityPeerEntity(peer));
                if(locator)
                {
                    in_ddsiGuidPrefixRef destGuidPrefix;

                    destGuidPrefix = in_connectivityPeerParticipantGetGuidPrefix(peer);
                    result = in_streamWriterAppendParticipantMessage(
                        in_streamWriter(_this->streamWriter),
                        facade,
                        destGuidPrefix,
                        locator);
                    if(result != IN_RESULT_OK)
                    {
                        /* todo report error */
                    }
                } else
                {
                    IN_TRACE(Send, 2, "Unable to find a suitable UDPV4 locator, unable to transmit participant message.");
                }

                peerIterator = Coll_Iter_getNext(peerIterator);
            }
        }
        iterator = Coll_Iter_getNext(iterator);

    }

    Coll_List_popBack(&tempList);
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

    assert(_this);
    assert(connAdmin);

    //singleDestination = in_streamWriterGetDataMulticastLocator(_this->streamWriter);
    clientEntities = in_connectivityAdminGetParticipants(connAdmin);
    IN_TRACE_1(Send, 2, ">>> in_channelSdpWriterSendParticipantsHeartbeat - sending heartbeat messages for %d participant(s)", Coll_Set_getNrOfElements(clientEntities));
    iterator = Coll_Set_getFirstElement(clientEntities);
    while(iterator)
    {
        static os_uint32 partCount = 0;
        static os_uint32 writerCount = 0;
        static os_uint32 readerCount = 0;
        Coll_Set* participantPeers;
        Coll_Iter* peerIter;

        facade = in_connectivityParticipantFacade(Coll_Iter_getObject(iterator));
        tmpGuid = in_connectivityEntityFacadeGetGuid(in_connectivityEntityFacade(facade));
        sourceGuidPrefix = tmpGuid->guidPrefix;
        participantPeers = in_connectivityParticipantFacadeGetMatchedPeerParticipantsUnsafe(facade);
        peerIter = Coll_Set_getFirstElement(participantPeers);
        while(peerIter)
        {
            in_connectivityPeerParticipant peer;

            peer = in_connectivityPeerParticipant(Coll_Iter_getObject(peerIter));
            destGuidPrefix = in_connectivityPeerParticipantGetGuidPrefix(peer);
            singleDestination = in_channelSdpWriterGetEntityLocator(in_connectivityPeerEntity(peer));
            if(!singleDestination)
            {
                IN_TRACE(Send, 2, "Unable to find a suitable UDPV4 locator, unable to transmit heartbeat.");
            } else
            {

                lastSN = in_connectivityParticipantFacadeGetNrOfWriters(facade);
IN_TRACE_2(Send, 2, "in_channelSdpWriterSendParticipantsHeartbeat - last seq number writers = %d, %d", lastSN->high, lastSN->low);
              //  if(in_ddsiSequenceNumberCompare(lastSN, &nilSN) == C_GT)
              //  {


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
                         /*TODO report error*/
                    }
              //  }
                lastSN = in_connectivityParticipantFacadeGetNrOfReaders(facade);
IN_TRACE_2(Send, 2, "in_channelSdpWriterSendParticipantsHeartbeat - last seq number readers = %d, %d", lastSN->high, lastSN->low);
              //  if(in_ddsiSequenceNumberCompare(lastSN, &nilSN) == C_GT)
             //   {


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
                         /*TODO report error*/
                    }
               // }

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
                if(result != IN_RESULT_OK)
                {
                     /*TODO report error*/
                }
            }
            peerIter = Coll_Iter_getNext(peerIter);
        }
        iterator = Coll_Iter_getNext(iterator);
    }
    in_streamWriterFlushSingle(_this->streamWriter, singleDestination);
    IN_TRACE_1(Send, 2, "<<< in_channelSdpWriterSendParticipantsHeartbeat - sent heartbeat messages for %d participant(s)", Coll_Set_getNrOfElements(clientEntities));
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

    assert(entity);

    /* step 1: locate the corresponding peer participant for the peer entity */
    connAdmin = in_connectivityAdminGetInstance();
    kind = in_objectGetKind(in_object(entity));
    switch (kind)
    {
        case IN_OBJECT_KIND_PEER_PARTICIPANT:
            peerParticipant = in_connectivityPeerParticipant(entity);
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
        Coll_List* locators;

        locators = in_connectivityPeerParticipantGetDefaultUnicastLocators(peerParticipant);
        locator = in_channelSdpWriterFindUDPV4Locator(locators);
        if(!locator)
        {
            locators = in_connectivityPeerParticipantGetDefaultMulticastLocators(peerParticipant);
            locator = in_channelSdpWriterFindUDPV4Locator(locators);
        }
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
                /* TODO report error */
            }
            IN_TRACE_3(Send,2,"in_channelSdpWriterParticipantAction - sending participant data %x (%x,%x)",_this, data->key.systemId,data->key.localId);
            result = in_streamWriterAppendParticipantData(
                    in_streamWriter(sdpWriter->streamWriter),
                    sdpWriter->discoveryData,
                    facade,
                    &tempList);
            if(result != IN_RESULT_OK)
            {
                /* TODO report error */
            }
            in_streamWriterFlush(
                in_streamWriter(sdpWriter->streamWriter),
                &tempList);
            /* clear the list */
            Coll_List_popBack(&tempList);
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
    in_connectivityReaderFacade facade;
    in_channelSdpWriterEntityAction action;
    in_channelSdpWriter sdpWriter;


    IN_TRACE_3(Send,2,"in_channelSdpWriterSubscriptionAction %x (%x,%x)",_this, data->key.systemId,data->key.localId);

    sdpWriter = in_channelSdpWriter(_this);
    action = in_channelSdpWriterDetermineEntityAction(sampleState, instanceState);
    connAdmin = in_connectivityAdminGetInstance();
    /* step 1: if participant is new or modified then add/update the participant
     */
    if(action == ACTION_NEW)
    {
        result = in_connectivityAdminAddReader(connAdmin, data);
        if(result != IN_RESULT_OK)
        {
            /* TODO error report */
        }
    }
    /* step 2: notified all interest parties of the participant status, however
     * if the participant was already deleted, then we wont inform this at all
     * and just ignore it
     */
    if(action != ACTION_NEW_DELETED)
    {

        facade = in_connectivityAdminGetReader(connAdmin, data);
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
                /* TODO report error */
            }
        //    result = in_streamWriterAppendReaderData(
        //            in_streamWriter(sdpWriter->streamWriter),
        //            NULL, /* optional */
        //            sdpWriter->discoveryData,
        ///            facade,
        //            &tempList);
        //    if(result != IN_RESULT_OK)
       //     {
                /* TODO report error */
       //     }
       //     in_streamWriterFlush(
      //          in_streamWriter(sdpWriter->streamWriter),
     //           &tempList);
            /* clear the list */
            Coll_List_popBack(&tempList);
        } /* else no one interested */
    }
    /* step 3: if the participant was deleted, we can remove it from the
     * connectivity admin after we notified all interested parties
     */
    if(action == ACTION_DELETED)
    {
        result = in_connectivityAdminRemoveReader(connAdmin, data);
        if(result != IN_RESULT_OK)
        {
            /* TODO error report */
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
    in_connectivityWriterFacade facade;
    in_channelSdpWriterEntityAction action;
    in_channelSdpWriter sdpWriter;


    IN_TRACE_3(Send,2,"in_channelSdpWriterPublicationAction %x (%x,%x)",_this, data->key.systemId,data->key.localId);
    IN_TRACE_3(Send,2,"in_channelSdpWriterPublicationAction %x part(%x,%x)",_this,data->participant_key.systemId,data->participant_key.localId);

    sdpWriter = in_channelSdpWriter(_this);
    action = in_channelSdpWriterDetermineEntityAction(sampleState, instanceState);
    connAdmin = in_connectivityAdminGetInstance();
    /* step 1: if participant is new or modified then add/update the participant
     */
    IN_TRACE_1(Send,2,"in_channelSdpWriterPublicationAction here 1",_this);
    if(action == ACTION_NEW)
    {
        IN_TRACE_1(Send,2,"in_channelSdpWriterPublicationAction here 2",_this);
        result = in_connectivityAdminAddWriter(connAdmin, data);
        IN_TRACE_1(Send,2,"in_channelSdpWriterPublicationAction here 3",_this);
        if(result != IN_RESULT_OK)
        {
            /* TODO error report */
        }
    }
    /* step 2: notified all interest parties of the participant status, however
     * if the participant was already deleted, then we wont inform this at all
     * and just ignore it
     */
    if(action != ACTION_NEW_DELETED)
    {
IN_TRACE_1(Send,2,"in_channelSdpWriterPublicationAction here 4",_this);
        facade = in_connectivityAdminGetWriter(connAdmin, data);
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
                /* TODO report error */
            }
            IN_TRACE_1(Send,2,"in_channelSdpWriterPublicationAction here 5",_this);
     //       result = in_streamWriterAppendWriterData(
     //               in_streamWriter(sdpWriter->streamWriter),
     //               sdpWriter->discoveryData,
    //                facade,
    //                &tempList);
    //        if(result != IN_RESULT_OK)
    //        {
                /* TODO report error */
    //        }
            IN_TRACE_1(Send,2,"in_channelSdpWriterPublicationAction here 6",_this);
 //           in_streamWriterFlush(
  //              in_streamWriter(sdpWriter->streamWriter),
  //              &tempList);
            /* clear the list */
            Coll_List_popBack(&tempList);
        } /* else no one interested */
    }
    /* step 3: if the participant was deleted, we can remove it from the
     * connectivity admin after we notified all interested parties
     */
    if(action == ACTION_DELETED)
    {IN_TRACE_1(Send,2,"in_channelSdpWriterPublicationAction here 7",_this);
        result = in_connectivityAdminRemoveWriter(connAdmin, data);
        if(result != IN_RESULT_OK)
        {
            /* TODO error report */
        }
        IN_TRACE_1(Send,2,"in_channelSdpWriterPublicationAction here 8",_this);
    }
    IN_TRACE_1(Send,2,"in_channelSdpWriterPublicationAction here 9",_this);
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

    if (userData &&
        ((event == V_EVENT_NEW_GROUP) || (event == V_EVENT_UNDEFINED)))
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
            u_entity(_this->reader),
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
        /* Step 3: determine number of channels which will use the created entry
         * --- skipped
         */

        /* Step 4: Create a new entry within the network reader */
        entry = v_networkReaderEntryNew(
            reader,
            group,
            V_NETWORKID_DDSI,
            1,/* must be 1 to allow finalizing the entry, see next step */
            networkPartitionId);
        /* Step 5: Notify the entry to finalize itself, should rewrite the
         * entry to finalize itself upon creation. This code is a tad.. weird
         * now...
         */
        serviceName = u_serviceGetName(_this->service);
        v_networkReaderEntryNotifyConnected(entry, serviceName);
        os_free(serviceName);
    }
    else
    {
        IN_TRACE(Send, 1, "in_channelSdpWriterProcessGroupEvent: this else shall never happen!!!!");
    }
}

v_networkPartitionId
in_channelSdpWriterGetBestFitPartition(
    in_channelSdpWriter _this,
    const os_char* dcpsPartitionName,
    const os_char* dcpsTopicName)
{
    return 1;
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
