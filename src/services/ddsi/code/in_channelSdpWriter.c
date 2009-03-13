#include "in_channel.h"
#include "in_channelSdpWriter.h"
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

static os_boolean
in_channelSdpWriterInit(
    in_channelSdpWriter _this,
    in_channelSdp channel,
    in_objectKind kind,
    in_objectDeinitFunc deinit,
    u_participant participant,
    in_streamWriter writer);

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
in_channelSdpWriterPeriodicAction (
    in_runnable _this);

static in_locator
in_channelSdpWriterGetEntityLocator(
    in_connectivityPeerEntity entity);

OS_STRUCT(in_channelSdpWriter)
{
    OS_EXTENDS(in_channelWriter);
    OS_STRUCT(in_clientMonitor) monitor;
    in_streamWriter streamWriter;
    Coll_List discoveredPeers;
};

in_channelSdpWriter
in_channelSdpWriterNew(
    in_channelSdp sdp,
    u_participant participant,
    in_streamWriter writer)
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
            writer);
        if(!success)
        {
            os_free(_this);
            _this = NULL;
        }
    }
    return _this;
}


typedef enum in_channelSdpWriterEntityAction_e
{
    ACTION_NEW,
    ACTION_NEW_DELETED,
    ACTION_DELETED
} in_channelSdpWriterEntityAction;

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



void
in_channelSdpWriterParticipantAction (
    in_runnable _this,
    v_state sampleState,
    v_state instanceState,
    v_message msg,
    struct v_participantInfo *data)
{
    in_result result;
    in_connectivityAdmin connAdmin;
    in_connectivityParticipantFacade facade;
    in_channelSdpWriterEntityAction action;
    in_channelSdpWriter sdpWriter;

    sdpWriter = in_channelSdpWriter(_this);
    action = in_channelSdpWriterDetermineEntityAction(sampleState, instanceState);
    connAdmin = in_connectivityAdminGetInstance();
    /* step 1: if participant is new or modified then add/update the participant
     */
    if(action == ACTION_NEW)
    {
        result = in_connectivityAdminAddParticipant(connAdmin, data);
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

        facade = in_connectivityAdminGetParticipant(connAdmin, data);
        if(facade)
        {
            in_result result;
            Coll_List tempList;
            os_uint32 errorCode;
            in_locator locator;

            Coll_List_init(&tempList);
            locator = NULL;/* TODO fill in correct call to fetch locator from the stream */
            errorCode = Coll_List_pushBack(&tempList, locator);
            if(errorCode != COLL_OK)
            {
                /* TODO report error */
            }
            result = in_streamWriterAppendParticipantData(
                    in_streamWriter(sdpWriter->streamWriter),
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
        if(result != IN_RESULT_OK)
        {
            /* TODO error report */
        }
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
            locator = NULL;/* TODO fill in correct call to fetch locator from the stream */
            errorCode = Coll_List_pushBack(&tempList, locator);
            if(errorCode != COLL_OK)
            {
                /* TODO report error */
            }
            result = in_streamWriterAppendReaderData(
                    in_streamWriter(sdpWriter->streamWriter),
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

    sdpWriter = in_channelSdpWriter(_this);
    action = in_channelSdpWriterDetermineEntityAction(sampleState, instanceState);
    connAdmin = in_connectivityAdminGetInstance();
    /* step 1: if participant is new or modified then add/update the participant
     */
    if(action == ACTION_NEW)
    {
        result = in_connectivityAdminAddWriter(connAdmin, data);
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

        facade = in_connectivityAdminGetWriter(connAdmin, data);
        if(facade)
        {
            in_result result;
            Coll_List tempList;
            os_uint32 errorCode;
            in_locator locator;

            Coll_List_init(&tempList);
            locator = NULL;/* TODO fill in correct call to fetch locator from the stream */
            errorCode = Coll_List_pushBack(&tempList, locator);
            if(errorCode != COLL_OK)
            {
                /* TODO report error */
            }
            result = in_streamWriterAppendWriterData(
                    in_streamWriter(sdpWriter->streamWriter),
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
        result = in_connectivityAdminRemoveWriter(connAdmin, data);
        if(result != IN_RESULT_OK)
        {
            /* TODO error report */
        }
    }
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

    errorCode = Coll_List_pushBack(&_this->discoveredPeers, entity);
    if(errorCode != COLL_OK)
    {
        result = IN_RESULT_OUT_OF_MEMORY;
    }

    return result;
}

void
in_channelSdpWriterPeriodicAction (
    in_runnable runnable)
{
    in_channelSdpWriter _this;
    in_connectivityPeerEntity peerEntity;
    in_locator locator;
    in_connectivityAdmin connAdmin;
    Coll_List tempList;/* on stack definition */
    Coll_Set* clientEntities;
    Coll_Iter* iterator;
    in_result result;

    assert(runnable);

    _this = in_channelSdpWriter(runnable);
    connAdmin = in_connectivityAdminGetInstance();
    Coll_List_init(&tempList);
    /* step 1: ack received discovery information */
    /*for each received discovery entity, send an ack msg back to the sending locator...
    //so i need:
    // - seq number
    // - sender locator
    */
    /* step 2: send out local discovery information to newly detected peers */
    while(Coll_List_getNrOfElements(&_this->discoveredPeers) > 0)
    {
        peerEntity = in_connectivityPeerEntity(Coll_List_popBack(&_this->discoveredPeers));
        /* step 2.1: Get a unicast locator, or if not available the multicast locator */
        locator = in_channelSdpWriterGetEntityLocator(peerEntity);
        assert(locator);/* expect each entity to have a least 1 locator! */
        Coll_List_pushBack(&tempList, locator);
        /* step 2.2: Get all local entity information, so we can send this to the
         * locator found in the previous step
         */
        clientEntities = in_connectivityAdminGetParticipants(connAdmin);
        iterator = Coll_Set_getFirstElement(clientEntities);
        while(iterator)
        {
            in_connectivityParticipantFacade facade;

            facade = in_connectivityParticipantFacade(Coll_Iter_getObject(iterator));
            result = in_streamWriterAppendParticipantData(
                    in_streamWriter(_this->streamWriter),
                    facade,
                    &tempList);
            if(result != IN_RESULT_OK)
            {
                /* TODO report error */
            }
            iterator = Coll_Iter_getNext(iterator);
        }
        clientEntities = in_connectivityAdminGetReaders(connAdmin);
        iterator = Coll_Set_getFirstElement(clientEntities);
        while(iterator)
        {
            in_connectivityReaderFacade facade;

            facade = in_connectivityReaderFacade(Coll_Iter_getObject(iterator));
            result = in_streamWriterAppendReaderData(
                    in_streamWriter(_this->streamWriter),
                    facade,
                    &tempList);
            if(result != IN_RESULT_OK)
            {
                /* TODO report error */
            }
            iterator = Coll_Iter_getNext(iterator);
        }
        clientEntities = in_connectivityAdminGetWriters(connAdmin);
        iterator = Coll_Set_getFirstElement(clientEntities);
        while(iterator)
        {
            in_connectivityWriterFacade facade;

            facade = in_connectivityWriterFacade(Coll_Iter_getObject(iterator));
            result = in_streamWriterAppendWriterData(
                    in_streamWriter(_this->streamWriter),
                    facade,
                    &tempList);
            if(result != IN_RESULT_OK)
            {
                /* TODO report error */
            }
            iterator = Coll_Iter_getNext(iterator);
        }
        in_streamWriterFlush(
            in_streamWriter(_this->streamWriter),
            &tempList);
        /* clear the list */
        Coll_List_popBack(&tempList);
    }

}

in_locator
in_channelSdpWriterGetEntityLocator(
    in_connectivityPeerEntity entity)
{
    Coll_List* locators;
    in_locator locator = NULL;

    assert(entity);
    locators = in_connectivityPeerEntityGetUnicastLocators(entity);
    if(Coll_List_getNrOfElements(locators) > 0)
    {
        locator = in_locator(Coll_List_getObject(locators, 0));
    } else
    {
        locators = in_connectivityPeerEntityGetMulticastLocators(entity);
        if(Coll_List_getNrOfElements(locators) > 0)
        {
            locator = in_locator(Coll_List_getObject(locators, 0));
        }
    }
    return locator;
}


os_boolean
in_channelSdpWriterInit(
    in_channelSdpWriter _this,
    in_channelSdp channel,
    in_objectKind kind,
    in_objectDeinitFunc deinit,
    u_participant participant,
    in_streamWriter writer)
{
    os_boolean success;
    c_time periodic = {1, 0};

    assert(_this);
    assert(kind < IN_OBJECT_KIND_COUNT);
    assert(kind > IN_OBJECT_KIND_INVALID);
    assert(deinit);

    success = in_channelWriterInit(
        in_channelWriter(_this),
        in_channel(channel),
        kind,
        deinit,
        "in_channelSdpWriter",
        NULL,
        in_channelSdpWriterRun,
        in_channelSdpWriterTrigger);

    _this->streamWriter = in_streamWriterKeep(writer);
    Coll_List_init(&_this->discoveredPeers);

    in_clientMonitorInit(
		&_this->monitor,
		in_runnable(_this),
		participant,
		periodic,
		in_channelSdpWriterParticipantAction,
		in_channelSdpWriterSubscriptionAction,
		in_channelSdpWriterPublicationAction,
		in_channelSdpWriterPeriodicAction);

#if 0
    _this->participant = participant;
    _this->waitset = u_waitsetNew(participant);
    if(!_this->waitset)
    {
        in_channelWriterDeinit(in_object(_this));
        success = OS_FALSE;
    }
    if(success)
    {
        success = in_channelSdpWriterSetupClientMonitor(_this);
    }
#endif

    return success;
}

void
in_channelSdpWriterDeinit(
    in_object _this)
{
    assert(_this);
    assert(in_channelSdpWriterIsValid(_this));

    in_streamWriterFree(in_channelSdpWriter(_this)->streamWriter);

    in_channelWriterDeinit(_this);
}

void*
in_channelSdpWriterRun(
    in_runnable runnable)
{
    in_channelSdpWriter _this;

    assert(runnable);

    _this = in_channelSdpWriter(runnable);

    in_clientMonitorRun(&_this->monitor);
    return NULL;
}

void
in_channelSdpWriterTrigger(
    in_runnable runnable)
{
    in_channelSdpWriter _this;

    assert(runnable);

    _this = in_channelSdpWriter(runnable);
    /* TODO tbd */
}

#if 0
os_boolean
in_channelSdpWriterSetupClientMonitor(
    in_channelSdpWriter _this)
{
    os_boolean result = OS_TRUE;
    u_subscriber subscriber;
    u_participant participant = NULL;
    c_iter readers;
    os_uint32 length;
    u_result uresult;

    assert(_this);

    /* Gain access to the built-in subscriber. */
    _this->subscriber = u_participantGetBuiltinSubscriber(_this->participant);
    if(_this->subscriber)
    {
        /* Gain access to the built-in participant reader. */
        readers = u_subscriberLookupReaders(_this->subscriber, V_PARTICIPANTINFO_NAME);
        length  = c_iterLength(readers);
        if(length == 1)
        {
            _this->participantReader = (u_dataReader)c_iterTakeFirst(readers);
        } else
        {
            printf("Could not resolve built-in participant reader.\n");
            result = OS_FALSE;
        }
        c_iterFree(readers);
    }
    if(_this->subscriber && result)
    {
        /* Gain access to the built-in subscription reader. */
        readers = u_subscriberLookupReaders(_this->subscriber, V_SUBSCRIPTIONINFO_NAME);
        length  = c_iterLength(readers);
        if(length == 1)
        {
            _this->subscriptionReader = (u_dataReader)c_iterTakeFirst(readers);
        } else
        {
            printf("Could not resolve built-in subscription reader.\n");
            result = OS_FALSE;
        }
        c_iterFree(readers);
    }
    if(_this->subscriber && result)
    {
        /* Gain access to the built-in publication reader. */
        readers = u_subscriberLookupReaders(_this->subscriber, V_PUBLICATIONINFO_NAME);
        length  = c_iterLength(readers);
        if(length == 1)
        {
            _this->publicationReader = (u_dataReader)c_iterTakeFirst(readers);
        } else
        {
            printf("Could not resolve built-in publication reader.\n");
            result = FALSE;
        }
        c_iterFree(readers);
    }
    return result;
}

void
in_channelSdpWriterTerminateClientMonitor(
    in_channelSdpWriter _this)
{
    u_result uresult;

    assert(_this);

    /* Delete datareaders */
    uresult = u_dataReaderFree(_this->participantReader);
    if(uresult != U_RESULT_OK)
    {
        printf("Deletion of participant reader failed.\n");
    }

    uresult = u_dataReaderFree(_this->subscriptionReader);
    if(uresult != U_RESULT_OK)
    {
        printf("Deletion of subscription reader failed.\n");
    }

    uresult = u_dataReaderFree(_this->publicationReader);
    if(uresult != U_RESULT_OK)
    {
        printf("Deletion of publication reader failed.\n");
    }

    /* Delete subscriber */
    uresult = u_subscriberFree(_this->subscriber);
    if(uresult != U_RESULT_OK)
    {
        printf("Deletion of subscriber failed.\n");
    }
}

u_result
in_channelSdpWriterAttachClientMonitor(
	in_channelSdpWriter _this)
{
    u_waitset waitset;
    u_dataReader dataReader;
    c_iter readers;
    u_result result;
    u_uint32 i;
    u_uint32 length;

    /*Set event mask of the waitset.*/
    result = u_waitsetSetEventMask(_this->waitset, V_EVENT_DATA_AVAILABLE);
    if(result == U_RESULT_OK)
    {
        readers = c_iterNew(participantReader);
        readers = c_iterInsert(readers, _this->publicationReader);
        readers = c_iterInsert(readers, _this->subscriptionReader);
        length = c_iterLength(readers);
        for( i = 0; i < length && (result == U_RESULT_OK); i++)
        {
            dataReader = (u_dataReader)(c_iterObject(readers, i));

            /* Set event mask of the datareader to trigger on available data. */
            result = u_dispatcherSetEventMask(
                (u_dispatcher)dataReader,
                V_EVENT_DATA_AVAILABLE);
            if(result == U_RESULT_OK)
            {
                /*Attach reader to the waitset.*/
                result = u_waitsetAttach(
                    waitset,
                    (u_entity)dataReader,
                    (u_entity)dataReader);
                if(result != U_RESULT_OK)
                {
                    printf("Could not attach datareader to waitset.\n");
                }
            } else
            {
                printf("Could not set event mask of datareader.\n");
            }
        }
    } else
    {
        printf("Could not set event mask of waitset.\n");
    }
    return result;
}

u_result
in_channelSdpWriterDetachClientMonitor(
	in_channelSdpWriter _this)
{
    c_iter readers;
    u_result result;
    u_uint32 i;
    u_uint32 length;

    assert(_this);

    readers = c_iterNew(participantReader);
    readers = c_iterInsert(readers, _this->publicationReader);
    readers = c_iterInsert(readers, _this->subscriptionReader);
    length = c_iterLength(readers);

    /* Detach all datareaders from the waitset. */
    for(i = 0; i < length; i++)
    {
        u_waitsetDetach(waitset, (u_entity)(c_iterObject(readers, i)));
    }
    c_iterFree(readers);

    return result;
}

void
in_channelSdpWriterMonitorForClients(
    in_channelSdpWriter _this)
{
    assert(_this);


}

u_result
in_channelSdpWriterPrepareClientMonitor(
	in_channelSdpWriter _this,
    const u_dataReader participantReader,
    const u_dataReader publicationReader,
    const u_dataReader subscriptionReader)
{
    u_waitset waitset;
    u_dataReader dataReader;
    c_iter readers;
    u_result result = U_RESULT_INTERNAL_ERROR;
    os_uint32 i;
    os_uint32 length;

    u_participant participant = _this->participant;

    /* Create waitset. */
    waitset = u_waitsetNew(participant);

    if(waitset)
    {
        /* Set event mask of the waitset. */
        result = u_waitsetSetEventMask(waitset, V_EVENT_DATA_AVAILABLE);
        if(result == U_RESULT_OK)
        {
            readers = c_iterNew(participantReader);
            readers = c_iterInsert(readers, publicationReader);
            readers = c_iterInsert(readers, subscriptionReader);
            length = c_iterLength(readers);

            for(i = 0; i < length && (result == U_RESULT_OK); i++)
            {
                dataReader = (u_dataReader)(c_iterObject(readers, i));

                /* Set event mask of the datareader to trigger on available
                 * data.
                 */
                result = u_dispatcherSetEventMask(
                    (u_dispatcher)dataReader,
                    V_EVENT_DATA_AVAILABLE);

                if(result == U_RESULT_OK)
                {
                    /* Attach reader to the waitset. */
                    result = u_waitsetAttach(
                        waitset,
                        (u_entity)dataReader,
                        (u_entity)dataReader);

                    if(result != U_RESULT_OK)
                    {
                        printf("Could not attach datareader to waitset.\n");
                    }
                } else
                {
                    printf("Could not set event mask of datareader.\n");
                }
            }
        } else
        {
            printf("Could not set event mask of waitset.\n");
        }

        if(result == U_RESULT_OK)
        {
            /*Start monitoring the creation/deletion of entities.*/
            result = in_clientMonitorStartMonitoring(
                self,
                waitset,
                participantReader,
                publicationReader,
                subscriptionReader);
        }
        /*Detach all datareaders from the waitset.*/
        for(i = 0; i < length; i++)
        {
            u_waitsetDetach(waitset, (u_entity)(c_iterObject(readers, i)));
        }
        c_iterFree(readers);
        /* Delete the waitset. */
        result = u_waitsetFree(waitset);

        if(result != U_RESULT_OK)
        {
            printf("Deletion of waitset failed.\n");
        }
    } else
    {
        printf("Could not create waitset.\n");
    }

    return result;
}


u_result
in_channelSdpWriterHandleParticipant(
	in_channelSdpWriter _this,
    u_dataReader dataReader,
    c_long dataOffset)
{
    v_dataReaderSample sample;
    u_result result;
    v_state state;
    v_message msg;
    struct v_participantInfo *data;

    result = u_dataReaderTake(dataReader, takeOne, &sample);

    while(sample && (result == U_RESULT_OK)){
        state = v_readerSample(sample)->sampleState;
        msg   = v_dataReaderSampleMessage(sample);
        data  = (struct v_participantInfo *)(C_DISPLACE(msg, dataOffset));
#if 0
        if(v_stateTest(state, L_DISPOSED)){
            printf("DomainParticipant deleted.\n");
        } else {
            printf("DomainParticipant created.\n");
        }
        printf("    - Global ID         : %d, %d, %d\n",
                        data->key.systemId,
                        data->key.localId,
                        data->key.serial);

        _this->participantAction(_this->runnable,
        		state,
        		msg,
        		data);
#endif
        c_free(sample);
        sample = NULL;
        result = u_dataReaderTake(dataReader, takeOne, &sample);
    }
    return result;
}

u_result
in_channelSdpWriterHandleSubscription(
	in_channelSdpWriter _this,
    u_dataReader dataReader,
    c_long dataOffset)
{
    v_dataReaderSample sample;
    u_result result;
    v_state state;
    v_message msg;
    struct v_subscriptionInfo *data;

    result = u_dataReaderTake(dataReader, takeOne, &sample);

    while(sample && (result == U_RESULT_OK)){
        state = v_readerSample(sample)->sampleState;
        msg   = v_dataReaderSampleMessage(sample);
        data  = (struct v_subscriptionInfo *)(C_DISPLACE(msg, dataOffset));
#if 0
        if(v_stateTest(state, L_DISPOSED)){
            printf("DataReader deleted.\n");
        } else {
            printf("DataReader created.\n");
        }
        printf("    - Global ID         : %d, %d, %d\n",
                        data->key.systemId,
                        data->key.localId,
                        data->key.serial);
        printf("    - DomainParticipant : %d, %d, %d\n",
                        data->participant_key.systemId,
                        data->participant_key.localId,
                        data->participant_key.serial);
        printf("    - Topic             : %s\n",
                        data->topic_name);

        _this->subscriptionAction(_this->runnable,
        		state,
        		msg,
        		data);
#endif
        c_free(sample);
        sample = NULL;
        result = u_dataReaderTake(dataReader, takeOne, &sample);
    }
    return result;
}

u_result
in_channelSdpWriterHandlePublication(
	in_channelSdpWriter _this,
    u_dataReader dataReader,
    c_long dataOffset)
{
    v_dataReaderSample sample;
    u_result result;
    v_state state;
    v_message msg;
    struct v_publicationInfo *data;

    result = u_dataReaderTake(dataReader, takeOne, &sample);

    while(sample && (result == U_RESULT_OK))
    {
        state = v_readerSample(sample)->sampleState;
        msg   = v_dataReaderSampleMessage(sample);
        data  = (struct v_publicationInfo *)(C_DISPLACE(msg, dataOffset));

#if 0
        if(v_stateTest(state, L_DISPOSED))
        {
            printf("DataWriter deleted.\n");
        } else
        {
            printf("DataWriter created.\n");
        }
        printf("    - Global ID         : %d, %d, %d\n",
                data->key.systemId,
                data->key.localId,
                data->key.serial);
        printf("    - DomainParticipant : %d, %d, %d\n",
                data->participant_key.systemId,
                data->participant_key.localId,
                data->participant_key.serial);
        printf("    - Topic             : %s\n",
                data->topic_name);

        _this->publicationAction(_this->runnable,
        		state,
        		msg,
        		data);
#endif
        c_free(sample);
        sample = NULL;
        result = u_dataReaderTake(dataReader, takeOne, &sample);
    }
    return result;
}

#endif
