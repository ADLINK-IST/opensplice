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
#include "os_abstract.h"
#include "os_heap.h"
#include "os_report.h"

#include "c_iterator.h"

#include "v__subscriber.h"
#include "v__subscriberQos.h"
#include "v_participant.h"
#include "v__domain.h"
#include "v_topic.h"
#include "v__domainAdmin.h"
#include "v__reader.h"
#include "v_group.h"
#include "v__observer.h"
#include "v_observable.h"
#include "v_public.h"
#include "v_groupSet.h"
#include "v_dataReaderEntry.h"
#include "v_status.h"
#include "v_event.h"
#include "v__groupStream.h"
#include "v__groupQueue.h"
#include "v__networkReader.h"
#include "v__dataReader.h"
#include "v__policy.h"
#include "v__kernel.h"


/* ----------------------------------- Private ------------------------------ */
static c_bool
qosChangedAction(
    c_object o,
    c_voidp arg)
{
    v_dataReader r;

    if (v_objectKind(o) == K_DATAREADER) {
        r = v_dataReader(o);
        v_dataReaderNotifyChangedQos(r, (v_dataReaderNotifyChangedQosArg *)arg);
    }

    return TRUE;
}

/* ----------------------------------- Public ------------------------------- */

v_subscriber
v_subscriberNew(
    v_participant p,
    const c_char *name,
    v_subscriberQos qos,
    c_bool enable)
{
    v_kernel kernel;
    v_subscriber s;
    v_subscriberQos q;
    v_entity found;

    kernel = v_objectKernel(p);
    q = v_subscriberQosNew(kernel,qos);
    if (q != NULL) {
        s = v_subscriber(v_objectNew(kernel,K_SUBSCRIBER));
        v_observerInit(v_observer(s),name, NULL, enable);
        s->qos = q;
        c_mutexInit(&s->sharesMutex, SHARED_MUTEX);
        if (q->share.enable) {
            v_lockShares(kernel);
            found = v_addShareUnsafe(kernel,v_entity(s));
            if (found != v_entity(s)) {
            	/* Make sure to set the domain list to NULL, because
				 * v_publicFree will cause a crash in the v_subscriberDeinit
				 * otherwise.
				 */
            	s->domains = NULL;
            	/*v_publicFree to free reference held by the handle server.*/
            	v_publicFree(v_public(s));
            	/*Now free the local reference as well.*/
                c_free(s);
                pa_increment(&(v_subscriber(found)->shareCount));
                v_unlockShares(kernel);
                return c_keep(found);
            }
            s->shares = c_tableNew(v_kernelType(kernel,K_READER),
                                   "qos.share.name");
        } else {
            s->shares = NULL;
        }
        s->shareCount  = 1;
        s->domains     = v_domainAdminNew(kernel);
        s->readers     = c_setNew(v_kernelType(kernel,K_READER));
        s->participant = p;

        c_lockInit(&s->lock,SHARED_LOCK);
        v_participantAdd(p,v_entity(s));

        if (q->share.enable) {
            v_unlockShares(kernel);
        }
        if (enable) {
            v_subscriberEnable(s);
        }
    } else {
        OS_REPORT(OS_ERROR,
                  "v_subscriberNew", 0,
                  "Subscriber not created: inconsistent qos");
        s = NULL;
    }
    return s;
}

v_result
v_subscriberEnable (
    v_subscriber _this)
{
    v_kernel kernel;
    c_iter list;
    c_char *partitionName;
    v_result result = V_RESULT_ILL_PARAM;

    if (_this) {
        kernel = v_objectKernel(_this);

        v_observableAddObserver(v_observable(kernel->groupSet),
                                v_observer(_this), NULL);

        if (_this->qos->partition != NULL) {
            list = v_partitionPolicySplit(_this->qos->partition);
            while((partitionName = c_iterTakeFirst(list)) != NULL) {
                v_subscriberSubscribe(_this,partitionName);
                os_free(partitionName);
            }
            c_iterFree(list);
        }
        result = V_RESULT_OK;
    }
    return result;
}


void
v_subscriberFree(
    v_subscriber s)
{
    v_kernel kernel;
    v_participant p;
    v_reader o;
    v_entity found;
    c_long sc;

    kernel = v_objectKernel(s);

    sc = (c_long)pa_decrement(&(s->shareCount));
    if (sc > 0) return;
    assert(sc == 0);

    if (s->qos->share.enable) {
        found = v_removeShare(kernel,v_entity(s));
        assert(found == v_entity(s));
        c_free(found);
    }
    while ((o = c_take(s->readers)) != NULL) {
        switch (v_objectKind(o)) {
        case K_DATAREADER:
            v_dataReaderFree(v_dataReader(o));
        break;
        case K_GROUPQUEUE:
            v_groupQueueFree(v_groupQueue(o));
        break;
        case K_NETWORKREADER:
            v_networkReaderFree(v_networkReader(o));
        break;
        default:
            OS_REPORT_1(OS_ERROR,
                        "v_subscriber", 0,
                        "Unknown reader %d",
                        v_objectKind(o));
            assert(FALSE);
        break;
        }
        c_free(o);
    }
    p = v_participant(s->participant);
    if (p != NULL) {
        v_participantRemove(p,v_entity(s));
        s->participant = NULL;
    }
    v_observableRemoveObserver(v_observable(kernel->groupSet),v_observer(s));

    v_publicFree(v_public(s));
}

void
v_subscriberDeinit(
    v_subscriber s)
{
	if(s->domains){
		v_domainAdminFree(s->domains);
		s->domains = NULL;
	}
    v_observerDeinit(v_observer(s));
}

static c_bool
collectDomains(
    c_object o,
    c_voidp arg)
{
    v_domain d = (v_domain)o;
    c_iter iter = (c_iter)arg;

    iter = c_iterInsert(iter,c_keep(d));
    return TRUE;
}

void
v_subscriberAddReader(
    v_subscriber s,
    v_reader r)
{
    v_reader found;
    v_domain d;
    c_iter iter;

    assert(s != NULL);
    assert(r != NULL);

    iter = c_iterNew(NULL);
    v_domainAdminWalkDomains(s->domains,collectDomains,iter);
    while ((d = c_iterTakeFirst(iter)) != NULL) {
        v_readerSubscribe(r,d);
        c_free(d);
    }
    c_iterFree(iter);
    c_lockWrite(&s->lock);
    found = c_setInsert(s->readers,r);
    c_lockUnlock(&s->lock);
    if (found != r) {
        OS_REPORT_1(OS_ERROR,
                    "v_subscriberAddReader", 0,
                    "shared <%s> name already defined",
                    r->qos->share.name);
    }
}

void
v_subscriberRemoveReader(
    v_subscriber s,
    v_reader r)
{
    v_reader found;
    v_domain d;
    c_iter iter;

    assert(s != NULL);
    assert(r != NULL);

    c_lockWrite(&s->lock);
    found = c_remove(s->readers,r,NULL,NULL);
    c_lockUnlock(&s->lock);
    iter = c_iterNew(NULL);
    v_domainAdminWalkDomains(s->domains,collectDomains,iter);
    while ((d = c_iterTakeFirst(iter)) != NULL) {
        v_readerUnSubscribe(r,d);
        c_free(d);
    }
    c_iterFree(iter);
    c_free(found);
}


void
v_subscriberLockShares(
    v_subscriber _this)
{
    c_mutexLock(&_this->sharesMutex);
}

void
v_subscriberUnlockShares(
    v_subscriber _this)
{
    c_mutexUnlock(&_this->sharesMutex);
}

v_reader
v_subscriberAddShareUnsafe(
    v_subscriber _this,
    v_reader reader)
{
    v_reader found;

    found = c_tableInsert(_this->shares,reader);

    return found;
}

v_reader
v_subscriberRemoveShare(
    v_subscriber _this,
    v_reader reader)
{
    v_reader found;

    v_subscriberLockShares(_this);
    found = c_remove(_this->shares,reader,NULL,NULL);
    v_subscriberUnlockShares(_this);

    return found;
}


c_bool
v_subscriberCheckDomainInterest(
    v_subscriber s,
    v_domain partition)
{
    return v_domainAdminFitsInterest(s->domains, partition);
}

void
v_subscriberSubscribe(
    v_subscriber s,
    const c_char *partitionExpr)
{
    v_domain d;
    v_dataReaderNotifyChangedQosArg arg;
    v_partitionPolicy old;

    assert(s != NULL);

    arg.removedDomains = NULL;

    c_lockWrite(&s->lock);
    arg.addedDomains = v_domainAdminAdd(s->domains, partitionExpr);
    old = s->qos->partition;
    s->qos->partition = v_partitionPolicyAdd(old, partitionExpr,
                                             c_getBase(c_object(s)));
    c_free(old);

    c_setWalk(s->readers, qosChangedAction, &arg);
    d = v_domain(c_iterTakeFirst(arg.addedDomains));
    while (d != NULL) {
        c_free(d);
        d = v_domain(c_iterTakeFirst(arg.addedDomains));
    }
    c_iterFree(arg.addedDomains);
    c_lockUnlock(&s->lock);
}


void
v_subscriberUnSubscribe(
    v_subscriber s,
    const c_char *partitionExpr)
{
    v_domain d;
    v_dataReaderNotifyChangedQosArg arg;
    v_partitionPolicy old;

    assert(s != NULL);
    assert(C_TYPECHECK(s,v_subscriber));

    arg.addedDomains = NULL;

    c_lockWrite(&s->lock);

    arg.removedDomains = v_domainAdminRemove(s->domains, partitionExpr);
    old = s->qos->partition;
    s->qos->partition = v_partitionPolicyRemove(old, partitionExpr,
                                                c_getBase(c_object(s)));
    c_free(old);
    c_setWalk(s->readers, qosChangedAction, &arg);

    d = v_domain(c_iterTakeFirst(arg.removedDomains));
    while (d != NULL) {
        c_free(d);
        d = v_domain(c_iterTakeFirst(arg.removedDomains));
    }
    c_iterFree(arg.removedDomains);

    c_lockUnlock(&s->lock);
}

c_iter
v_subscriberLookupReaders(
    v_subscriber s)
{
    c_iter list;

    assert(s != NULL);

    c_lockRead(&s->lock);
    list = c_select(s->readers,0);
    c_lockUnlock(&s->lock);
    return list;
}

struct lookupReaderByTopicArg {
    c_iter list;
    const c_char *topicName;
};

static c_bool
checkTopic (
    c_object o,
    c_voidp arg)
{
    v_dataReaderEntry entry;
    c_char *topicName = (c_char *)arg;

    if(v_object(o)->kind == K_DATAREADERENTRY){
        entry = v_dataReaderEntry(o);
        if (strcmp(v_topicName(entry->topic),topicName) == 0) {
            return FALSE;
        }
    }
    return TRUE;
}

static c_bool
lookupReaderByTopic(
    v_reader reader,
    c_voidp arg)
{
    struct lookupReaderByTopicArg *a = (struct lookupReaderByTopicArg *)arg;

    if (v_readerWalkEntries(reader,checkTopic, (c_voidp)a->topicName) == FALSE) {
        /* FALSE means that the topic name is found! */
        a->list = c_iterInsert(a->list, c_keep(reader));
    }
    return TRUE;
}

c_iter
v_subscriberLookupReadersByTopic(
    v_subscriber s,
    const c_char *topicName)
{
    struct lookupReaderByTopicArg arg;

    assert(s != NULL);

    arg.list = NULL;
    arg.topicName = topicName;

    c_lockRead(&s->lock);
    c_setWalk(s->readers, (c_action)lookupReaderByTopic, &arg);
    c_lockUnlock(&s->lock);

    return arg.list;
}

c_iter
v_subscriberLookupDomains(
    v_subscriber s,
    const c_char *partitionExpr)
{
    c_iter list;

    assert(s != NULL);

    list = v_domainAdminLookupDomains(s->domains, partitionExpr);

    return list;
}

v_subscriberQos
v_subscriberGetQos(
    v_subscriber s)
{
    v_subscriberQos qos;

    assert(s != NULL);

    c_lockRead(&s->lock);
    qos = v_subscriberQosNew(v_objectKernel(s), s->qos);
    c_lockUnlock(&s->lock);

    return qos;
}

v_result
v_subscriberSetQos(
    v_subscriber s,
    v_subscriberQos qos)
{
    v_result result;
    v_qosChangeMask cm;
    v_dataReaderNotifyChangedQosArg arg;
    v_domain d;

    assert(s != NULL);

    arg.addedDomains = NULL;
    arg.removedDomains = NULL;

    c_lockWrite(&s->lock);
    result = v_subscriberQosSet(s->qos, qos, v_entity(s)->enabled,&cm);
    if ((result == V_RESULT_OK) && (cm != 0)) {
        c_lockUnlock(&s->lock); /* since creation builtin topic might lock subscriber again. */
        if (cm & V_POLICY_BIT_PARTITION) { /* partition policy has changed! */
            v_domainAdminSet(s->domains, s->qos->partition,
                             &arg.addedDomains, &arg.removedDomains);
        }

        c_setWalk(s->readers, qosChangedAction, &arg);

        d = v_domain(c_iterTakeFirst(arg.addedDomains));
        while (d != NULL) {
            c_free(d);
            d = v_domain(c_iterTakeFirst(arg.addedDomains));
        }
        c_iterFree(arg.addedDomains);
        d = v_domain(c_iterTakeFirst(arg.removedDomains));
        while (d != NULL) {
            c_free(d);
            d = v_domain(c_iterTakeFirst(arg.removedDomains));
        }
        c_iterFree(arg.removedDomains);
    } else {
        c_lockUnlock(&s->lock);
    }

    return result;
}

static c_bool
notifyGroupQueues(
    v_reader reader,
    v_event event)
{
    if(v_objectKind(reader) == K_GROUPQUEUE){
        v_groupStreamNotify(v_groupStream(reader), event, NULL);
    }
    return TRUE;
}

void
v_subscriberNotify(
    v_subscriber s,
    v_event e)
{
    c_bool connect;
    v_group g;
    c_iter addedDomains;
    v_domain d;

    if (e->kind == V_EVENT_NEW_GROUP) {
        g = v_group(e->userData);
        connect = v_domainAdminFitsInterest(s->domains, g->partition);

        if (connect) {
            addedDomains = v_domainAdminAdd(s->domains,
                                            v_partitionName(g->partition));
            d = v_domain(c_iterTakeFirst(addedDomains));
            while (d != NULL) {
                c_free(d);
                d = v_domain(c_iterTakeFirst(addedDomains));
            }
            c_iterFree(addedDomains);
            c_setWalk(s->readers, (c_action)v_readerSubscribeGroup, g);
        } else {
            /**
             * NK: In case connect == FALSE, the groupQueue still might need
             * to connect to this group. Therefore all groupqueue readers
             * of this subscriber need to be notified. The groupQueues can
             * determine by themselves if they need to take any action.
             */
            c_setWalk(s->readers, (c_action)notifyGroupQueues, e);
        }
    }
}
