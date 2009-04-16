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
#include "in_connectivityAdmin.h"
#include "in_connectivityPeerWriter.h"
#include "in_connectivityWriterFacade.h"
#include "in_connectivityParticipantFacade.h"
#include "in__object.h"
#include "Coll_Compare.h"

#include "v_public.h"
#include "c_stringSupport.h"
#include "os_defs.h"
#include "os_heap.h"


OS_STRUCT(in_connectivityAdmin)
{
    OS_EXTENDS(in_object);
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
in_connectivityAdminDestroy(
    in_object _this)
{
    assert(_this);

    os_free(_this);
}


static void
in_connectivityAdminInit(
    in_connectivityAdmin _this)
{
    assert(_this);

    in_objectInit(
        OS_SUPER(_this),
        IN_OBJECT_KIND_CONNECTIVITY_ADMIN,
        in_connectivityAdminDestroy);

    Coll_Set_init(&_this->Participants,     pointerIsLessThen, TRUE);
    Coll_Set_init(&_this->Readers,          pointerIsLessThen, TRUE);
    Coll_Set_init(&_this->Writers,          pointerIsLessThen, TRUE);
    Coll_Set_init(&_this->PeerParticipants, pointerIsLessThen, TRUE);
    Coll_Set_init(&_this->PeerReaders,      pointerIsLessThen, TRUE);
    Coll_Set_init(&_this->PeerWriters,      pointerIsLessThen, TRUE);
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


static in_connectivityAdmin
in_connectivityAdminNew()
{
    in_connectivityAdmin _this;

    _this = in_connectivityAdmin(os_malloc(sizeof(OS_STRUCT(in_connectivityAdmin))));

    if(_this)
    {
        in_connectivityAdminInit(_this);
    }

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
   return TRUE;
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


/**
 * Determine wether a facade and a peer are compatible.
 * This should include a check on topic and Qos.
 */
static os_boolean
matchPeerWriter(
    in_connectivityReaderFacade facade,
    in_connectivityPeerWriter peer)
{
   if ( c_compareString(in_connectivityReaderFacadeGetInfo(facade)->topic_name,
                        in_connectivityPeerWriterGetInfo(peer)->topic_name) != C_EQ ){
      return FALSE;
   }
   /* TODO: QOS checks must be performed here */

   return TRUE;
}

/**
 * Determine wether a facade and a peer are compatible.
 * This should include a check on topic and Qos.
 */
static os_boolean
matchPeerReader(
    in_connectivityWriterFacade facade,
    in_connectivityPeerReader peer)
{
    if ( c_compareString(in_connectivityWriterFacadeGetInfo(facade)->topic_name,
                         in_connectivityPeerReaderGetInfo(peer)->topic_name) != C_EQ ){
       return FALSE;
    }
    /* TODO: QOS checks must be performed here */

    return TRUE;
}


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
        if ( matchPeerWriter(facade,knownpeer)) {
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
        if ( matchPeerWriter(facade,peer)) {
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
        if ( matchPeerReader(facade,knownpeer)) {
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
        if ( matchPeerReader(facade,peer)) {
            in_connectivityWriterFacadeAddMatchedPeer(facade, peer);
        }
        iterator = Coll_Iter_getNext(iterator);
    }
}




/* ------------- public ----------------------- */

in_connectivityAdmin
in_connectivityAdminGetInstance()
{
    if (theConnectivityAdmin == NULL ) {
       theConnectivityAdmin = in_connectivityAdminNew();
    }
    return theConnectivityAdmin;
}

in_result
in_connectivityAdminAddParticipant(
    in_connectivityAdmin _this,
    struct v_participantInfo *participant)
{
    Coll_Iter* iterator;
    in_connectivityParticipantFacade facade;
    os_boolean found = FALSE;
    in_result result = IN_RESULT_OK;

    /* go over all facade's to see if it is already present */
    iterator = Coll_Set_getFirstElement(&_this->Participants);
    while(iterator)
    {
        facade = in_connectivityParticipantFacade(Coll_Iter_getObject(iterator));
        /* This should be replaced by an identity match */
        if ( v_gidCompare(in_connectivityParticipantFacadeGetInfo(facade)->key,
                          participant->key) == C_EQ ) {
            /* Copy updated info into the Facade */
            *(in_connectivityParticipantFacadeGetInfo(facade)) = *participant;
            found = TRUE;
            result = IN_RESULT_ALREADY_EXISTS;
        }
        iterator = Coll_Iter_getNext(iterator);
    }

    if ( !found ) {
        /* create new facade and insert it */
        facade = in_connectivityParticipantFacadeNew(participant);
        Coll_Set_add(&_this->Participants, facade);

        /* Participant matching is trivial, all match to all */
        findMatchedPeerParticipants(_this, facade);
    }

    return result;
}

in_result
in_connectivityAdminAddReader(
    in_connectivityAdmin _this,
    struct v_subscriptionInfo *reader)
{
    Coll_Iter* iterator;
    in_connectivityReaderFacade facade;
    os_boolean found = FALSE;
    in_result result = IN_RESULT_OK;

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
            found = TRUE;
            result = IN_RESULT_ALREADY_EXISTS;
        }
        iterator = Coll_Iter_getNext(iterator);
    }

    if ( !found ) {
        /* create new facade and insert it */
        facade = in_connectivityReaderFacadeNew(reader);
        Coll_Set_add(&_this->Readers, facade);

        findMatchedPeerWriters(_this, facade);
    }

    return result;
}

in_result
in_connectivityAdminAddWriter(
    in_connectivityAdmin _this,
    struct v_publicationInfo *writer)
{
    Coll_Iter* iterator;
    in_connectivityWriterFacade facade;
    os_boolean found = FALSE;
    in_result result = IN_RESULT_OK;

    /* go over all facade's to see if it is already present */
    iterator = Coll_Set_getFirstElement(&_this->Writers);
    while(iterator)
    {
        facade = in_connectivityWriterFacade(Coll_Iter_getObject(iterator));
        /* This should be replaced by an identity match */
        if ( v_gidCompare(in_connectivityWriterFacadeGetInfo(facade)->key,
                          writer->key) == C_EQ ) {
            /* Copy updated info into the Facade */
            *(in_connectivityWriterFacadeGetInfo(facade)) = *writer;
            found = TRUE;
            result = IN_RESULT_ALREADY_EXISTS;
        }
        iterator = Coll_Iter_getNext(iterator);
    }

    if ( !found ) {
        /* create new facade and insert it */
        facade = in_connectivityWriterFacadeNew(writer);
        Coll_Set_add(&_this->Writers, facade);

        findMatchedPeerReaders(_this, facade);
    }

    return result;
}


in_connectivityParticipantFacade
in_connectivityAdminGetParticipant(
    in_connectivityAdmin _this,
    struct v_participantInfo *participant)
{
    Coll_Iter* iterator;
    in_connectivityParticipantFacade facade;

    /* go over all facade's to find it */
    iterator = Coll_Set_getFirstElement(&_this->Participants);
    while(iterator)
    {
        facade = in_connectivityParticipantFacade(Coll_Iter_getObject(iterator));
        /* This should be replaced by an identity match */
        if ( v_gidCompare(in_connectivityParticipantFacadeGetInfo(facade)->key,
                          participant->key) == C_EQ ) {
            return in_connectivityParticipantFacade(in_objectKeep(in_object(facade)));
        }
        iterator = Coll_Iter_getNext(iterator);
    }

    return NULL;
}

in_connectivityReaderFacade
in_connectivityAdminGetReader(
    in_connectivityAdmin _this,
    struct v_subscriptionInfo *reader)
{
    Coll_Iter* iterator;
    in_connectivityReaderFacade facade;

    /* go over all facade's to find it */
    iterator = Coll_Set_getFirstElement(&_this->Readers);
    while(iterator)
    {
        facade = in_connectivityReaderFacade(Coll_Iter_getObject(iterator));
        /* This should be replaced by an identity match */
        if ( v_gidCompare(in_connectivityReaderFacadeGetInfo(facade)->key,
                          reader->key) == C_EQ ) {
            return in_connectivityReaderFacade(in_objectKeep(in_object(facade)));
        }
        iterator = Coll_Iter_getNext(iterator);
    }

    return NULL;
}

in_connectivityWriterFacade
in_connectivityAdminGetWriter(
    in_connectivityAdmin _this,
    struct v_publicationInfo *writer)
{
    Coll_Iter* iterator;
    in_connectivityWriterFacade facade;

    /* go over all facade's to find it */
    iterator = Coll_Set_getFirstElement(&_this->Writers);
    while(iterator)
    {
        facade = in_connectivityWriterFacade(Coll_Iter_getObject(iterator));
        /* This should be replaced by an identity match */
        if ( v_gidCompare(in_connectivityWriterFacadeGetInfo(facade)->key,
                          writer->key) == C_EQ ) {
            return in_connectivityWriterFacade(in_objectKeep(in_object(facade)));
        }
        iterator = Coll_Iter_getNext(iterator);
    }

    return NULL;
}


in_result
in_connectivityAdminRemoveParticipant(
    in_connectivityAdmin _this,
    struct v_participantInfo *participant)
{
    Coll_Iter* iterator;
    in_connectivityParticipantFacade facade;
    in_result result = IN_RESULT_NOT_FOUND;

    /* go over all facade's to find it */
    iterator = Coll_Set_getFirstElement(&_this->Participants);
    while(iterator)
    {
        facade = in_connectivityParticipantFacade(Coll_Iter_getObject(iterator));
        /* This should be replaced by an identity match */
        if ( v_gidCompare(in_connectivityParticipantFacadeGetInfo(facade)->key,
                          participant->key) == C_EQ ) {
            /* remove this facade from the collection*/
            Coll_Set_remove(&_this->Participants, facade);

            /* free the facade */
            in_connectivityParticipantFacadeFree(facade);

            result = IN_RESULT_OK;
        }
        iterator = Coll_Iter_getNext(iterator);
    }

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

    /* go over all facade's to find it */
    iterator = Coll_Set_getFirstElement(&_this->Readers);
    while(iterator)
    {
        facade = in_connectivityReaderFacade(Coll_Iter_getObject(iterator));
        /* This should be replaced by an identity match */
        if ( v_gidCompare(in_connectivityReaderFacadeGetInfo(facade)->key,
                          reader->key) == C_EQ ) {
            /* remove this facade from the collection*/
            Coll_Set_remove(&_this->Readers, facade);

            /* free the facade */
            in_connectivityReaderFacadeFree(facade);

            result = IN_RESULT_OK;
        }
        iterator = Coll_Iter_getNext(iterator);
    }

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

    /* go over all facade's to find it */
    iterator = Coll_Set_getFirstElement(&_this->Writers);
    while(iterator)
    {
        facade = in_connectivityWriterFacade(Coll_Iter_getObject(iterator));
        /* This should be replaced by an identity match */
        if ( v_gidCompare(in_connectivityWriterFacadeGetInfo(facade)->key,
                          writer->key) == C_EQ ) {
            /* remove this facade from the collection*/
            Coll_Set_remove(&_this->Writers, facade);

            /* free the facade */
            in_connectivityWriterFacadeFree(facade);

            result = IN_RESULT_OK;
        }
        iterator = Coll_Iter_getNext(iterator);
    }

    return result;
}


in_result
in_connectivityAdminAddPeerParticipant(
    in_connectivityAdmin _this,
    in_connectivityPeerParticipant participant)
{
    Coll_Iter* iterator;
    in_connectivityPeerParticipant knownpeer;
    os_boolean found = FALSE;
    in_result result = IN_RESULT_OK;

    /* go over all known peers to see if it is already present */
    iterator = Coll_Set_getFirstElement(&_this->PeerParticipants);
    while(iterator)
    {
        knownpeer = in_connectivityPeerParticipant(Coll_Iter_getObject(iterator));
        if ( in_ddsiGuidEqual(in_connectivityPeerEntityGetGuid(in_connectivityPeerEntity(participant)),
                              in_connectivityPeerEntityGetGuid(in_connectivityPeerEntity(knownpeer)))) {
                              /* or should we update? */
            result = IN_RESULT_ALREADY_EXISTS;
            found = TRUE;
        }
        iterator = Coll_Iter_getNext(iterator);
    }

    if ( !found ) {
        /* create new facade and insert it */
        Coll_Set_add(&_this->PeerParticipants, participant);

        findMatchedParticipantFacades(_this, participant);
    }

    return result;
}


in_result
in_connectivityAdminAddPeerReader(
    in_connectivityAdmin _this,
    in_connectivityPeerReader reader)
{
    Coll_Iter* iterator;
    in_connectivityPeerReader knownpeer;
    os_boolean found = FALSE;
    in_result result = IN_RESULT_OK;

    /* go over all known peers to see if it is already present */
    iterator = Coll_Set_getFirstElement(&_this->PeerReaders);
    while(iterator)
    {
        knownpeer = in_connectivityPeerReader(Coll_Iter_getObject(iterator));
        if ( in_ddsiGuidEqual(in_connectivityPeerEntityGetGuid(in_connectivityPeerEntity(reader)),
                              in_connectivityPeerEntityGetGuid(in_connectivityPeerEntity(knownpeer)))) {
                              /* or should we update? */
            result = IN_RESULT_ALREADY_EXISTS;
            found = TRUE;
        }
        iterator = Coll_Iter_getNext(iterator);
    }

    if ( !found ) {
        /* create new facade and insert it */
        Coll_Set_add(&_this->PeerReaders, reader);

        findMatchedWriterFacades(_this, reader);
    }

    return result;
}


in_result
in_connectivityAdminAddPeerWriter(
    in_connectivityAdmin _this,
    in_connectivityPeerWriter writer)
{
    Coll_Iter* iterator;
    in_connectivityPeerWriter knownpeer;
    os_boolean found = FALSE;
    in_result result = IN_RESULT_OK;

    /* go over all known peers to see if it is already present */
    iterator = Coll_Set_getFirstElement(&_this->PeerWriters);
    while(iterator)
    {
        knownpeer = in_connectivityPeerWriter(Coll_Iter_getObject(iterator));
        if ( in_ddsiGuidEqual(in_connectivityPeerEntityGetGuid(in_connectivityPeerEntity(writer)),
                              in_connectivityPeerEntityGetGuid(in_connectivityPeerEntity(knownpeer)))) {
                              /* or should we update? */
            result = IN_RESULT_ALREADY_EXISTS;
            found = TRUE;
        }
        iterator = Coll_Iter_getNext(iterator);
    }

    if ( !found ) {
        /* create new facade and insert it */
        Coll_Set_add(&_this->PeerWriters, writer);

        findMatchedReaderFacades(_this, writer);
    }

    return result;
}

in_connectivityPeerParticipant
in_connectivityAdminGetPeerParticipant(
    in_connectivityAdmin _this,
    in_ddsiGuid guid)
{
    Coll_Iter* iterator;
    in_connectivityPeerParticipant knownpeer;

    /* go over all known peers to see if it is present */
    iterator = Coll_Set_getFirstElement(&_this->PeerParticipants);
    while(iterator)
    {
        knownpeer = in_connectivityPeerParticipant(Coll_Iter_getObject(iterator));
        if ( in_ddsiGuidEqual(guid,in_connectivityPeerEntityGetGuid(in_connectivityPeerEntity(knownpeer)))) {
            return in_connectivityPeerParticipant(in_objectKeep(in_object(knownpeer)));
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

    /* go over all known peers to see if it is present */
    iterator = Coll_Set_getFirstElement(&_this->PeerReaders);
    while(iterator)
    {
        knownpeer = in_connectivityPeerReader(Coll_Iter_getObject(iterator));
        if ( in_ddsiGuidEqual(guid,in_connectivityPeerEntityGetGuid(in_connectivityPeerEntity(knownpeer)))) {
            return in_connectivityPeerReader(in_objectKeep(in_object(knownpeer)));
        }
        iterator = Coll_Iter_getNext(iterator);
    }

    return NULL;
}

in_connectivityPeerWriter
in_connectivityAdminGetPeerWriter(
    in_connectivityAdmin _this,
    in_ddsiGuid guid)
{
    Coll_Iter* iterator;
    in_connectivityPeerWriter knownpeer;

    /* go over all known peers to see if it is present */
    iterator = Coll_Set_getFirstElement(&_this->PeerWriters);
    while(iterator)
    {
        knownpeer = in_connectivityPeerWriter(Coll_Iter_getObject(iterator));
        if ( in_ddsiGuidEqual(guid,in_connectivityPeerEntityGetGuid(in_connectivityPeerEntity(knownpeer)))) {
            return in_connectivityPeerWriter(in_objectKeep(in_object(knownpeer)));
        }
        iterator = Coll_Iter_getNext(iterator);
    }

    return NULL;
}


in_result
in_connectivityAdminRemovePeerParticipant(
    in_connectivityAdmin _this,
    in_ddsiGuid guid)
{
    Coll_Iter *iterator, *peeriterator;
    in_connectivityPeerParticipant knownpeer;
    in_connectivityParticipantFacade facade;
    in_result result = IN_RESULT_NOT_FOUND;

    /* go over all known peers to see if it is present */
    iterator = Coll_Set_getFirstElement(&_this->PeerParticipants);
    while(iterator)
    {
        knownpeer = in_connectivityPeerParticipant(Coll_Iter_getObject(iterator));
        if ( in_ddsiGuidEqual(guid,in_connectivityPeerEntityGetGuid(in_connectivityPeerEntity(knownpeer)))) {
            /* remove this peer from the collection*/
            Coll_Set_remove(&_this->PeerParticipants, knownpeer);

            /* go over all facade's to remove this peer */
            peeriterator = Coll_Set_getFirstElement(&_this->Participants);
            while(peeriterator)
            {
                facade = in_connectivityParticipantFacade(Coll_Iter_getObject(iterator));
                in_connectivityParticipantFacadeRemoveMatchedPeer(facade, knownpeer);
                peeriterator = Coll_Iter_getNext(peeriterator);
            }

            /* free the peer */
            in_connectivityPeerParticipantFree(knownpeer);

            result = IN_RESULT_OK;
        }
        iterator = Coll_Iter_getNext(iterator);
    }

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

    /* go over all known peers to see if it is present */
    iterator = Coll_Set_getFirstElement(&_this->PeerReaders);
    while(iterator)
    {
        knownpeer = in_connectivityPeerReader(Coll_Iter_getObject(iterator));
        if ( in_ddsiGuidEqual(guid,in_connectivityPeerEntityGetGuid(in_connectivityPeerEntity(knownpeer)))) {
            /* remove this peer from the collection*/
            Coll_Set_remove(&_this->PeerReaders, knownpeer);

            /* go over all facade's to remove this peer */
            peeriterator = Coll_Set_getFirstElement(&_this->Writers);
            while(peeriterator)
            {
                facade = in_connectivityWriterFacade(Coll_Iter_getObject(iterator));
                in_connectivityWriterFacadeRemoveMatchedPeer(facade, knownpeer);
                peeriterator = Coll_Iter_getNext(peeriterator);
            }

            /* free the peer */
            in_connectivityPeerReaderFree(knownpeer);

            result = IN_RESULT_OK;
        }
        iterator = Coll_Iter_getNext(iterator);
    }

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

    /* go over all known peers to see if it is present */
    iterator = Coll_Set_getFirstElement(&_this->PeerWriters);
    while(iterator)
    {
        knownpeer = in_connectivityPeerWriter(Coll_Iter_getObject(iterator));
        if ( in_ddsiGuidEqual(guid,in_connectivityPeerEntityGetGuid(in_connectivityPeerEntity(knownpeer)))) {
            /* remove this peer from the collection*/
            Coll_Set_remove(&_this->PeerWriters, knownpeer);

            /* go over all facade's to remove this peer */
            peeriterator = Coll_Set_getFirstElement(&_this->Readers);
            while(peeriterator)
            {
                facade = in_connectivityReaderFacade(Coll_Iter_getObject(iterator));
                in_connectivityReaderFacadeRemoveMatchedPeer(facade, knownpeer);
                peeriterator = Coll_Iter_getNext(peeriterator);
            }

            /* free the peer */
            in_connectivityPeerWriterFree(knownpeer);

            result = IN_RESULT_OK;
        }
        iterator = Coll_Iter_getNext(iterator);
    }

    return result;
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


