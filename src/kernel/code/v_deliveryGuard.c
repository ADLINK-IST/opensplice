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
#include "v_public.h"
#include "v_message.h"

#include "v__kernel.h"
#include "v__deliveryGuard.h"
#include "v__deliveryWaitList.h"
#include "v__partition.h"

#include "v_writer.h"
#include "v_entity.h"
#include "v_topic.h"

#include "c_sync.h"

#include "os_report.h"

/*
 * This method updates the v_deliveryGuard->publications list with all synchronous
 * writers the v_deliveryService is aware of, preconditions for to be added to the list are:
 *  a) guard->writer->topic matches the sInfo->topic
 *  b) guard->writer->partitions match the sInfo->partitions
 *
 * parameters:
 *  o - a v_subscriptionInfoTemplate instance from the v_deliveryService->subscriptions list
 *  arg - the v_deliveryGuard who's publications list is to be updated
 *
 */
static c_bool
getMatchingPublications (
    c_object o,
    c_voidp arg)
{
    v_subscriptionInfoTemplate sInfo = (v_subscriptionInfoTemplate)o;
    v_deliveryPublisher p, found;
    C_STRUCT(v_deliveryPublisher) template;
    c_type type;
    c_long nrOfPartitions, i;
    v_deliveryGuard guard;
    v_writer writer;
    v_writerGroup wGroup;
    struct v_writerGroupSet groupSet;
    c_bool result = TRUE;

    guard = (v_deliveryGuard)arg;

    writer = v_writer(v_gidClaim (guard->writerGID, v_objectKernel(guard->owner)));

    if(writer){
        groupSet = writer->groupSet;

        wGroup = groupSet.firstGroup;

        while(wGroup){
            nrOfPartitions = c_arraySize(sInfo->userData.partition.name);
            if(strcmp(v_topicName(wGroup->group->topic), sInfo->userData.topic_name) == 0) {
                for(i=0; i<nrOfPartitions; i++){
                    if(v_partitionStringMatchesExpression(v_entity(wGroup->group->partition)->name, sInfo->userData.partition.name[i])){
                        template.readerGID = sInfo->userData.key;
                        found = v_deliveryPublisher(c_find(guard->publications,&template));
                        if (found) {
                            found->count++;
                        } else {
                            type = c_subType(guard->publications);
                            p = c_new(type);
                            if (p) {
                                p->count = 1;
                                p->readerGID = sInfo->userData.key;
                                found = c_insert(guard->publications,p);
                            } else {
                                OS_REPORT(OS_ERROR,
                                          "getMatchingPublications",0,
                                          "Failed to allocate v_deliveryPublisher object.");
                                assert(FALSE);
                                result = FALSE;
                            }
                        }
                    }
                }
            }
            wGroup = wGroup->next;
        }
        v_gidRelease(guard->writerGID, v_objectKernel(guard->owner));
    }
    else
    {
        OS_REPORT(OS_ERROR,
                  "getMatchingPublications",0,
                  "Could not claim writer; writer = NULL.");
        assert(writer);
        result = FALSE;

    }

    return result;
}



v_deliveryGuard
v_deliveryGuardNew(
    v_deliveryService _this,
    v_writer writer)
{
    C_STRUCT(v_deliveryGuard) template;
    v_deliveryGuard guard, found;
    c_type type;

    assert(C_TYPECHECK(_this,v_deliveryService));
    assert(C_TYPECHECK(writer,v_writer));

    if (_this && writer) {
        /* lookup or create a writer specific admin.
         */
        template.writerGID = v_publicGid(v_public(writer));
        found = v_deliveryGuard(c_find(_this->guards,&template));
        if (!found) {
            type = c_subType(_this->guards);
            guard = c_new(type);
            c_free(type);
            if (guard) {
                guard->writerGID = template.writerGID;
                guard->owner = (c_voidp)_this; /* backref used by the free */
                guard->gidType = c_resolve(c_getBase(_this),
                                 "kernelModule::v_gid");
                type = c_resolve(c_getBase(_this),
                                 "kernelModule::v_deliveryWaitList");
                /*
                 * The sequence number is the key, as this is unique
                 * within each writer.
                 */
                guard->waitlists = c_tableNew(type, "sequenceNumber");
                c_free(type);

                /* The following subscription set attribute contains the
                 * subscriptions of nodes that have synchronous DataReaders
                 * that communicate with this particular DataWriter.
                 * This list will be maintained by the reception of builtin
                 * topics. The list does not need to be initialized with
                 * the correct data right now because the topic is transient
                 * so the whole state will be delivered and delivery will
                 * update the list.
                 */
                type = c_resolve(c_getBase(_this),
                                 "kernelModule::v_deliveryPublisher");
                guard->publications = c_tableNew(type,
                                                 "readerGID.systemId,"
                                                 "readerGID.localId,"
                                                 "readerGID.serial");
                c_free(type);

                found = c_tableInsert(_this->guards, guard);
                if (found != guard) {
                    /* This should not happen! */
                    OS_REPORT(OS_ERROR,
                              "v_deliveryGuardNew",0,
                              "detected inconsistent writer admin.");
                }
                c_free(guard);
                c_keep(found);

                /* Now update publication list from delivery service subscriptions.
                 */
                c_walk(_this->subscriptions, getMatchingPublications, guard);
            } else {
                OS_REPORT(OS_ERROR,
                          "v_deliveryGuardNew",0,
                          "Failed to allocate v_deliveryGuard object.");
                assert(FALSE);
                found = NULL;
            }
        }
    } else {
        found = NULL;
    }
    return found;
}

v_result
v_deliveryGuardFree(
    v_deliveryGuard _this)
{
    v_deliveryGuard found;
    v_result result;

    assert(C_TYPECHECK(_this,v_deliveryGuard));

    if (_this) {
        found = c_remove(v_deliveryService(_this->owner)->guards,_this,NULL,NULL);
        if (found != _this) {
            /* This should not happen! */
            OS_REPORT(OS_ERROR,
                      "v_deliveryGuardFree",0,
                      "detected inconsistent writer admin.");
        }
        c_free(found);
        result = V_RESULT_OK;
    } else {
        OS_REPORT(OS_ERROR,
                  "v_deliveryGuardFree",0,
                  "Precondition not met; _this = NULL.");
        result = V_RESULT_PRECONDITION_NOT_MET;
    }
    return result;
}

static c_bool
waitlistNotify(
    c_object o,
    c_voidp arg)
{
    v_deliveryInfoTemplate msg = (v_deliveryInfoTemplate)arg;
    v_result result;

    result = v_deliveryWaitListNotify(v_deliveryWaitList(o), msg);
    return TRUE;
}

/* This method is called to pass a received delivery message to
 * a local synchronous DataWriter admin.
 * This method will subsequently notify all the blocking threads
 * about this delivery message.
 */
v_result
v_deliveryGuardNotify(
    v_deliveryGuard _this,
    v_deliveryInfoTemplate msg)
{
    v_result result;

    assert(C_TYPECHECK(_this,v_deliveryGuard));

    if (_this && msg) {
        /* lookup or create a writer specific admin.
         */
        c_walk(_this->waitlists,waitlistNotify,msg);
    } else {
        result = V_RESULT_ILL_PARAM;
    }
    return result;
}

static c_bool
waitlistIgnore(
    c_object o,
    c_voidp arg)
{
    v_gid readerGID = *(v_gid *)arg;
    v_result result;

    result = v_deliveryWaitListIgnore(v_deliveryWaitList(o),
                                      readerGID);
    return TRUE;
}

v_result
v_deliveryGuardIgnore(
    v_deliveryGuard _this,
    v_gid readerGID)
{
    v_result result;

    assert(C_TYPECHECK(_this,v_deliveryGuard));

    if (_this) {
        /* lookup or create a writer specific admin.
         */
        c_walk(_this->waitlists,waitlistIgnore,&readerGID);
    } else {
        result = V_RESULT_ILL_PARAM;
    }
    return result;
}
