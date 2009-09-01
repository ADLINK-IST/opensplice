#include "kernelModule.h"
#include "in_connectivityAdmin.h"
#include "in_connectivityPeerWriter.h"
#include "in_connectivityWriterFacade.h"
#include "in_connectivityParticipantFacade.h"
#include "in_report.h"
#include "in__object.h"
#include "in__ddsiParticipant.h"
#include "in__ddsiSubscription.h"
#include "in__ddsiPublication.h"
#include "Coll_Compare.h"

#include "v_public.h"
#include "c_stringSupport.h"
#include "os_defs.h"
#include "os_heap.h"

static in_connectivityParticipantFacade
in_connectivityAdminFindParticipantUnsafe(
    in_connectivityAdmin _this,
    v_gid entity_id);

static in_connectivityPeerParticipant
in_connectivityAdminFindPeerParticipantUnsafe(
    in_connectivityAdmin _this,
    in_ddsiGuidPrefix prefix);

static in_result
in_connectivityAdminCopyLocators(
    Coll_List* source,
    Coll_List* target);

OS_STRUCT(in_connectivityAdmin)
{
    OS_EXTENDS(in_object);
    os_mutex mutex; /* protects the administration */
    Coll_Set Participants;
    Coll_Set Readers;
    Coll_Set Writers;
    Coll_Set PeerParticipants;
    Coll_Set PeerReaders;
    Coll_Set PeerWriters;
};

/* ------------- private  ------------------- */

static in_connectivityAdmin  theConnectivityAdmin = NULL;


static void
in_connectivityAdminDeinit(
    in_object _this)
{
	Coll_Iter* iterator;
	in_connectivityParticipantFacade pfacade;
	in_connectivityReaderFacade rfacade;
	in_connectivityWriterFacade wfacade;
	in_connectivityPeerParticipant ppeer;
	in_connectivityPeerReader rpeer;
	in_connectivityPeerWriter wpeer;

    assert(_this);
    assert(_this == (in_object)theConnectivityAdmin);

    iterator = Coll_Set_getFirstElement(&theConnectivityAdmin->Participants);

	while(iterator)
	{
		pfacade = in_connectivityParticipantFacade(Coll_Iter_getObject(iterator));
		iterator = Coll_Iter_getNext(iterator);
		Coll_Set_remove(&theConnectivityAdmin->Participants, pfacade);
		in_connectivityParticipantFacadeFree(pfacade);
	}
	iterator = Coll_Set_getFirstElement(&theConnectivityAdmin->Readers);

	while(iterator)
	{
		rfacade = in_connectivityReaderFacade(Coll_Iter_getObject(iterator));
		iterator = Coll_Iter_getNext(iterator);
		Coll_Set_remove(&theConnectivityAdmin->Readers, rfacade);
		in_connectivityReaderFacadeFree(rfacade);
	}
	iterator = Coll_Set_getFirstElement(&theConnectivityAdmin->Writers);

	while(iterator)
	{
		wfacade = in_connectivityWriterFacade(Coll_Iter_getObject(iterator));
		iterator = Coll_Iter_getNext(iterator);
		Coll_Set_remove(&theConnectivityAdmin->Writers, wfacade);
		in_connectivityWriterFacadeFree(wfacade);

	}
	iterator = Coll_Set_getFirstElement(&theConnectivityAdmin->PeerParticipants);

	while(iterator)
	{
		ppeer = in_connectivityPeerParticipant(Coll_Iter_getObject(iterator));
		iterator = Coll_Iter_getNext(iterator);
		Coll_Set_remove(&theConnectivityAdmin->PeerParticipants, ppeer);
		in_connectivityPeerParticipantFree(ppeer);

	}
	iterator = Coll_Set_getFirstElement(&theConnectivityAdmin->PeerReaders);

	while(iterator)
	{
		rpeer = in_connectivityPeerReader(Coll_Iter_getObject(iterator));
		iterator = Coll_Iter_getNext(iterator);
		Coll_Set_remove(&theConnectivityAdmin->PeerReaders, rpeer);
		in_connectivityPeerReaderFree(rpeer);
	}
	iterator = Coll_Set_getFirstElement(&theConnectivityAdmin->PeerWriters);

	while(iterator)
	{
		wpeer = in_connectivityPeerWriter(Coll_Iter_getObject(iterator));
		iterator = Coll_Iter_getNext(iterator);
		Coll_Set_remove(&theConnectivityAdmin->PeerWriters, wpeer);
		in_connectivityPeerWriterFree(wpeer);
	}
    os_mutexDestroy(&(theConnectivityAdmin->mutex));
    in_objectDeinit(theConnectivityAdmin);
    theConnectivityAdmin = NULL;
}


static os_boolean
in_connectivityAdminInit(
    in_connectivityAdmin _this)
{
    os_boolean success= OS_TRUE;
    os_mutexAttr mutexAttr;
    os_result resultmutex;
    assert(_this);

    success = in_objectInit(
        OS_SUPER(_this),
        IN_OBJECT_KIND_CONNECTIVITY_ADMIN,
        in_connectivityAdminDeinit);

    if( success ) {
        Coll_Set_init(&_this->Participants,     pointerIsLessThen, OS_TRUE);
        Coll_Set_init(&_this->Readers,          pointerIsLessThen, OS_TRUE);
        Coll_Set_init(&_this->Writers,          pointerIsLessThen, OS_TRUE);
        Coll_Set_init(&_this->PeerParticipants, pointerIsLessThen, OS_TRUE);
        Coll_Set_init(&_this->PeerReaders,      pointerIsLessThen, OS_TRUE);
        Coll_Set_init(&_this->PeerWriters,      pointerIsLessThen, OS_TRUE);


        mutexAttr.scopeAttr = OS_SCOPE_PRIVATE;
        resultmutex = os_mutexInit(&(_this->mutex), &mutexAttr);
        if(resultmutex != os_resultSuccess)
        {
            success = OS_FALSE;
        }
    }
    return success;
}



static in_connectivityAdmin
in_connectivityAdminNew(void)
{
    in_connectivityAdmin _this;

    _this = in_connectivityAdmin(os_malloc(sizeof(OS_STRUCT(in_connectivityAdmin))));

    if(_this)
    {
        in_connectivityAdminInit(_this);
    }

    IN_TRACE_1(Construction,2,"in_connectivityAdmin created = %x",_this);

    return _this;
}


/**
 * Determine wether a facade and a peer are compatible.
 */
static os_boolean
matchParticipant(
    in_connectivityParticipantFacade facade,
    in_connectivityPeerParticipant peer)
{
    /* Participants are always compatible */
   return OS_TRUE;
}


/**
 * Match a participant against all present Peers and
 * fill the matched participants set.
 */
static void
findMatchedPeerParticipants(
    in_connectivityAdmin _this,
    in_connectivityParticipantFacade facade)
{
    Coll_Iter* iterator;
    in_connectivityPeerParticipant knownpeer;

    /* go over all known peers to see if it matches */
    iterator = Coll_Set_getFirstElement(&_this->PeerParticipants);
    while(iterator)
    {
        knownpeer = in_connectivityPeerParticipant(Coll_Iter_getObject(iterator));
        if ( matchParticipant(facade,knownpeer)) {
            in_connectivityParticipantFacadeAddMatchedPeer(facade, knownpeer);
        } else {
            in_connectivityParticipantFacadeRemoveMatchedPeer(facade, knownpeer);
        }

        iterator = Coll_Iter_getNext(iterator);
    }
}

/**
 * Match a Peer against all present facades and
 * fill the matched participants set.
 */
static void
findMatchedParticipantFacades(
    in_connectivityAdmin _this,
    in_connectivityPeerParticipant peer)
{
    Coll_Iter* iterator;
    in_connectivityParticipantFacade facade;

    /* go over all facade's to find it */
    iterator = Coll_Set_getFirstElement(&_this->Participants);
    while(iterator)
    {
        facade = in_connectivityParticipantFacade(Coll_Iter_getObject(iterator));
        if ( matchParticipant(facade,peer)) {
            in_connectivityParticipantFacadeAddMatchedPeer(facade, peer);
        }
        iterator = Coll_Iter_getNext(iterator);
    }
}

static in_connectivityParticipantFacade
findParticipantFacadeByGuidUnsafe(
		in_connectivityAdmin _this, 
		in_ddsiGuidPrefixRef guidPrefixRef)
{
	Coll_Iter* iterator;
	
	in_connectivityParticipantFacade facade=NULL, result=NULL;
	in_ddsiGuidPrefixRef facadesGuidRef = NULL;
	
	/* go over all facade's to find it */
	iterator = Coll_Set_getFirstElement(&_this->Participants);
	while(iterator && !result)
	{
		facade = in_connectivityParticipantFacade(Coll_Iter_getObject(iterator));
		facadesGuidRef = in_connectivityParticipantFacadeGetGuidPrefix(facade);
		
		if ( in_ddsiGuidPrefixEqual(guidPrefixRef, facadesGuidRef)) {
			result = facade;
		}
		iterator = Coll_Iter_getNext(iterator);
	}	

	return result; /* may be NULL */ 
}

/**
 * Determine wether a facade and a peer are compatible.
 * This should include a check on topic and Qos.

static os_boolean
matchPeerWriter(
    in_connectivityReaderFacade facade,
    in_connectivityPeerWriter peer)
{
   if ( c_compareString(in_connectivityReaderFacadeGetInfo(facade)->topic_name,
                        in_connectivityPeerWriterGetInfo(peer)->topicData.info.topic_name) != C_EQ ){
      return OS_FALSE;
   }

   IN_TRACE_2(Connectivity,2,"matchPeerWriter (%x %x) MATCHED",facade,  peer);

   return OS_TRUE;
}
*/

/**
 * Determine wether a facade and a peer are compatible.
 * This should include a check on topic and Qos.

static os_boolean
matchPeerReader(
    in_connectivityWriterFacade facade,
    in_connectivityPeerReader peer)
{
	c_long nofPartitions;

    if ( c_compareString(in_connectivityWriterFacadeGetInfo(facade)->topic_name,
                         in_connectivityPeerReaderGetInfo(peer)->topicData.info.topic_name) != C_EQ ){
       return OS_FALSE;
    }


    IN_TRACE_2(Connectivity,2,"matchPeerReader (%x %x) MATCHED",facade,  peer);

    return OS_TRUE;
}
*/

/**
 * Match a reader against all present Peers and
 * fill the matched readers set.
 */
static void
findMatchedPeerWriters(
    in_connectivityAdmin _this,
    in_connectivityReaderFacade facade)
{
    Coll_Iter* iterator;
    in_connectivityPeerWriter knownpeer;

    /* go over all known peers to see if it matches */
    iterator = Coll_Set_getFirstElement(&_this->PeerWriters);
    while(iterator)
    {
        knownpeer = in_connectivityPeerWriter(Coll_Iter_getObject(iterator));
        if ( in_connectivityReaderFacadeMatchesPeerWriter(facade,knownpeer)) {
            in_connectivityReaderFacadeAddMatchedPeer(facade, knownpeer);
        } else {
            in_connectivityReaderFacadeRemoveMatchedPeer(facade, knownpeer);
        }

        iterator = Coll_Iter_getNext(iterator);
    }
}

/**
 * Match a Peer against all present facades and
 * fill the matched readers set.
 */
static void
findMatchedReaderFacades(
    in_connectivityAdmin _this,
    in_connectivityPeerWriter peer)
{
    Coll_Iter* iterator;
    in_connectivityReaderFacade facade;

    /* go over all facade's to find it */
    iterator = Coll_Set_getFirstElement(&_this->Readers);
    while(iterator)
    {
        facade = in_connectivityReaderFacade(Coll_Iter_getObject(iterator));
        if ( in_connectivityReaderFacadeMatchesPeerWriter(facade,peer)) {
            in_connectivityReaderFacadeAddMatchedPeer(facade, peer);
        }
        iterator = Coll_Iter_getNext(iterator);
    }
}




/**
 * Match a writer against all present Peers and
 * fill the matched writers set.
 */
static void
findMatchedPeerReaders(
    in_connectivityAdmin _this,
    in_connectivityWriterFacade facade)
{
    Coll_Iter* iterator;
    in_connectivityPeerReader knownpeer;

    /* go over all known peers to see if it matches */
    iterator = Coll_Set_getFirstElement(&_this->PeerReaders);
    while(iterator)
    {
        knownpeer = in_connectivityPeerReader(Coll_Iter_getObject(iterator));
        if ( in_connectivityWriterFacadeMatchesPeerReader(facade,knownpeer)) {
            in_connectivityWriterFacadeAddMatchedPeer(facade, knownpeer);
        } else {
            in_connectivityWriterFacadeRemoveMatchedPeer(facade, knownpeer);
        }

        iterator = Coll_Iter_getNext(iterator);
    }
}

/**
 * Match a Peer against all present facades and
 * fill the matched writers set.
 */
static void
findMatchedWriterFacades(
    in_connectivityAdmin _this,
    in_connectivityPeerReader peer)
{
    Coll_Iter* iterator;
    in_connectivityWriterFacade facade;

    /* go over all facade's to find it */
    iterator = Coll_Set_getFirstElement(&_this->Writers);
    while(iterator)
    {
        facade = in_connectivityWriterFacade(Coll_Iter_getObject(iterator));
        if ( in_connectivityWriterFacadeMatchesPeerReader(facade,peer)) {
            in_connectivityWriterFacadeAddMatchedPeer(facade, peer);
        }
        iterator = Coll_Iter_getNext(iterator);
    }
}




/* ------------- public ----------------------- */

in_connectivityAdmin
in_connectivityAdminGetInstance(void)
{ /* TODO protect! if (undefined) { lock; if (still undefined) { newAdmin;  } unlock;*/
    if (theConnectivityAdmin == NULL ) {

       theConnectivityAdmin = in_connectivityAdminNew();
    }
    return theConnectivityAdmin;
}

os_boolean 
in_connectivityAdminIsLocalEntity(
	in_connectivityAdmin _this,
	in_ddsiGuidPrefixRef guidPrefixRef)
{
	os_boolean result = OS_FALSE;
	in_connectivityParticipantFacade found = NULL;
	
	in_connectivityAdminLock(_this);
	
	/* this operation does not increment the refcounter, so you 
	 * must not free the object afterwards */ 
	found = findParticipantFacadeByGuidUnsafe(_this, guidPrefixRef);

	result = found!=NULL;
	
	in_connectivityAdminUnlock(_this);

	return found!=NULL;
}

void
in_connectivityAdminLock(
    in_connectivityAdmin _this)
{
    assert(_this);
    os_mutexLock(&(_this->mutex));

    IN_TRACE(Connectivity,2,"in_connectivityAdminLock()");
}

void
in_connectivityAdminUnlock(
    in_connectivityAdmin _this)
{
    assert(_this);
    os_mutexUnlock(&(_this->mutex));

    IN_TRACE(Connectivity,2,"in_connectivityAdminUnlock()");
}

in_result
in_connectivityAdminAddParticipant(
    in_connectivityAdmin _this,
    struct v_participantInfo *participant)
{
    Coll_Iter* iterator;
    in_connectivityParticipantFacade facade;
    os_boolean found = OS_FALSE;
    in_result result = IN_RESULT_OK;

    os_mutexLock(&(_this->mutex));

    /* go over all facade's to see if it is already present */
    iterator = Coll_Set_getFirstElement(&_this->Participants);
    while(iterator)
    {
        facade = in_connectivityParticipantFacade(Coll_Iter_getObject(iterator));
        /* This should be replaced by an identity match */
        /*
        if ( v_gidCompare(in_connectivityParticipantFacadeGetInfo(facade)->key,
                          participant->key) == C_EQ ) {
            *(in_connectivityParticipantFacadeGetInfo(facade)) = *participant;
            found = OS_TRUE;
            result = IN_RESULT_ALREADY_EXISTS;
        }
        */

        /* SHORTCUT: for now, all participants will map onto the same facade */
        found = OS_TRUE;
        result = IN_RESULT_ALREADY_EXISTS;

        iterator = Coll_Iter_getNext(iterator);
    }

    if ( !found ) {
        /* create new facade and insert it */
        facade = in_connectivityParticipantFacadeNew(participant);
        Coll_Set_add(&_this->Participants, facade);

        /* Participant matching is trivial, all match to all */
        findMatchedPeerParticipants(_this, facade);
    }

    os_mutexUnlock(&(_this->mutex));

    IN_TRACE_2(Connectivity,2,"in_connectivityAdminAddParticipant(%x) result = %d",participant,  result);

    return result;
}

in_result
in_connectivityAdminAddReader(
    in_connectivityAdmin _this,
    struct v_subscriptionInfo *reader,
    os_boolean hasKey)
{
    Coll_Iter* iterator;
    in_connectivityReaderFacade facade;
    os_boolean found = OS_FALSE;
    in_result result = IN_RESULT_OK;

    os_mutexLock(&(_this->mutex));

    /* go over all facade's to see if it is already present */
    iterator = Coll_Set_getFirstElement(&_this->Readers);
    while(iterator)
    {
        facade = in_connectivityReaderFacade(Coll_Iter_getObject(iterator));
        /* This should be replaced by an identity match */
        if ( v_gidCompare(in_connectivityReaderFacadeGetInfo(facade)->key,
                          reader->key) == C_EQ ) {
            /* Copy updated info into the Facade */
            *(in_connectivityReaderFacadeGetInfo(facade)) = *reader;
            found = OS_TRUE;
            result = IN_RESULT_ALREADY_EXISTS;
        }
        iterator = Coll_Iter_getNext(iterator);
    }

    if ( !found ) {
        in_connectivityParticipantFacade  participant;
        in_ddsiSequenceNumber             seq;

        /* find participant and increment sequencenumber */
        participant = in_connectivityAdminFindParticipantUnsafe(_this, reader->participant_key);
        seq = in_connectivityParticipantFacadeIncReader(participant);

        /* create new facade and insert it */
        facade = in_connectivityReaderFacadeNew(reader, hasKey, seq, participant);
        Coll_Set_add(&_this->Readers, facade);

        findMatchedPeerWriters(_this, facade);
        in_connectivityParticipantFacadeFree(participant);
    }

    os_mutexUnlock(&(_this->mutex));

    IN_TRACE_2(Connectivity,2,"in_connectivityAdminAddReader(%x) result = %d",reader,  result);

    return result;
}

in_result
in_connectivityAdminAddWriter(
    in_connectivityAdmin _this,
    struct v_publicationInfo *writer,
    os_boolean hasKey)
{
    Coll_Iter* iterator;
    in_connectivityWriterFacade facade;
    in_connectivityParticipantFacade  participant;
    in_ddsiSequenceNumber seq;
    os_boolean found = OS_FALSE;
    in_result result = IN_RESULT_OK;

    os_mutexLock(&(_this->mutex));
    /* go over all facade's to see if it is already present */
    iterator = Coll_Set_getFirstElement(&_this->Writers);

    while(iterator)
    {
        facade = in_connectivityWriterFacade(Coll_Iter_getObject(iterator));
        /* This should be replaced by an identity match */
        if ( v_gidCompare(in_connectivityWriterFacadeGetInfo(facade)->key,
                          writer->key) == C_EQ )
        {
            /* Copy updated info into the Facade */
            *(in_connectivityWriterFacadeGetInfo(facade)) = *writer;
            found = OS_TRUE;
            result = IN_RESULT_ALREADY_EXISTS;
        }
        iterator = Coll_Iter_getNext(iterator);
    }
    if ( !found )
    {
        /* find participant and increment sequencenumber */
        participant = in_connectivityAdminFindParticipantUnsafe(_this, writer->participant_key);
        seq = in_connectivityParticipantFacadeIncWriter(participant);

        /* create new facade and insert it */
        facade = in_connectivityWriterFacadeNew(writer,hasKey, seq, participant);
        Coll_Set_add(&_this->Writers, facade);

        findMatchedPeerReaders(_this, facade);
        in_connectivityParticipantFacadeFree(participant);
    }
    os_mutexUnlock(&(_this->mutex));

    return result;
}


in_connectivityParticipantFacade
in_connectivityAdminGetParticipant(
    in_connectivityAdmin _this,
    struct v_participantInfo *participant)
{
    Coll_Iter* iterator;
    in_connectivityParticipantFacade facade;

    os_mutexLock(&(_this->mutex));

    /* go over all facade's to find it */
    iterator = Coll_Set_getFirstElement(&_this->Participants);
    while(iterator)
    {
        facade = in_connectivityParticipantFacade(Coll_Iter_getObject(iterator));
        /*
        if ( v_gidCompare(in_connectivityParticipantFacadeGetInfo(facade)->key,
                          participant->key) == C_EQ ) {
            return in_connectivityParticipantFacade(in_objectKeep(in_object(facade)));
        }
        */

        /* SHORTCUT: for now all participant map to the same facade */
        os_mutexUnlock(&(_this->mutex));
        return in_connectivityParticipantFacadeKeep(facade);


        iterator = Coll_Iter_getNext(iterator);
    }

    os_mutexUnlock(&(_this->mutex));

    return NULL;
}

in_connectivityReaderFacade
in_connectivityAdminGetReader(
    in_connectivityAdmin _this,
    struct v_subscriptionInfo *reader)
{
    Coll_Iter* iterator;
    in_connectivityReaderFacade facade;

    os_mutexLock(&(_this->mutex));

    /* go over all facade's to find it */
    iterator = Coll_Set_getFirstElement(&_this->Readers);
    while(iterator)
    {
        facade = in_connectivityReaderFacade(Coll_Iter_getObject(iterator));
        /* This should be replaced by an identity match */
        if ( v_gidCompare(in_connectivityReaderFacadeGetInfo(facade)->key,
                          reader->key) == C_EQ ) {
            os_mutexUnlock(&(_this->mutex));
            return in_connectivityReaderFacadeKeep(facade);
        }
        iterator = Coll_Iter_getNext(iterator);
    }

    os_mutexUnlock(&(_this->mutex));

    return NULL;
}

in_connectivityWriterFacade
in_connectivityAdminGetWriter(
    in_connectivityAdmin _this,
    struct v_publicationInfo *writer)
{
    Coll_Iter* iterator;
    in_connectivityWriterFacade facade;

    os_mutexLock(&(_this->mutex));

    /* go over all facade's to find it */
    iterator = Coll_Set_getFirstElement(&_this->Writers);
    while(iterator)
    {
        facade = in_connectivityWriterFacade(Coll_Iter_getObject(iterator));
        /* This should be replaced by an identity match */
        if ( v_gidCompare(in_connectivityWriterFacadeGetInfo(facade)->key,
                          writer->key) == C_EQ ) {
            os_mutexUnlock(&(_this->mutex));
            return in_connectivityWriterFacadeKeep(facade);
        }
        iterator = Coll_Iter_getNext(iterator);
    }

    os_mutexUnlock(&(_this->mutex));

    return NULL;
}

in_connectivityWriterFacade
in_connectivityAdminFindWriter(
    in_connectivityAdmin _this,
    v_message message)
{
    Coll_Iter* iterator;
    in_connectivityWriterFacade facade;

    os_mutexLock(&(_this->mutex));

    /* go over all facade's to find it */
    iterator = Coll_Set_getFirstElement(&_this->Writers);
    while(iterator)
    {

        facade = in_connectivityWriterFacade(Coll_Iter_getObject(iterator));
        /* This should be replaced by an identity match */
        if ( v_gidCompare(in_connectivityWriterFacadeGetInfo(facade)->key,
                          message->writerGID) == C_EQ ) {
            IN_TRACE_1(Send, 2, ">>> in_connectivityAdminFindWriter - found %p", facade);
            os_mutexUnlock(&(_this->mutex));
            return in_connectivityWriterFacadeKeep(facade);
        }
        iterator = Coll_Iter_getNext(iterator);
    }

    os_mutexUnlock(&(_this->mutex));

    return NULL;
}

in_connectivityParticipantFacade
in_connectivityAdminFindParticipant(
    in_connectivityAdmin _this,
    v_gid entity_id
    )
{
    in_connectivityParticipantFacade facade;

    os_mutexLock(&(_this->mutex));

    facade = in_connectivityAdminFindParticipantUnsafe(_this, entity_id);

    os_mutexUnlock(&(_this->mutex));

    return facade;
}

in_connectivityParticipantFacade
in_connectivityAdminFindParticipantUnsafe(
    in_connectivityAdmin _this,
    v_gid entity_id)
{
    Coll_Iter* iterator;
    in_connectivityParticipantFacade facade;

    /* go over all facade's to find it */
    iterator = Coll_Set_getFirstElement(&_this->Participants);
    while(iterator)
    {
        facade = in_connectivityParticipantFacade(Coll_Iter_getObject(iterator));

        /* SHORTCUT: for now all participant map to the same facade */
        return in_connectivityParticipantFacadeKeep(facade);


        iterator = Coll_Iter_getNext(iterator);
    }

    return NULL;
}

static in_connectivityPeerParticipant
in_connectivityAdminFindPeerParticipantUnsafe(
    in_connectivityAdmin _this,
    in_ddsiGuidPrefix prefix)
{
    Coll_Iter* iterator;
    in_connectivityPeerParticipant peer;

    /* go over all facade's to find it */
    iterator = Coll_Set_getFirstElement(&_this->PeerParticipants);
    while(iterator)
    {
        peer = in_connectivityPeerParticipant(Coll_Iter_getObject(iterator));

        if ( in_ddsiGuidPrefixEqual(prefix, in_connectivityPeerParticipantGetGuidPrefix(peer)))
        {
            return in_connectivityPeerParticipantKeep(peer);
        }

        iterator = Coll_Iter_getNext(iterator);
    }
    return NULL;
}

in_connectivityPeerParticipant
in_connectivityAdminFindPeerParticipant(
    in_connectivityAdmin _this,
    in_ddsiGuidPrefix prefix)
{
    in_connectivityPeerParticipant peer;

    os_mutexLock(&(_this->mutex));
    peer = in_connectivityAdminFindPeerParticipantUnsafe(_this, prefix);
    os_mutexUnlock(&(_this->mutex));

    return peer;
}



in_result
in_connectivityAdminRemoveParticipant(
    in_connectivityAdmin _this,
    struct v_participantInfo *participant)
{
    Coll_Iter* iterator;
    in_connectivityParticipantFacade facade;
    in_result result = IN_RESULT_NOT_FOUND;


    /* SHORTCUT: for now, all participants will map onto the same facade */
    /* So we will never remove it */
    return IN_RESULT_OK;

    os_mutexLock(&(_this->mutex));

    /* go over all facade's to find it */
    iterator = Coll_Set_getFirstElement(&_this->Participants);
    while(iterator)
    {
        facade = in_connectivityParticipantFacade(Coll_Iter_getObject(iterator));
        /* This should be replaced by an identity match */
        if ( v_gidCompare(in_connectivityParticipantFacadeGetInfo(facade)->key,
                          participant->key) == C_EQ ) {

            /*jump to next element, before removing object*/
            iterator = Coll_Iter_getNext(iterator);

            /* remove this facade from the collection*/
            Coll_Set_remove(&_this->Participants, facade);

            /* free the facade */
            in_connectivityParticipantFacadeFree(facade);

            result = IN_RESULT_OK;
        } else {
            iterator = Coll_Iter_getNext(iterator);
        }
    }

    os_mutexUnlock(&(_this->mutex));

    return result;
}

in_result
in_connectivityAdminRemoveReader(
    in_connectivityAdmin _this,
    struct v_subscriptionInfo *reader)
{
    Coll_Iter* iterator;
    in_connectivityReaderFacade facade;
    in_result result = IN_RESULT_NOT_FOUND;

    os_mutexLock(&(_this->mutex));

    /* go over all facade's to find it */
    iterator = Coll_Set_getFirstElement(&_this->Readers);
    while(iterator)
    {
        facade = in_connectivityReaderFacade(Coll_Iter_getObject(iterator));
        /* This should be replaced by an identity match */
        if ( v_gidCompare(in_connectivityReaderFacadeGetInfo(facade)->key,
                          reader->key) == C_EQ ) {
            /*jump to next element, before removing object*/
            iterator = Coll_Iter_getNext(iterator);

            /* remove this facade from the collection*/
            Coll_Set_remove(&_this->Readers, facade);

            /* free the facade */
            in_connectivityReaderFacadeFree(facade);

            result = IN_RESULT_OK;
        } else {
            iterator = Coll_Iter_getNext(iterator);
        }
    }

    os_mutexUnlock(&(_this->mutex));

    return result;
}

in_result
in_connectivityAdminRemoveWriter(
    in_connectivityAdmin _this,
    struct v_publicationInfo *writer)
{
    Coll_Iter* iterator;
    in_connectivityWriterFacade facade;
    in_result result = IN_RESULT_NOT_FOUND;

    os_mutexLock(&(_this->mutex));

    /* go over all facade's to find it */
    iterator = Coll_Set_getFirstElement(&_this->Writers);
    while(iterator)
    {
        facade = in_connectivityWriterFacade(Coll_Iter_getObject(iterator));
        /* This should be replaced by an identity match */
        if ( v_gidCompare(in_connectivityWriterFacadeGetInfo(facade)->key,
                          writer->key) == C_EQ ) {
            /*jump to next element, before removing object*/
            iterator = Coll_Iter_getNext(iterator);

            /* remove this facade from the collection*/
            Coll_Set_remove(&_this->Writers, facade);

            /* free the facade */
            in_connectivityWriterFacadeFree(facade);

            result = IN_RESULT_OK;
        } else  {
            iterator = Coll_Iter_getNext(iterator);
        }
    }

    os_mutexUnlock(&(_this->mutex));

    return result;
}


in_result
in_connectivityAdminAddPeerParticipant(
    in_connectivityAdmin _this,
    in_connectivityPeerParticipant participant)
{
    Coll_Iter* iterator;
    in_connectivityPeerParticipant knownpeer;
    os_boolean found = OS_FALSE;
    in_result result = IN_RESULT_OK;

    os_mutexLock(&(_this->mutex));

    /* go over all known peers to see if it is already present */
    iterator = Coll_Set_getFirstElement(&_this->PeerParticipants);
    while(iterator && !found)
    {
        knownpeer = in_connectivityPeerParticipant(Coll_Iter_getObject(iterator));
        if ( in_ddsiGuidPrefixEqual(in_connectivityPeerParticipantGetGuidPrefix(participant),
                              in_connectivityPeerParticipantGetGuidPrefix(knownpeer))) {
                              /* or should we update? */
            result = IN_RESULT_ALREADY_EXISTS;
            found = OS_TRUE;
        }
        iterator = Coll_Iter_getNext(iterator);
    }

    if ( !found ) { /* TODO: verify the sequence number, if newer than replace existing one*/
        /* create new facade and insert it */
        Coll_Set_add(&_this->PeerParticipants, in_connectivityPeerParticipantKeep(participant));

        findMatchedParticipantFacades(_this, participant);
    }


    IN_TRACE_2(Connectivity,2,"in_connectivityAdminAddPeerParticipant(%x) result = %d",
               participant,  result);

    os_mutexUnlock(&(_this->mutex));

    return result;
}


in_result
in_connectivityAdminAddPeerReader(
    in_connectivityAdmin _this,
    in_connectivityPeerReader reader,
    in_ddsiSequenceNumber seq)
{
    Coll_Iter* iterator;
    in_connectivityPeerReader knownpeer;
    os_boolean found = OS_FALSE;
    in_result result = IN_RESULT_OK;
    os_uint32 errorCode;
    in_ddsiGuid guid;
    in_connectivityPeerParticipant participant;
    Coll_List* unicastLocators;
    Coll_List* multicastLocators;
    Coll_List* partiticpantLocators;

    os_mutexLock(&(_this->mutex));

    /* go over all known peers to see if it is already present */
    iterator = Coll_Set_getFirstElement(&_this->PeerReaders);
    while(iterator && !found)
    {
        knownpeer = in_connectivityPeerReader(Coll_Iter_getObject(iterator));
        if ( in_ddsiGuidEqual(in_connectivityPeerReaderGetGuid(reader),
                              in_connectivityPeerReaderGetGuid(knownpeer))) {
                              /* or should we update? */
            result = IN_RESULT_ALREADY_EXISTS;
            found = OS_TRUE;
        }
        iterator = Coll_Iter_getNext(iterator);
    }

    /* find participant and add sequence number */
    guid = in_connectivityPeerReaderGetGuid(reader);
    participant = in_connectivityAdminFindPeerParticipantUnsafe(_this,guid->guidPrefix);
	assert(guid!=NULL);

    /* just process peer-entities of known peer participants, otherwise reply endpoints
     * may be undefined */
    if ( !found && participant )
    {
        unicastLocators = in_connectivityPeerReaderGetUnicastLocators(reader);
        multicastLocators = in_connectivityPeerReaderGetMulticastLocators(reader);
        if(Coll_List_getNrOfElements(unicastLocators) == 0 && Coll_List_getNrOfElements(multicastLocators) == 0)
        {
            partiticpantLocators = in_connectivityPeerParticipantGetDefaultUnicastLocators(participant);
            result = in_connectivityAdminCopyLocators(partiticpantLocators, unicastLocators);

            if(result == IN_RESULT_OK)
            {
                partiticpantLocators = in_connectivityPeerParticipantGetDefaultMulticastLocators(participant);
                result = in_connectivityAdminCopyLocators(partiticpantLocators, multicastLocators);
            }
        }
        if(result == IN_RESULT_OK)
        {
            /* create new facade and insert it */
            errorCode = Coll_Set_add(&_this->PeerReaders, in_connectivityPeerReaderKeep(reader));
            if(errorCode == COLL_OK)
            {
                findMatchedWriterFacades(_this, reader);

                assert(participant);

                in_connectivityPeerParticipantAddReaderSN(participant,seq);
            } else
            {
                result = IN_RESULT_ERROR;
            }
        }
        in_connectivityPeerParticipantFree(participant);
    } else if (participant) {
    	in_connectivityPeerParticipantFree(participant);
    } else {
    	/* corresponding peer participant not found */
    	result = IN_RESULT_NOT_FOUND;
    }

    IN_TRACE_2(Connectivity,2,"in_connectivityAdminAddPeerReader(%x) result = %d",
               reader,  result);

    os_mutexUnlock(&(_this->mutex));

    return result;
}

in_result
in_connectivityAdminCopyLocators(
    Coll_List* source,
    Coll_List* target)
{
    Coll_Iter* iterator;
    in_locator locator;
    os_uint32 errorCode;
    in_result result = IN_RESULT_OK;

    iterator = Coll_List_getFirstElement(source);
    while(iterator)
    {
        locator = in_locator(Coll_Iter_getObject(iterator));
        errorCode = Coll_List_pushBack(target, in_locatorClone(locator));
        if(errorCode != COLL_OK)
        {
            result = IN_RESULT_OUT_OF_MEMORY;
        }
        iterator = Coll_Iter_getNext(iterator);
    }
    return result;
}


in_result
in_connectivityAdminAddPeerWriter(
    in_connectivityAdmin _this,
    in_connectivityPeerWriter writer,
    in_ddsiSequenceNumber seq)
{
    Coll_Iter* iterator;
    in_connectivityPeerWriter knownpeer;
    os_boolean found = OS_FALSE;
    in_result result = IN_RESULT_OK;
    Coll_List* unicastLocators;
    Coll_List* multicastLocators;
    Coll_List* partiticpantLocators;
    os_uint32 errorCode;
    in_ddsiGuid guid = NULL;
    in_connectivityPeerParticipant participant = NULL;

    os_mutexLock(&(_this->mutex));

    /* go over all known peers to see if it is already present */
    iterator = Coll_Set_getFirstElement(&_this->PeerWriters);
    while(iterator && !found)
    {
        knownpeer = in_connectivityPeerWriter(Coll_Iter_getObject(iterator));
        if ( in_ddsiGuidEqual(in_connectivityPeerWriterGetGuid(writer),
                              in_connectivityPeerWriterGetGuid(knownpeer))) {
                              /* or should we update? */
            result = IN_RESULT_ALREADY_EXISTS;
            found = OS_TRUE;
        }
        iterator = Coll_Iter_getNext(iterator);
    }

    /* find participant and add sequence number */
    guid = in_connectivityPeerWriterGetGuid(writer);
    participant = in_connectivityAdminFindPeerParticipantUnsafe(_this,guid->guidPrefix);

    assert(guid!=NULL);

    if ( !found  && participant) {
        unicastLocators = in_connectivityPeerWriterGetUnicastLocators(writer);
        multicastLocators = in_connectivityPeerWriterGetMulticastLocators(writer);
        if(Coll_List_getNrOfElements(unicastLocators) == 0 && Coll_List_getNrOfElements(multicastLocators) == 0)
        {
            partiticpantLocators = in_connectivityPeerParticipantGetDefaultUnicastLocators(participant);
            result = in_connectivityAdminCopyLocators(partiticpantLocators, unicastLocators);

            if(result == IN_RESULT_OK)
            {
                partiticpantLocators = in_connectivityPeerParticipantGetDefaultMulticastLocators(participant);
                result = in_connectivityAdminCopyLocators(partiticpantLocators, multicastLocators);
            }
        }
        if(result == IN_RESULT_OK)
        {
            errorCode = Coll_Set_add(&_this->PeerWriters, in_connectivityPeerWriterKeep(writer));
            if(errorCode == COLL_OK)
            {
                /* create new facade and insert it */
                findMatchedReaderFacades(_this, writer);
                in_connectivityPeerParticipantAddWriterSN(participant,seq);
            } else
            {
                result = IN_RESULT_ERROR;
            }
        }
        in_connectivityPeerParticipantFree(participant);
    } else if (participant) {
    	in_connectivityPeerParticipantFree(participant);
    } else {
    	/* corresponding peer participant not found */
    	result = IN_RESULT_NOT_FOUND;
    }

    IN_TRACE_2(Connectivity,2,"in_connectivityAdminAddPeerWriter(%x) result = %d",
               writer,  result);

    os_mutexUnlock(&(_this->mutex));

    return result;
}

in_connectivityPeerParticipant
in_connectivityAdminGetPeerParticipant(
    in_connectivityAdmin _this,
    in_ddsiGuidPrefix guidPrefix)
{
    in_connectivityPeerParticipant result;

     os_mutexLock(&(_this->mutex));

    result = in_connectivityAdminGetPeerParticipantUnsafe(_this,guidPrefix);

    os_mutexUnlock(&(_this->mutex));

    return result;
}

in_connectivityPeerParticipant
in_connectivityAdminGetPeerParticipantUnsafe(
    in_connectivityAdmin _this,
    in_ddsiGuidPrefix guidPrefix)
{
    Coll_Iter* iterator;
    in_connectivityPeerParticipant knownpeer;

    /* go over all known peers to see if it is present */
    iterator = Coll_Set_getFirstElement(&_this->PeerParticipants);
    while(iterator)
    {
        knownpeer = in_connectivityPeerParticipant(Coll_Iter_getObject(iterator));
        if ( in_ddsiGuidPrefixEqual(guidPrefix,in_connectivityPeerParticipantGetGuidPrefix(knownpeer))) {
            return in_connectivityPeerParticipantKeep(knownpeer);
        }
        iterator = Coll_Iter_getNext(iterator);
    }

    return NULL;
}


in_connectivityPeerReader
in_connectivityAdminGetPeerReader(
    in_connectivityAdmin _this,
    in_ddsiGuid guid)
{
    Coll_Iter* iterator;
    in_connectivityPeerReader knownpeer;

    os_mutexLock(&(_this->mutex));

    /* go over all known peers to see if it is present */
    iterator = Coll_Set_getFirstElement(&_this->PeerReaders);
    while(iterator)
    {
        knownpeer = in_connectivityPeerReader(Coll_Iter_getObject(iterator));
        if ( in_ddsiGuidEqual(guid,in_connectivityPeerReaderGetGuid(knownpeer))) {
            os_mutexUnlock(&(_this->mutex));
            return in_connectivityPeerReaderKeep(knownpeer);
        }
        iterator = Coll_Iter_getNext(iterator);
    }

    os_mutexUnlock(&(_this->mutex));

    return NULL;
}

in_connectivityPeerWriter
in_connectivityAdminGetPeerWriter(
    in_connectivityAdmin _this,
    in_ddsiGuid guid)
{
    Coll_Iter* iterator;
    in_connectivityPeerWriter knownpeer;

    os_mutexLock(&(_this->mutex));

    /* go over all known peers to see if it is present */
    iterator = Coll_Set_getFirstElement(&_this->PeerWriters);
    while(iterator)
    {
        knownpeer = in_connectivityPeerWriter(Coll_Iter_getObject(iterator));
        if ( in_ddsiGuidEqual(guid,in_connectivityPeerWriterGetGuid(knownpeer))) {
            os_mutexUnlock(&(_this->mutex));
            return in_connectivityPeerWriterKeep(knownpeer);
        }
        iterator = Coll_Iter_getNext(iterator);
    }

    os_mutexUnlock(&(_this->mutex));

    return NULL;
}

/*TODO: check whether this method is really necessary*/
Coll_Set*
in_connectivityAdminGetPeerParticipantsUnsafe(
    in_connectivityAdmin _this)
{
    assert(_this);

    return &(_this->PeerParticipants);
}


in_result
in_connectivityAdminRemovePeerParticipant(
    in_connectivityAdmin _this,
    in_ddsiGuidPrefix guidPrefix)
{
    Coll_Iter *iterator, *peeriterator;
    in_connectivityPeerParticipant knownpeer;
    in_connectivityParticipantFacade facade;
    in_result result = IN_RESULT_NOT_FOUND;

    os_mutexLock(&(_this->mutex));

    /* go over all known peers to see if it is present */
    iterator = Coll_Set_getFirstElement(&_this->PeerParticipants);
    while(iterator)
    {
        knownpeer = in_connectivityPeerParticipant(Coll_Iter_getObject(iterator));
        if ( in_ddsiGuidPrefixEqual(guidPrefix,in_connectivityPeerParticipantGetGuidPrefix(knownpeer))) {

            /*jump to next element, before removing object*/
            iterator = Coll_Iter_getNext(iterator);

            /* remove this peer from the collection*/
            Coll_Set_remove(&_this->PeerParticipants, knownpeer);

            /* go over all facade's to remove this peer */
            peeriterator = Coll_Set_getFirstElement(&_this->Participants);
            while(peeriterator)
            {
                facade = in_connectivityParticipantFacade(Coll_Iter_getObject(peeriterator));
                peeriterator = Coll_Iter_getNext(peeriterator);
                in_connectivityParticipantFacadeRemoveMatchedPeer(facade, knownpeer);
            }

            /* free the peer */
            in_connectivityPeerParticipantFree(knownpeer);

            result = IN_RESULT_OK;
        } else {
            iterator = Coll_Iter_getNext(iterator);
        }
    }

    os_mutexUnlock(&(_this->mutex));

    return result;
}

in_result
in_connectivityAdminRemovePeerReader(
    in_connectivityAdmin _this,
    in_ddsiGuid guid)
{
    Coll_Iter *iterator, *peeriterator;
    in_connectivityPeerReader knownpeer;
    in_connectivityWriterFacade facade;
    in_result result = IN_RESULT_NOT_FOUND;

    os_mutexLock(&(_this->mutex));

    /* go over all known peers to see if it is present */
    iterator = Coll_Set_getFirstElement(&_this->PeerReaders);
    while(iterator)
    {
        knownpeer = in_connectivityPeerReader(Coll_Iter_getObject(iterator));
        if ( in_ddsiGuidEqual(guid,in_connectivityPeerReaderGetGuid(knownpeer))) {

            /*jump to next element, before removing object*/
            iterator = Coll_Iter_getNext(iterator);

            /* remove this peer from the collection*/
            Coll_Set_remove(&_this->PeerReaders, knownpeer);

            /* go over all facade's to remove this peer */
            peeriterator = Coll_Set_getFirstElement(&_this->Writers);
            while(peeriterator)
            {
                facade = in_connectivityWriterFacade(Coll_Iter_getObject(peeriterator));
                peeriterator = Coll_Iter_getNext(peeriterator);
                in_connectivityWriterFacadeRemoveMatchedPeer(facade, knownpeer);
            }

            /* free the peer */
            in_connectivityPeerReaderFree(knownpeer);

            result = IN_RESULT_OK;
        } else {
            iterator = Coll_Iter_getNext(iterator);
        }
    }

    os_mutexUnlock(&(_this->mutex));

    return result;
}

in_result
in_connectivityAdminRemovePeerWriter(
    in_connectivityAdmin _this,
    in_ddsiGuid guid)
{
    Coll_Iter* iterator, *peeriterator;
    in_connectivityPeerWriter knownpeer;
    in_connectivityReaderFacade facade;
    in_result result = IN_RESULT_NOT_FOUND;

    os_mutexLock(&(_this->mutex));

    /* go over all known peers to see if it is present */
    iterator = Coll_Set_getFirstElement(&_this->PeerWriters);
    while(iterator)
    {
        knownpeer = in_connectivityPeerWriter(Coll_Iter_getObject(iterator));
        if ( in_ddsiGuidEqual(guid,in_connectivityPeerWriterGetGuid(knownpeer))) {

            /*jump to next element, before removing object*/
            iterator = Coll_Iter_getNext(iterator);

            /* remove this peer from the collection*/
            Coll_Set_remove(&_this->PeerWriters, knownpeer);

            /* go over all facade's to remove this peer */
            peeriterator = Coll_Set_getFirstElement(&_this->Readers);
            while(peeriterator)
            {
                facade = in_connectivityReaderFacade(Coll_Iter_getObject(peeriterator));
                peeriterator = Coll_Iter_getNext(peeriterator);
                in_connectivityReaderFacadeRemoveMatchedPeer(facade, knownpeer);

            }

            /* free the peer */
            in_connectivityPeerWriterFree(knownpeer);

            result = IN_RESULT_OK;
        } else {
            iterator = Coll_Iter_getNext(iterator);
        }
    }

    os_mutexUnlock(&(_this->mutex));

    return result;
}

Coll_Set*
in_connectivityAdminGetParticipants(
    in_connectivityAdmin _this)
{
    assert(_this);

    return &(_this->Participants);
}

Coll_Set*
in_connectivityAdminGetReaders(
    in_connectivityAdmin _this)
{
    assert(_this);

    return &(_this->Readers);
}


Coll_Set*
in_connectivityAdminGetWriters(
    in_connectivityAdmin _this)
{
    assert(_this);

    return &(_this->Writers);
}

in_result
in_connectivityAdminAddListener(
    in_connectivityAdmin _this,
    in_connectivityListener listener)
{
    return IN_RESULT_OK;
}

in_result
in_connectivityAdminRemoveListener(
    in_connectivityAdmin _this,
    in_connectivityListener listener)
{
    return IN_RESULT_OK;
}


