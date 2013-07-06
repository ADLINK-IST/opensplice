/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2013 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE
 *
 *   for full copyright notice and license terms.
 *
 */

#include "c_base.h"
#include "v__entity.h"
#include "v_handle.h"
#include "v__participant.h"
#include "v__publisher.h"
#include "v__subscriber.h"
#include "v__writer.h"
#include "v_dataReader.h"
#include "v_dataReaderEntry.h"
#include "v__dataView.h"
#include "v__reader.h"
#include "v_partition.h"
#include "v__topic.h"
#include "v__topicQos.h"
#include "v_query.h"
#include "v_status.h"
#include "v_serviceManager.h"
#include "v_service.h"
#include "v_serviceState.h"
#include "v_qos.h"
#include "v__spliced.h"
#include "v_waitset.h"
#include "v_observer.h"
#include "v_public.h"
#include "v__collection.h"

#include "os_report.h"

v_result
v_entityInit(
    v_entity e,
    const c_char *name,
    v_statistics s,
    c_bool enable)
{
    assert(C_TYPECHECK(e,v_entity));

    if (name == NULL) {
        e->name = NULL;
    } else {
        e->name = c_stringNew(c_getBase(e),name);
    }
    e->status = v_statusNew(e);
    e->statistics = s;
    e->enabled = enable;
    v_publicInit(v_public(e));
    return V_RESULT_OK;
}

void
v_entityFree(
    v_entity e)
{
    assert(C_TYPECHECK(e,v_entity));

    v_publicFree(v_public(e));
}

void
v_entityDeinit(
    v_entity e)
{
    assert(C_TYPECHECK(e,v_entity));

    if (e == NULL) {
        return;
    }
    v_publicDeinit(v_public(e));
}

v_status
v_entityStatus(
    v_entity e)
{
    assert(e != NULL);
    assert(C_TYPECHECK(e,v_entity));

    return c_keep(e->status);
}

v_result
v_entityEnable (
    v_entity e)
{
    v_result result;

    assert(e != NULL);
    assert(C_TYPECHECK(e,v_entity));

    if (e->enabled) {
        result = V_RESULT_PRECONDITION_NOT_MET;
    } else {
        e->enabled = TRUE;
        switch (v_objectKind(e)) {
        case K_TOPIC:
            /* TODO: OSPL-12.
             * Topics are currently always created in an enabled state, so it makes no sense
             * to try and enable them here. What is needed is that, when disabled topics are
             * supported, we check whether the v_topicEnable fails, and if it does we return
             * this fail to the user layer, which then needs to reroute its handle to the
             * topic's precursor.
             */
            result = V_RESULT_OK;
        break;
        case K_WRITER:
            result = v_writerEnable(v_writer(e));
        break;
        case K_DATAREADER:
            result = v_dataReaderEnable(v_dataReader(e));
        break;
        case K_PUBLISHER:
            result = v_publisherEnable(v_publisher(e));
        break;
        case K_SUBSCRIBER:
            result = v_subscriberEnable(v_subscriber(e));
        break;
        case K_PARTICIPANT:
        case K_SERVICE:
        case K_SPLICED:
        case K_NETWORKING:
        case K_DURABILITY:
        case K_CMSOAP:
        case K_RNR:
        case K_NETWORKREADER:
        case K_GROUPQUEUE:
        case K_DATAVIEW:
            result = V_RESULT_OK;
        break;
        default:
            OS_REPORT_1(OS_ERROR,
                        "v_entityEnable", 0,
                        "Supplied entity (%d) can not be enabled",
                        v_objectKind(e));
            result = V_RESULT_CLASS_MISMATCH;
        break;
        }
    }
    return result;
}

c_bool
v_entityEnabled (
    v_entity _this)
{
    assert(_this != NULL);
    assert(C_TYPECHECK(_this,v_entity));

    return _this->enabled;
}

c_voidp
v_entityGetUserData (
    v_entity e)
{
    assert(e != NULL);
    assert(C_TYPECHECK(e,v_entity));

    return v_public(e)->userDataPublic;
}

v_statistics
v_entityStatistics(
    v_entity e)
{
    assert(e != NULL);
    assert(C_TYPECHECK(e,v_entity));

    return c_keep(e->statistics);
}
c_voidp
v_entitySetUserData (
    v_entity e,
    c_voidp userDataPublic)
{
    c_voidp old;

    assert(e != NULL);
    assert(C_TYPECHECK(e,v_entity));

    old = v_public(e)->userDataPublic;
    v_public(e)->userDataPublic = userDataPublic;
    return old;
}

typedef struct entryActionArg {
    c_action action;
    c_voidp arg;
} *entryActionArg_t;

static c_bool
entryAction (
    c_object o,
    c_voidp arg)
{
    v_dataReaderEntry entry = v_dataReaderEntry(o);
    entryActionArg_t a = (entryActionArg_t)arg;

    return a->action(v_entity(entry->topic),a->arg);
}

c_bool
v_entityWalkEntities(
    v_entity e,
    c_bool (*action)(v_entity e, c_voidp arg),
    c_voidp arg)
{
    c_bool r = TRUE;

    switch(v_objectKind(e)){
    case K_DATAREADER:
        v_observerLock(v_observer(e));
        r = c_setWalk(v_collection(e)->queries,(c_action)action,arg);
        if (r == TRUE) {
            struct entryActionArg a;
            a.action = (c_action)action;
            a.arg = arg;
            v_readerWalkEntries(v_reader(e),entryAction,&a);
        }
        v_observerUnlock(v_observer(e));
    break;
    case K_QUERY:
    case K_DATAREADERQUERY:
        v_observerLock(v_observer(e));
        r = c_setWalk(v_collection(e)->queries,(c_action)action,arg);
        v_observerUnlock(v_observer(e));
    break;
    case K_PARTICIPANT:
    case K_SERVICE:
    case K_SPLICED:
    case K_DURABILITY:
    case K_NETWORKING:
    case K_CMSOAP:
    case K_RNR:
        c_lockRead(&v_participant(e)->lock);
        r = c_setWalk(v_participant(e)->entities,(c_action)action,arg);
        c_lockUnlock(&v_participant(e)->lock);
    break;
    case K_PUBLISHER:
        c_lockRead(&v_publisher(e)->lock);
        r = c_setWalk(v_publisher(e)->writers,(c_action)action,arg);
        if (r == TRUE) {
            r = c_tableWalk(v_publisher(e)->partitions->partitions,(c_action)action,arg);
        }
        c_lockUnlock(&v_publisher(e)->lock);
    break;
    case K_SUBSCRIBER:
        c_lockRead(&v_subscriber(e)->lock);
        r = c_setWalk(v_subscriber(e)->readers,(c_action)action,arg);
        if (r == TRUE) {
            r = c_tableWalk(v_subscriber(e)->partitions->partitions,(c_action)action,arg);
        }
        c_lockUnlock(&v_subscriber(e)->lock);
    break;
    case K_WRITER:
        v_observerLock(v_observer(e));
        r = action(v_entity(v_writer(e)->topic),arg);
        v_observerUnlock(v_observer(e));
    break;
    case K_KERNEL:
        c_lockRead(&v_kernel(e)->lock);
        r = c_tableWalk(v_kernel(e)->topics,(c_action)action,arg);
        if (r == TRUE) {
            r = c_tableWalk(v_kernel(e)->partitions,(c_action)action,arg);
        }
        if (r == TRUE) {
            r = c_setWalk(v_kernel(e)->participants,(c_action)action,arg);
        }
        c_lockUnlock(&v_kernel(e)->lock);
    break;
    default:
    break;
    }
    return r;
}

static c_bool
getTopic (
    c_object o,
    c_voidp arg)
{
    c_iter *iter = (c_iter *)arg;
    c_bool result = TRUE;

    if ((iter != NULL) &&
        (v_objectKind(o) == K_DATAREADERENTRY))
    {
        *iter = c_iterInsert(*iter,c_keep(v_dataReaderEntryTopic(o)));
    } else {
        result = FALSE;
    }
    return result;
}

#define DDS_268
#define DDS_269

c_bool
v_entityWalkDependantEntities(
    v_entity e,
    c_bool (*action)(v_entity e, c_voidp arg),
    c_voidp arg)
{
    c_bool r;
    v_entity entity;
    c_iter iter;
#ifdef DDS_268
    c_iter iter2, iter3;
    v_entity entity2;
    v_topic topic;
#else
#ifdef DDS_269
    v_entity entity2;
#endif
#endif

    r = TRUE;

    switch(v_objectKind(e)){
    case K_DATAREADER:
        if (v_reader(e)->subscriber != NULL) {
            r = action(v_entity(v_reader(e)->subscriber), arg);
        } else {
            r = FALSE;
        }
    break;
    case K_PUBLISHER:
        if (v_publisher(e)->participant != NULL) {
            r = action(v_publisher(e)->participant, arg);
        } else {
            r = FALSE;
        }
    break;
    case K_QUERY:
    case K_DATAREADERQUERY:
        if (v_query(e)->source != NULL) {
            r = action(v_query(e)->source, arg);
        } else {
            r = FALSE;
        }
    break;
    case K_SUBSCRIBER:
        if (v_subscriber(e)->participant != NULL) {
            r = action(v_subscriber(e)->participant, arg);
        } else {
            r = FALSE;
        }
    break;
    case K_WRITER:
        if (v_writer(e)->publisher != NULL) {
            r = action(v_writer(e)->publisher, arg);
        } else {
            r = FALSE;
        }
    break;
    case K_TOPIC:
        /**@todo lookup v_join*/

        /*lookup readers.*/
        iter = v_topicLookupReaders(v_topic(e));
        entity = v_entity(c_iterTakeFirst(iter));

        while (entity != NULL) {
            if (r == TRUE) {
                r = action(entity, arg);
#ifdef DDS_269
                entity2 = v_reader(entity)->subscriber;
                c_lockRead(&v_subscriber(entity2)->lock);
                r = c_tableWalk(v_subscriber(entity2)->partitions->partitions,(c_action)action,arg);
                c_lockUnlock(&v_subscriber(entity2)->lock);
#endif
            }
            c_free(entity);
            entity = v_entity(c_iterTakeFirst(iter));
        }
        c_iterFree(iter);

         /*lookup writers.*/
        if (r == TRUE) {
            iter = v_topicLookupWriters(v_topic(e));
            entity = v_entity(c_iterTakeFirst(iter));

            while(entity != NULL){
               if (r == TRUE) {
                   r = action(entity, arg);
#ifdef DDS_269
                entity2 = v_writer(entity)->publisher;
                c_lockRead(&v_publisher(entity2)->lock);
                r = c_tableWalk(v_publisher(entity2)->partitions->partitions,(c_action)action,arg);
                c_lockUnlock(&v_publisher(entity2)->lock);
#endif
               }
               c_free(entity);
               entity = v_entity(c_iterTakeFirst(iter));
            }
            c_iterFree(iter);
        }
    break;
    case K_DOMAIN:
        iter = v_partitionLookupPublishers(v_partition(e));
        entity = v_entity(c_iterTakeFirst(iter));
#ifdef DDS_268
        iter3 = c_iterNew(NULL);
#endif
        while (entity != NULL) {
            if (r == TRUE) {
                r = action(entity, arg);
#ifdef DDS_268
                if(r == TRUE){
                    c_lockRead(&v_publisher(entity)->lock);
                    iter2 = ospl_c_select(v_publisher(entity)->writers, 0);
                    c_lockUnlock(&v_publisher(entity)->lock);

                    entity2 = v_entity(c_iterTakeFirst(iter2));

                    while (entity2 != NULL) {
                        topic = c_keep(v_topic(v_writer(entity2)->topic));

                        if (c_iterContains(iter3, topic) == FALSE) {
                            iter3 = c_iterInsert(iter3, topic);
                        } else {
                            c_free(topic);
                        }
                        c_free(entity2);
                        entity2 = v_entity(c_iterTakeFirst(iter2));
                    }
                    c_iterFree(iter2);
                }
#endif
            }
            c_free(entity);
            entity = v_entity(c_iterTakeFirst(iter));
        }
        c_iterFree(iter);

        if (r == TRUE) {
            iter = v_partitionLookupSubscribers(v_partition(e));
            entity = v_entity(c_iterTakeFirst(iter));

            while (entity != NULL) {
                if(r == TRUE){
                    r = action(entity, arg);
#ifdef DDS_268
                    if(r == TRUE){
                        c_lockRead(&v_subscriber(entity)->lock);
                        iter2 = ospl_c_select(v_subscriber(entity)->readers, 0);
                        c_lockUnlock(&v_subscriber(entity)->lock);

                        entity2 = v_entity(c_iterTakeFirst(iter2));

                        while (entity2 != NULL) {
                            if (c_iterContains(iter3, entity2) == FALSE) {
                                v_readerWalkEntries(v_reader(entity2),
                                                    getTopic,
                                                    &iter3);
                            }
                            c_free(entity2);
                            entity2 = v_entity(c_iterTakeFirst(iter2));
                        }
                        c_iterFree(iter2);
                    }
#endif
                }
                c_free(entity);
                entity = v_entity(c_iterTakeFirst(iter));
            }
            c_iterFree(iter);
        }

#ifdef DDS_268
        entity = v_entity(c_iterTakeFirst(iter3));

        while (entity != NULL) {
           if (r == TRUE) {
               r = action(entity, arg);
           }
           c_free(entity);
           entity = v_entity(c_iterTakeFirst(iter3));
        }
        c_iterFree(iter3);
#endif
    break;
    default:
    break;
    }
    return r;
}

#undef DDS_268
#undef DDS_269

v_qos
v_entityGetQos(
    v_entity e)
{
    v_qos qos;

    assert(e != NULL);
    assert(C_TYPECHECK(e,v_entity));

    qos = NULL;

    switch(v_objectKind(e)){
    case K_PARTICIPANT:
    case K_SERVICE:
    case K_SPLICED:
    case K_NETWORKING:
    case K_DURABILITY:
    case K_CMSOAP:
    case K_RNR:
        qos = v_qos(c_keep(v_participant(e)->qos));
    break;
    case K_TOPIC:
        qos = v_qos(c_keep(v_topic(e)->qos));
    break;
    case K_WRITER:
        qos = v_qos(c_keep(v_writer(e)->qos));
    break;
    case K_DATAREADER:
    case K_GROUPQUEUE:
    case K_NETWORKREADER:
        qos = v_qos(c_keep(v_reader(e)->qos));
    break;
    case K_PUBLISHER:
        qos = v_qos(c_keep(v_publisher(e)->qos));
    break;
    case K_SUBSCRIBER:
        qos = v_qos(c_keep(v_subscriber(e)->qos));
    break;
    case K_DATAVIEW:
        qos = v_qos(c_keep(v_dataView(e)->qos));
    break;
    default:
        OS_REPORT_1(OS_ERROR,
                    "v_entityGetQos", 0,
                    "Supplied entity (%d) has no QoS",
                    v_objectKind(e));
    break;
    }

    return qos;
}

v_result
v_entitySetQos(
    v_entity e,
    v_qos qos)
{
    v_result result;

    assert(e != NULL);
    assert(C_TYPECHECK(e,v_entity));

    /* Do not use the cast method (e.g. v_writerQos(qos)),
     * since the qos is allocated on heap! */

    switch (v_objectKind(e)) {
    case K_PARTICIPANT: /* intentionally no break */
    case K_SERVICE:     /* intentionally no break */
    case K_SPLICED:
    case K_NETWORKING:
    case K_DURABILITY:
    case K_CMSOAP:
    case K_RNR:
        result = v_participantSetQos(v_participant(e), (v_participantQos)qos);
    break;
    case K_TOPIC:
        result = v_topicSetQos(v_topic(e), (v_topicQos)qos);
    break;
    case K_WRITER:
        result = v_writerSetQos(v_writer(e), (v_writerQos)qos);
    break;
    case K_DATAREADER:       /* intentionally no break */
    case K_NETWORKREADER:    /* intentionally no break */
    case K_GROUPQUEUE:
        result = v_readerSetQos(v_reader(e), (v_readerQos)qos);
    break;
    case K_PUBLISHER:
        result = v_publisherSetQos(v_publisher(e), (v_publisherQos)qos);
    break;
    case K_SUBSCRIBER:
        result = v_subscriberSetQos(v_subscriber(e), (v_subscriberQos)qos);
    break;
    case K_DATAVIEW:
        result = v_dataViewSetQos(v_dataView(e), (v_dataViewQos)qos);
    break;
    default:
        OS_REPORT_1(OS_ERROR,
                    "v_entityGetQos", 0,
                    "Supplied entity (%d) has no QoS",
                    v_objectKind(e));
        result = V_RESULT_CLASS_MISMATCH;
    break;
    }

    return result;
}

