/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR PrismTech
 *   Limited, its affiliated companies and licensors. All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */
#include "v__kernel.h"
#include "v__groupInstance.h"
#include "v_groupCache.h"
#include "v_state.h"
#include "v_writer.h"
#include "v_writerCache.h"
#include "v__group.h"
#include "v__lifespanAdmin.h"
#include "v__transaction.h"
#include "c_collection.h"
#include "v_public.h"
#include "v__topic.h"
#include "v_instance.h"
#include "v_message.h"
#include "v_messageQos.h"
#include "v__policy.h"

#include "os_report.h"
#include "os_abstract.h"

#if 0
#define _ABORT_(condition) if (!(condition)) abort()
#else
#define _ABORT_(condition)
#endif

#define CHECK_REFCOUNT(var, i)

#ifndef NDEBUG
#define CHECK_COUNT(instance) v_groupInstanceCheckCount(instance)
#else
#define CHECK_COUNT(instance)
#endif

#ifndef NDEBUG
static void
v_groupInstanceCheckCount(
    v_groupInstance instance)
{
    int disposedFound = 0;
    int writeFound = 0;
    v_groupSample currentSample;
    v_registration reg, unreg;
    int inconsistent = 0;

    currentSample = v_groupInstanceTail(instance);
    while (currentSample != NULL) {
        if (v_messageStateTest(v_groupSampleMessage(currentSample),L_WRITE)) {
            writeFound++;
        }
        if (v_messageStateTest(v_groupSampleMessage(currentSample),L_DISPOSED)) {
            disposedFound++;
        }
        currentSample = currentSample->newer;
    }
    assert (writeFound == instance->historySampleCount);
    assert ((writeFound + disposedFound) == instance->count);


    /* Check the list again but now also in the opposite direction to verify
     * the instance consistentcy.
     */
    disposedFound = 0; writeFound = 0;
    currentSample = v_groupInstanceHead(instance);
    while (currentSample != NULL) {
        if (v_messageStateTest(v_groupSampleMessage(currentSample),L_WRITE)) {
            writeFound++;
        }
        if (v_messageStateTest(v_groupSampleMessage(currentSample),L_DISPOSED)) {
            disposedFound++;
        }
        currentSample = v_groupSample(currentSample->older);
    }
    assert (writeFound == instance->historySampleCount);
    assert ((writeFound + disposedFound) == instance->count);

    /* now check registrations and unregisterMessages lists.
     * first do cross check! messages in registrations may not
     * occur in unregisterMessages. */
    inconsistent = 0;
    reg = instance->registrations;
    while (reg != NULL) {
        unreg = instance->unregistrations;
        while ((unreg != NULL) && (!inconsistent)) {
            if (v_gidCompare(unreg->writerGID, reg->writerGID) == C_EQ) {
                inconsistent = 1;
            }
            assert(!inconsistent);
            unreg = unreg->next;
        }
        reg = reg->next;
    }
    /* check for duplicates in both lists.
     * Only check if next item is a duplicate as this is the most likely
     * place for a duplicate. */
    reg = instance->registrations;
    while (reg != NULL) {
        unreg = reg->next;
        if (unreg != NULL) {
            if (v_gidCompare(unreg->writerGID, reg->writerGID) == C_EQ) {
                inconsistent = 1;
            }
        }
        assert(!inconsistent);
        reg = reg->next;
    }
    unreg = instance->unregistrations;
    while (unreg != NULL) {
        reg = unreg->next;
        if (reg != NULL) {
            if (v_gidCompare(unreg->writerGID, reg->writerGID) == C_EQ) {
                inconsistent = 1;
            }
        }
        assert(!inconsistent);
        unreg = unreg->next;
    }
}

#endif

#if 0
static c_bool
v_groupInstanceValidateRegistrations(
    v_groupInstance instance)
{
    v_registration reg, unreg;
    c_bool result = TRUE;

    reg = instance->registrations;

    if(reg != NULL){
        if(!c_checkType(reg, "v_registration")){
            OS_REPORT(OS_ERROR, "v_groupInstance", 0,
                    "instance->registrations is corrupted.");
            assert(FALSE);
            result = FALSE;
        } else if(c_refCount(reg) != 1){
            OS_REPORT(OS_ERROR, "v_groupInstance", 0,
                    "instance->registrations refCount != 1.");
            assert(FALSE);
            result = FALSE;
        } else {
            while (reg != NULL) {
                unreg = reg->next;

                if (unreg != NULL) {
                    if (v_gidCompare(unreg->writerGID, reg->writerGID) == C_EQ) {
                        OS_REPORT(OS_ERROR, "v_groupInstance", 0,
                                "instance->registrations has duplicate registrations.");
                        result = FALSE;
                    }
                }
                reg = reg->next;
            }
        }
    }
    unreg = instance->unregistrations;
    if(unreg != NULL){
        if(!c_checkType(unreg, "v_registration")){
            OS_REPORT(OS_ERROR, "v_groupInstance", 0,
                    "instance->unregistrations is corrupted.");
            assert(FALSE);
            result = FALSE;
        } else if(c_refCount(unreg) != 1){
            OS_REPORT(OS_ERROR, "v_groupInstance", 0,
                    "instance->unregistrations refCount != 1.");
            assert(FALSE);
            result = FALSE;
        } else {
            while (unreg != NULL) {
                reg = unreg->next;
                if (reg != NULL) {
                    if (v_gidCompare(unreg->writerGID, reg->writerGID) == C_EQ) {
                        OS_REPORT(OS_ERROR, "v_groupInstance", 0,
                                "instance->unregistrations has duplicate registrations.");
                        result = FALSE;
                    }
                }
                unreg = unreg->next;
            }
        }
    }
    return result;
}

#define CHECK_REGISTRATIONS(instance) _ABORT_(v_groupInstanceValidateRegistrations(instance))
#else
#define CHECK_REGISTRATIONS(instance)
#endif

static v_groupInstance
v_groupAllocInstance(
    v_group _this)
{
    v_kernel kernel;
    v_groupInstance instance;

    assert(_this);
    assert(C_TYPECHECK(_this,v_group));

    if (_this->cachedInstance == NULL) {
        instance = v_groupInstance(c_new(_this->instanceType));
        if (instance) {
            _ABORT_(c_refCount(instance) == 1);
            kernel = v_objectKernel(_this);
            v_object(instance)->kernel = kernel;
            v_objectKind(instance) = K_GROUPINSTANCE;
            instance->targetCache = v_groupCacheNew(kernel, V_CACHE_TARGETS);
            instance->group = (c_voidp)_this;
            if (instance->targetCache == NULL) {
                OS_REPORT(OS_ERROR,
                          "v_groupAllocInstance",V_RESULT_INTERNAL_ERROR,
                          "Failed to allocate targetCache.");
                assert(FALSE);
                c_free(instance);
                instance = NULL;
            }
        } else {
            OS_REPORT(OS_FATAL,
                      "v_groupAllocInstance",V_RESULT_INTERNAL_ERROR,
                      "Failed to allocate group instance.");
            assert(FALSE);
        }
    } else {
        instance = _this->cachedInstance;
        _ABORT_(c_refCount(instance) == 1);
        _this->cachedInstance = NULL;
        assert(instance->group == (c_voidp)_this);
    }
    _ABORT_(c_refCount(instance->targetCache) == 1);

    return instance;
}

void
v_groupInstanceInit (
    v_groupInstance _this,
    v_message message)
{
    c_array instanceKeyList;
    c_array messageKeyList;
    c_ulong i, nrOfKeys;
    v_topicQos topicQos;

    topicQos = v_topicGetQos(v_groupTopic(_this->group));

    /*
     * copy the key value of the message into the newly created instance.
     */
    messageKeyList = v_topicMessageKeyList(v_groupTopic(_this->group));
    instanceKeyList = v_groupKeyList(_this->group);
    nrOfKeys = c_arraySize(messageKeyList);
    assert(nrOfKeys == c_arraySize(instanceKeyList));
    for (i=0;i<nrOfKeys;i++) {
        c_fieldCopy(messageKeyList[i],message,
                    instanceKeyList[i],_this);
    }
    c_free(instanceKeyList);

    _this->epoch                = OS_TIMEE_ZERO;
    _this->registrations        = NULL;
    _this->unregistrations      = NULL;
    _this->oldest               = NULL;
    _this->historySampleCount   = 0;
    _this->resourceSampleCount  = 0;
    _this->count                = 0;
    _this->state                = 0;
    _this->owner.exclusive      = (topicQos->ownership.v.kind == V_OWNERSHIP_EXCLUSIVE);

    v_stateSet(_this->state, L_EMPTY);
    assert(v_groupInstanceHead(_this) == NULL);
    v_groupInstanceSetHead(_this,NULL);

    c_free (topicQos);
}

v_groupInstance
v_groupInstanceNew(
    v_group group,
    v_message message)
{
    v_groupInstance _this;

    assert(C_TYPECHECK(group,v_group));
    assert(C_TYPECHECK(message,v_message));

    _this = v_groupAllocInstance(group);
    v_groupInstanceInit(_this, message);
    CHECK_REGISTRATIONS(_this);

    return _this;
}

void
v_groupInstanceFree(
    v_groupInstance instance)
{
    v_group group;
    v_groupSample sample;
    c_array instanceKeyList;
    c_ulong i, nrOfKeys;

    assert(C_TYPECHECK(instance,v_groupInstance));
    assert((instance->count == 0) == (v_groupInstanceHead(instance) == NULL));
    assert((instance->count == 0) == (v_groupInstanceTail(instance) == NULL));
    CHECK_COUNT(instance);
    CHECK_REGISTRATIONS(instance);

    if (c_refCount(instance) == 1) {
        c_free(instance->registrations);
        instance->registrations = NULL;
        c_free(instance->unregistrations);
        instance->unregistrations = NULL;

        /* make sure it is removed from any purge list! */
        instance->epoch = OS_TIMEE_ZERO;

        v_groupCacheDeinit(instance->targetCache);
        group = v_group(instance->group);
        if (group->cachedInstance == NULL) {
            /*
               Explicit free the sample when this v_groupInstance is
               cached. Because otherwise the reference is overwritten
               when the v_groupInstance is re-used.
               Note: the sample itself is also cached in its own module.
            */
            sample = v_groupInstanceHead(instance);
            c_free(sample);
            v_groupInstanceSetHead(instance,NULL);
            /*
               Also free the key values for this instance, otherwise the
               reference(s) is/are overwritten when the v_groupInstance
               is re-used.
            */
            instanceKeyList = v_groupKeyList(instance->group);
            nrOfKeys = c_arraySize(instanceKeyList);
            for (i = 0; i < nrOfKeys; i++) {
                c_fieldFreeRef(instanceKeyList[i], instance);
            }
            c_free(instanceKeyList);
            group->cachedInstance = c_keep(instance);
        }
    }
    c_free(instance);
}

void
v_groupInstanceDisconnect(
    v_groupInstance instance)
{
    OS_UNUSED_ARG(instance);
    assert(C_TYPECHECK(instance,v_groupInstance));
    assert((instance->count == 0) == (v_groupInstanceHead(instance) == NULL));
    assert((instance->count == 0) == (v_groupInstanceTail(instance) == NULL));
    CHECK_COUNT(instance);
    CHECK_REGISTRATIONS(instance);

    /* This function used to call v_writerCacheDeinit.  See dds230
     * for discussion why sourceCache was removed from v_groupInstance
     */
}

static c_equality
v_registrationMessageCompare (
    v_registration _this,
    v_message msg)
{
    C_STRUCT(v_message) template;

    /* If the v_registration is an implicit UNREGISTER, then any message may re-register. */
    if (_this->state & L_IMPLICIT) {
        assert(_this->state & L_UNREGISTER);
        return C_LT;
    }

    template.writeTime = _this->writeTime;
    template.writerGID = _this->writerGID;
    template.sequenceNumber = _this->sequenceNumber;
    ((v_node)&template)->nodeState = _this->state & L_IMPLICIT;
    return v_messageCompare(&template, msg);
}

v_writeResult
v_groupInstanceRegister (
    v_groupInstance instance,
    v_message message)
{
    v_registration *registration;
    v_registration found;
    v_writeResult result;
    v_topicQos topicQos;
    c_bool bySource;

    assert(instance != NULL);
    assert(C_TYPECHECK(instance,v_groupInstance));
    assert(message != NULL);
    assert(C_TYPECHECK(message,v_message));
    CHECK_COUNT(instance);
    CHECK_REGISTRATIONS(instance);
    /* First look if an unregister message exists for the message writer.
     * If an unregister message is found then verify if the register
     * message is newer than the unregister message. If the register
     * message is newer then remove the unregister message and insert the
     * register message and otherwise the register message is 'old news' and
     * can be discarded.
     * If there was no unregister message then there could still be a
     * register message so first look for an existing registration before
     * inserting it.
     * In case there already exists a registration for the given writer
     * then keep the newest.
     * Under normal condition register messages are always followed by an
     * unregister message but message can be reordered due to transport.
     */
    found = NULL;
    if (instance->unregistrations) {
        os_duration delay = OS_DURATION_INIT(5, 0);
        os_timeE purgeTime = os_timeESub(os_timeEGet(), delay);

        /*
         * Keep track of the address of the current registration to be able to
         * redirect the content of the previous item in the registration list
         * when removing the current registration.
         */
        registration = &instance->unregistrations;
        while (*registration != NULL && found == NULL) {
            if (v_gidCompare((*registration)->writerGID, message->writerGID) == C_EQ) {
                found = *registration;
                /* Pull registration from list by redirecting the contents of the
                 * previous registration pointer to the next registration. */
                *registration = found->next;
                found->next = NULL;
            } else {
                /* Temporary implementation. Final solution must be more efficient.
                 * e.g. by walking from oldest to newer and use of an active garbage collector.
                 * The purgeDelay should also be specified via a configuration parameter. */
                if (os_timeECompare((*registration)->unregisterTime, purgeTime) == OS_LESS) {
                    found = *registration;
                    /* Pull registration from list by redirecting the contents of the
                     * previous registration pointer to the next registration. */
                    *registration = found->next;
                    found->next = NULL;
                    c_free(found);
                    found = NULL;
                } else {
                    registration = &((*registration)->next);
                }
            }
        }
    }
    if (found == NULL) {
        /* No unregister message found so start looking for a register
         * message in the registration list.
         */
        registration = &instance->registrations;
        while (*registration != NULL && found == NULL) {
            if (v_gidCompare((*registration)->writerGID, message->writerGID) == C_EQ) {
                found = *registration;
            } else {
                registration = &((*registration)->next);
            }
        }
        if (found == NULL) {
            /* No existing registration found so insert a new one for
             * this writer.
             */
            found = c_new(v_kernelType(v_objectKernel(instance),
                                       K_REGISTRATION));
            if (found) {
                found->writerGID = message->writerGID;
                found->qos = c_keep(message->qos);
                found->writeTime = message->writeTime;
                found->sequenceNumber = message->sequenceNumber;
                found->state = v_messageState(message);
                /* Registrations are never IMPLICIT, not even when created by IMPLICIT samples. */
                v_stateClear(found->state, L_IMPLICIT);
                v_stateSet(found->state, L_REGISTER);
                found->transaction = NULL; /* Only unregistrations have a transaction ref */
                found->next = instance->registrations;
                instance->registrations = found;
                result = V_WRITE_REGISTERED;
            } else {
                result = V_WRITE_PRE_NOT_MET;
                OS_REPORT(OS_CRITICAL,
                  "v_groupInstanceRegister",result,
                  "Failed to allocate v_registration object.");
                assert(FALSE);
            }
        } else {
            /* if register message is newer than found and is explicitly
             * registered so replace the old one because the new one may
             * have different QoS values.
             * Note that probably in this case the unregister message is
             * belonging to the previous register message is not yet received.
             * It will be discarded when it arrives.
             * Note that the reader are not aware of this situation and
             * do miss a generation count increase!
             */
            if (v_messageStateTest(message, L_REGISTER)) {
                if (v_registrationMessageCompare(found, message) == C_LT)
                {
                    c_free(found->qos);
                    found->qos = c_keep(message->qos);
                    found->writeTime = message->writeTime;
                    found->sequenceNumber = message->sequenceNumber;
                    found->state = v_messageState(message);
                    c_free(found->transaction);
                    found->transaction = NULL;
                }
            }
            result = V_WRITE_SUCCESS;
        }
    } else if (!v_messageStateTest(message, L_UNREGISTER)){
        /* check writeTime of unregister message and given message in case
           the destination order policy is BY_SOURCE_TIMESTAMP.
           In that case If given message is older, return WRITE_SUCCESS,
           so no explicit register message is send.
           If the destination order policy is BY_RECEPTION_TIMESTAMP
           then a registration must be generated to build the instance
           pipeline.
        */
        if (message->qos != NULL) {
            bySource = v_messageQos_isBySource(message->qos);
        } else {
            /* Received NIL message, use topicQos */
            topicQos = v_topicGetQos(v_groupTopic(v_groupInstanceGroup(instance)));
            bySource = (topicQos->orderby.v.kind == V_ORDERBY_SOURCETIME) ? TRUE : FALSE;
            c_free (topicQos);
        }
        if ( (bySource == TRUE) &&
             v_registrationMessageCompare(found, message) == C_GT) {
            /* reinsert as unregister message */
            found->unregisterTime = message->allocTime;
            found->next = instance->unregistrations;
            c_free(found->transaction);
            found->transaction = NULL;
            instance->unregistrations = found;
            result = V_WRITE_UNREGISTERED;
        } else {
            c_free(found->qos);
            found->qos = c_keep(message->qos);
            found->writeTime = message->writeTime;
            found->sequenceNumber = message->sequenceNumber;
            found->state = v_messageState(message);
            /* Registrations are never IMPLICIT, not even when created by IMPLICIT samples. */
            v_stateClear(found->state, L_IMPLICIT);
            c_free(found->transaction);
            found->transaction = NULL;
            found->next = instance->registrations;
            instance->registrations = found;
            result = V_WRITE_REGISTERED;
        }
    } else {
        /* The writer already had the instance unregistered. */
        found->next = instance->unregistrations;
        instance->unregistrations = found;
        result = V_WRITE_SUCCESS;
    }
    if (instance->registrations != NULL) {
        v_stateClear(instance->state, L_NOWRITERS);
        instance->epoch = OS_TIMEE_ZERO;
    }
    CHECK_COUNT(instance);
    CHECK_REGISTRATIONS(instance);

    return result;
}


static void
v_groupInstanceRemoveUnregistrationIfObsolete(
    v_groupInstance instance,
    v_gid writerGID)
{
    v_registration *unregistration;
    v_registration unregistrationFound;
    v_groupSample sample;
    v_groupSample sampleFound;

    CHECK_REGISTRATIONS(instance);
    /*
     * First find the corresponding unregister message, if available.
     * Keep track of the address of the current unregistration to be able to
     * redirect the content of the previous item in the unregistration list
     * when removing the current unregistration.
     */
    unregistration = &instance->unregistrations;
    unregistrationFound = NULL;
    while (*unregistration != NULL && unregistrationFound == NULL)
    {
        if (v_gidCompare((*unregistration)->writerGID, writerGID) == C_EQ)
        {
            unregistrationFound = *unregistration;
        }
        else
        {
            unregistration = &((*unregistration)->next);
        }
    }
    /* Only continue in case the unregistration exists */
    if (unregistrationFound != NULL)
    {
        /* Now figure out if the instance contains any messages
         * from this gid */
        sample = v_groupSample(instance->oldest);
        sampleFound = NULL;
        while (sample != NULL && sampleFound == NULL)
        {
            if (v_gidCompare(v_groupSampleTemplate(sample)->message->writerGID,
                    writerGID) == C_EQ)
            {
                sampleFound = sample;
            }
            else
            {
                sample = sample->newer;
            }
        }
        if (sampleFound == NULL)
        {
            /* No sample found for this gid so remove the corresponding
             * unregistration message. Pull the registration from the list
             * by redirecting the contents of the previous registration pointer
             * to the next registration. */
            *unregistration = unregistrationFound->next;
            unregistrationFound->next = NULL;
            c_free(unregistrationFound);
        }
    }
    CHECK_REGISTRATIONS(instance);
}


v_writeResult
v_groupInstanceUnregister (
    v_groupInstance instance,
    v_message message,
    v_transaction transaction)
{
    v_writeResult result;
    v_registration *registration;
    v_registration found;
    c_equality equality;
    v_transaction txn;

    assert(instance != NULL);
    assert(C_TYPECHECK(instance,v_groupInstance));
    assert(message != NULL);
    assert(C_TYPECHECK(message,v_message));
    CHECK_COUNT(instance);
    CHECK_REGISTRATIONS(instance);

    if (v_messageStateTest(message,L_MARK)) {
        v_stateClear(instance->state, L_MARK);
    }

    /*
     * Keep track of the address of the current registration to be able to
     * redirect the content of the previous item in the registration list
     * when removing the current registration.
     */
    registration = &instance->registrations;
    found = NULL;
    while (*registration != NULL && found == NULL)
    {
        if (v_gidCompare((*registration)->writerGID, message->writerGID) == C_EQ)
        {
            found = *registration;
            /* Pull registration from list by redirecting the contents of the
             * previous registration pointer to the next registration. */
            *registration = found->next;
            found->next = NULL;
        }
        else
        {
            /* must be in else clause,
             * since then clause could have NULL-ified..
             */
            registration = &((*registration)->next);
        }
    }
    if (found != NULL)
    {
        /* If resolved registration is older than unregister message, or if topic is ordered BY_RECEPTION,
         * then process the unregister. Otherwise skip this unregister since it belongs to an older
         * generation than the current registration.
         */
        if (v_registrationMessageCompare(found, message) == C_LT ||
                v_topicQosRef(v_groupInstanceGroup(instance)->topic)->orderby.v.kind == V_ORDERBY_RECEPTIONTIME)

        {
            /* If message is currently owning instance, reset owner */
            equality = v_gidCompare (message->writerGID, instance->owner.gid);
            if (equality == C_EQ)
            {
                v_gidSetNil(instance->owner.gid);
                instance->owner.strength = 0;
            }

            c_free(found->qos);
            found->qos = c_keep(message->qos);
            found->writeTime = message->writeTime;
            found->unregisterTime = message->allocTime;
            found->sequenceNumber = message->sequenceNumber;
            found->state = v_messageState(message);
            /* Use the txn variable to ensure that when txn == transaction the
             * transaction is not accidentally freed. */
            txn = found->transaction;
            found->transaction = c_keep(transaction);
            c_free(txn);
            /* Insert the registration into the unregistration list. */
            found->next = instance->unregistrations;
            instance->unregistrations = found;
            result = V_WRITE_UNREGISTERED;
        }
        else
        {
            /* reinsert registration */
            result = V_WRITE_SUCCESS;
            found->next = instance->registrations;
            instance->registrations = found;
        }
    }
    else
    {
        /* Registration not found; This may happen during concurrent alignment
         * and the unregistration by the DataWriter in case that one is still
         * active at that time. In that case the unregister has already
         * arrived via the regular path before durability aligns it.
         */
        result = V_WRITE_SUCCESS;
    }
    if (instance->registrations == NULL)
    {
        /* no more writers, so set state to NOWRITERS */
        v_stateSet(instance->state, L_NOWRITERS);
    }

    if (result == V_WRITE_UNREGISTERED)
    {
        /* Unregister succeeded, remove unregister message in case there
         * are no messages in the instance at all.
         */
        v_groupInstanceRemoveUnregistrationIfObsolete(instance, message->writerGID);
    }

    CHECK_COUNT(instance);
    CHECK_REGISTRATIONS(instance);
    return result;
}

c_bool
v_groupInstanceWalkRegistrations (
    v_groupInstance instance,
    v_groupInstanceWalkRegistrationAction action,
    c_voidp arg)
{
    v_registration registration;
    c_bool proceed = TRUE;

    assert(instance != NULL);
    assert(C_TYPECHECK(instance,v_groupInstance));
    assert(action != NULL);
    CHECK_COUNT(instance);
    CHECK_REGISTRATIONS(instance);

    registration = instance->registrations;
    while ((registration != NULL) && (proceed == TRUE)) {
        proceed = action(registration, arg);
        registration = registration->next;
    }
    CHECK_COUNT(instance);
    CHECK_REGISTRATIONS(instance);

    return proceed;
}

c_bool
v_groupInstanceWalkUnregisterMessages (
    v_groupInstance instance,
    v_groupInstanceWalkRegistrationAction action,
    c_voidp arg)
{
    v_registration registration;
    c_bool proceed = TRUE;

    assert(instance != NULL);
    assert(C_TYPECHECK(instance,v_groupInstance));
    assert(action != NULL);
    CHECK_COUNT(instance);
    CHECK_REGISTRATIONS(instance);

    registration = instance->unregistrations;

    while ((registration != NULL) && (proceed == TRUE)) {
        proceed = action(registration,arg);
        registration = registration->next;
    }
    CHECK_COUNT(instance);
    CHECK_REGISTRATIONS(instance);

    return proceed;
}

v_message
v_groupInstanceCreateMessage(
    v_groupInstance _this)
{
    c_array instanceKeyList;
    c_array messageKeyList;
    c_ulong i, nrOfKeys;
    v_group group;
    v_message message = NULL;

    if (_this != NULL)
    {
        group = v_groupInstanceGroup(_this);
        message = v_topicMessageNew(v_groupTopic(group));
        if (message != NULL)
        {
            messageKeyList = v_topicMessageKeyList(v_groupTopic(group));
            instanceKeyList = v_groupKeyList(group);
            assert(c_arraySize(messageKeyList) == c_arraySize(instanceKeyList));
            nrOfKeys = c_arraySize(messageKeyList);
            for (i=0;i<nrOfKeys;i++)
            {
                c_fieldCopy(instanceKeyList[i],_this,
                            messageKeyList[i],message);
            }
            c_free(instanceKeyList);
        }
        else
        {
            OS_REPORT(OS_ERROR,
                      "v_groupInstance",0,
                      "v_groupInstanceCreateMessage(_this=0x%"PA_PRIxADDR")\n"
                      "        Failed to allocate a v_message.", (os_address)_this);
        }
    }
    return message;
}

v_message
v_groupInstanceCreateTypedInvalidMessage(
    v_groupInstance _this,
    v_message untypedMsg)
{
    v_message typedMsg;

    /* Create a message for the invalid sample to carry. */
    typedMsg = v_groupInstanceCreateMessage(_this);
    if (typedMsg)
    {
        /* Set correct attributes. */
        v_node(typedMsg)->nodeState = v_node(untypedMsg)->nodeState;
        typedMsg->writerGID = untypedMsg->writerGID;
        typedMsg->writeTime = untypedMsg->writeTime;
        typedMsg->writerInstanceGID = untypedMsg->writerInstanceGID;
        typedMsg->qos = c_keep(untypedMsg->qos);
        typedMsg->sequenceNumber = untypedMsg->sequenceNumber;
        typedMsg->transactionId = untypedMsg->transactionId;
    }
    else
    {
        OS_REPORT(OS_ERROR,
                  "v_groupInstance", V_RESULT_INTERNAL_ERROR,
                  "v_groupInstanceCreateTypedInvalidMessage(_this=0x%"PA_PRIxADDR", untypedMsg=0x%"PA_PRIxADDR")\n"
                  "        Operation failed to allocate new v_message: result = NULL.",
                  (os_address)_this, (os_address)untypedMsg);
        assert(FALSE);
    }

    return typedMsg;
}

c_bool
v_groupInstanceWalkSamples (
    v_groupInstance instance,
    v_groupInstanceWalkSampleAction action,
    c_voidp arg)
{
    v_groupSample sample;
    c_bool proceed = TRUE;

    assert(instance != NULL);
    assert(C_TYPECHECK(instance,v_groupInstance));
    assert(action != NULL);

    sample = v_groupSample(instance->oldest);
    while ((sample != NULL) && (proceed == TRUE)) {
        proceed = action(sample,arg);
        sample = sample->newer;
    }
    return proceed;
}

c_bool
v_groupInstanceClaimResource(
    v_groupInstance _this,
    v_message message)
{
    c_bool resourceAvailable = TRUE;
    v_group group = v_group(_this->group);
    v_topicQos qos = v_topicQosRef(group->topic);

    if (v_messageStateTest(message, L_WRITE) && v_groupIsDurable(group) && !v_groupIsOnRequest(group)) {
        if ((qos->durabilityService.v.max_samples != V_LENGTH_UNLIMITED) &&
                (group->resourceSampleCount >= qos->durabilityService.v.max_samples)) {
            resourceAvailable = FALSE;
        }
        if (resourceAvailable && qos->durabilityService.v.max_samples_per_instance != V_LENGTH_UNLIMITED &&
                v_groupInstanceResourceSampleCount(_this) >= qos->durabilityService.v.max_samples_per_instance) {
            resourceAvailable = FALSE;
        }
        if (resourceAvailable) {
            _this->resourceSampleCount++;
            v_groupSampleCountIncrement(group);
        }
    }


    return resourceAvailable;
}

void
v_groupInstanceReleaseResource(
    v_groupInstance _this)
{
    v_group group = v_group(_this->group);

    if (v_groupIsDurable(group) && !v_groupIsOnRequest(group)) {
        assert(_this->resourceSampleCount);

        _this->resourceSampleCount--;
        v_groupSampleCountDecrement(group);
    }
}

static v_writeResult
insertSample(
    v_groupInstance instance,
    v_message message,
    c_bool isTransactionFlush,
    v_transaction transaction)
{
    v_groupSample sample;
    v_groupSample oldest;
    v_groupSample ptr;
    v_group group;
    v_topicQos topicQos;
    v_state state;
    c_bool resourcesClaimed = TRUE;

    group = v_group(instance->group);
    topicQos = v_topicQosRef(group->topic);

    if (v_stateTest(instance->state, L_EMPTY)) {
        if (!isTransactionFlush) {
            resourcesClaimed = v_groupInstanceClaimResource(instance, message);
        }
        if (resourcesClaimed) {
            sample = v_groupSampleNew(group,message);

            if (!sample) {
                return V_WRITE_PRE_NOT_MET;
            }

            assert(v_groupInstanceHead(instance) == NULL);
            assert(v_groupInstanceTail(instance) == NULL);
            assert(instance->count == 0);
            v_groupInstanceSetHead(instance,sample);
            v_groupInstanceSetTail(instance,sample);
            v_stateClear(instance->state, L_EMPTY);
            ptr = NULL;
        } else {
            /* No chance to replace an existing sample, so return V_WRITE_REJECTED. */
            return V_WRITE_REJECTED;
        }
    } else {
        assert(v_groupInstanceHead(instance) != NULL);
        assert(v_groupInstanceTail(instance) != NULL);
        assert(instance->count != 0);

        /* Determine where the sample needs to be stored in the instance. */
        if (topicQos->orderby.v.kind == V_ORDERBY_SOURCETIME) {
            c_long depthBookmark = 0;
            c_equality equality;
            ptr = v_groupInstanceHead(instance);
            /* May have received an older message, so need to find the
               right spot in the instance.  Must insert after ptr, if
               ptr is NULL, then our message is the new oldest. */
            do {
                equality = v_messageCompare(message, v_groupSampleMessage(ptr));
                if (equality == C_EQ) {
                    return V_WRITE_DUPLICATE;
                }
                if (equality == C_LT) {
                    if (v_messageStateTest(v_groupSampleMessage(ptr), L_WRITE)) {
                        depthBookmark++;
                    }
                    ptr = ptr->older;
                }
            } while (ptr != NULL && equality == C_LT);
            /* If this sample is older then all VALDID samples in the current
             * history, and the history has already reached its full depth,
             * then the sample can discarded.
             */
            if (topicQos->durabilityService.v.history_kind == V_HISTORY_KEEPLAST &&
                    depthBookmark >= topicQos->durabilityService.v.history_depth) {
                return V_WRITE_SUCCESS_NOT_STORED;
            }
        } else {
            /* Check whether there is a potential duplicate of this sample in the history. */
            ptr = v_groupInstanceHead(instance);
            do {
                if (v_messageCheckDuplicate(message, v_groupSampleMessage(ptr))) {
                    return V_WRITE_DUPLICATE;
                }
                ptr = ptr->older;
            } while (ptr != NULL);
        }

        /* If the sample requires resources, the history policy is KEEP_LAST and the
         * maximum depth is already achieved, then try to push out samples to make room
         * for this one.
         */
        if (v_messageStateTest(message, L_WRITE) &&
                topicQos->durabilityService.v.history_kind == V_HISTORY_KEEPLAST) {
            while (instance->historySampleCount >= topicQos->durabilityService.v.history_depth) {
                /* We have reached max history depth for this instance.
                 * So the tail samples must be removed until we have removed a
                 * write sample.
                 * Note: after removing the oldest (write) message this algorthm
                 * continues to remove any Dispose messages until the next oldest
                 * (write) message is found. This avoids unecessary resource
                 * consumption by superfluous dispose messages.
                 */
                /* Algorithm: walk the history from oldest towards newest.
                 * first:     skip all data-less samples since they are not counted.
                 * second:    check if the oldest data sample is not the sample
                 *            just inserted. If that is true then the inserted
                 *            sample is accepted and can be inserted into the
                 *            lifespan admin.
                 * third:     skip all data-less samples between the oldest data
                 *            sample and the next newer data sample.
                 * last:      delete all (skipped) samples older than the found
                 *            second last oldest data sample.
                 */
                oldest = v_groupSample(instance->oldest);
                assert(oldest);
                if (v_messageStateTest(v_groupSampleMessage(oldest),L_DISPOSED)) {
                    instance->count--;
                }
                if (v_messageStateTest(v_groupSampleMessage(oldest),L_WRITE)) {
                    instance->count--;
                    instance->historySampleCount--;
                    instance->resourceSampleCount--;
                    v_groupSampleCountDecrement(group);
                }
                v_groupInstanceRemoveUnregistrationIfObsolete(instance,
                                                              v_groupSampleMessage(oldest)->writerGID);
                v_lifespanAdminRemove(group->lifespanAdmin,v_lifespanSample(oldest));
                sample = v_groupSample(oldest->newer);
                if (sample) {
                    sample->older = NULL;
                } else {
                    v_groupInstanceTemplate(instance)->newest = NULL;
                }
                assert(oldest->older == NULL);
                /* If the sample pushed out happens to be our ptr 'bookmark', then reset ptr too. */
                if (oldest == ptr) {
                    ptr = NULL;
                }
                c_free(oldest);
                instance->oldest = sample;
            }
        }

        if (!isTransactionFlush) {
            resourcesClaimed = v_groupInstanceClaimResource(instance, message);
        }
        if (resourcesClaimed) {
            sample = v_groupSampleNew(group,message);

            if (!sample) {
                return V_WRITE_PRE_NOT_MET;
            }
        } else {
            return V_WRITE_REJECTED;
        }

        /* The transient store history must obey the destination order
         * qos policy of the topic, so put in it the history as the
         * last one in case of ordering by reception timestamp and
         * put in the right location in the history otherwise.
         */
        if(topicQos->orderby.v.kind == V_ORDERBY_RECEPTIONTIME){
            sample->older = v_groupInstanceHead(instance);      /* Transfer existing refCount. */
            if (v_groupInstanceHead(instance)) {
                v_groupInstanceHead(instance)->newer = sample;  /* 'newer' attribute not refCounted */
            } else {
                v_groupInstanceSetTail (instance, sample);
            }
            v_groupInstanceSetHead(instance, sample);           /* v_groupInstanceSetHead performs keep */
        } else if (ptr == NULL) {
            sample->newer = v_groupInstanceTail(instance);      /* 'newer' attribute not refCounted */
            if (v_groupInstanceTail(instance)) {
                v_groupInstanceTail(instance)->older = c_keep(sample);
            } else {
                v_groupInstanceSetHead(instance, sample);       /* v_groupInstanceSetHead performs keep */
            }
            v_groupInstanceSetTail (instance, sample);
        } else {
            if (ptr != v_groupInstanceHead(instance)) {
                sample->older = ptr;                            /* Transfer existing refCount. */
                sample->newer = ptr->newer;                     /* 'newer' attribute not refCounted */
                v_groupSample(ptr->newer)->older = c_keep(sample);
                ptr->newer = sample;                            /* 'newer' attribute not refCounted */

            } else {
                sample->older = v_groupInstanceHead(instance);  /* Transfer existing refCount. */
                v_groupInstanceHead(instance)->newer = sample;  /* 'newer' attribute not refCounted */
                v_groupInstanceSetHead(instance, sample);       /* v_groupInstanceSetHead performs keep */
            }
        }
    }
    assert(c_refCount(sample) == 2);

    sample->instance = instance;
    state = v_nodeState(message);
    sample->transaction = c_keep(transaction);

    if (v_stateTest(state,L_DISPOSED)) {
        instance->count++;
    }
    if (v_stateTest(state, L_WRITE)) {
        instance->count++;
        instance->historySampleCount++;
        v_lifespanAdminInsert(group->lifespanAdmin, v_lifespanSample(sample));
    }
    if (v_messageStateTest(v_groupSampleMessage(v_groupInstanceHead(instance)), L_DISPOSED)) {
        v_stateSet(instance->state, L_DISPOSED);
    } else {
        if (v_stateTest(instance->state, L_DISPOSED)) {
            instance->epoch = OS_TIMEE_ZERO;
            v_stateClear(instance->state, L_DISPOSED);
        }
    }
    c_free(sample);

    CHECK_COUNT(instance);
    CHECK_REGISTRATIONS(instance);
    assert((instance->count == 0) == (v_groupInstanceTail(instance) == NULL));
    assert((instance->count == 0) == (v_groupInstanceHead(instance) == NULL));

    return V_WRITE_SUCCESS;
}

static v_groupActionKind
determineStreamAction(
        v_groupInstance instance,
        os_timeE now,
        v_groupActionKind intendedAction)
{
    os_duration delay;
    v_topicQos qos;
    v_groupActionKind resultingAction = intendedAction;

    /* if the instance state is NOWRITERS and DISPOSED then and only
     * then add the instance to the purge admin.
     */
    qos = v_topicQosRef(v_group(instance->group)->topic);
    if (v_groupInstanceStateTest(instance, L_DISPOSED | L_NOWRITERS)) {
        delay = qos->durabilityService.v.service_cleanup_delay;

        /* If service_cleanup_delay is zero, remove all samples and
         * insert instance in emptyList.
         */
        if (OS_DURATION_ISZERO(delay)) {
            if (!v_groupInstanceStateTest(instance,L_EMPTY)) {
                v_groupInstancePurge(instance);
            }
            assert(v_groupInstanceStateTest(instance,L_EMPTY));
           _empty_purgeList_insert(instance, now);
           resultingAction = V_GROUP_ACTION_CLEANUP_DELAY_EXPIRE;
        }
        /* Else if service_cleanup_delay is finite, insert instance
         * in disposed list.
         */
        else if (!OS_DURATION_ISINFINITE(delay)) {
            _dispose_purgeList_insert(instance, now);
            resultingAction = V_GROUP_ACTION_UNREGISTER;
        }
        /* If service_cleanup_delay is infinite, do nothing. */
        else {
            resultingAction = V_GROUP_ACTION_UNREGISTER;
        }
    }
    return resultingAction;
}

static void processStreamActions(
    v_groupInstance instance,
    v_message message,
    c_bool isTransactionFlush,
    c_bool stream)
{
    v_groupActionKind actionKind;
    v_group group = v_group(instance->group);
    os_timeE now = os_timeEGet();

    if (v_messageStateTest(message,L_WRITE)) {
        actionKind = V_GROUP_ACTION_WRITE;
    } else if (v_messageStateTest(message,L_DISPOSED)) {
        actionKind = V_GROUP_ACTION_DISPOSE;
    } else if (v_messageStateTest(message,L_UNREGISTER)) {
        actionKind = V_GROUP_ACTION_UNREGISTER;
    } else if (v_messageStateTest(message,L_REGISTER)) {
        actionKind = V_GROUP_ACTION_REGISTER;
    } else {
        actionKind = V_GROUP_ACTION_WRITE;
    }
    actionKind = determineStreamAction(instance, now, actionKind);
    /* Only forward to stream when sample is inserted in groupInstance and
     * when streaming is required */
    if(stream == TRUE && !isTransactionFlush) {
        forwardMessageToStreams(group, instance, message, now, actionKind);
    }
}

v_writeResult
v_groupInstanceInsert(
    v_groupInstance instance,
    v_message message,
    c_bool isTransactionFlush,
    v_transaction transaction,
    c_bool stream)
{
    v_group group;
    v_writeResult result;
    struct v_owner ownership;
    c_bool complete = FALSE;
    v_transactionAdmin transactionAdmin;

    assert(message != NULL);
    assert(instance != NULL);
    assert(C_TYPECHECK(message,v_message));
    assert(C_TYPECHECK(instance,v_groupInstance));
    assert((instance->count == 0) == (v_groupInstanceHead(instance) == NULL));
    assert((instance->count == 0) == (v_groupInstanceTail(instance) == NULL));
    CHECK_COUNT(instance);
    CHECK_REGISTRATIONS(instance);

    group = v_group(instance->group);

    if (v_messageStateTest(message,L_UNREGISTER) ||
        v_messageStateTest(message,L_REGISTER)) {
        if (!isTransactionFlush && v_message_isTransaction(message)) {
            transactionAdmin = v__groupGetTransactionAdmin(group);
            if (transactionAdmin) {
                (void)v_transactionAdminInsertMessage(transactionAdmin, message, v_instance(instance), FALSE, &complete);
                if (complete) {
                    if (!v_kernelGroupTransactionLockAccess(v_objectKernel(group))) {
                        v_kernelGroupTransactionFlush(v_objectKernel(group), transactionAdmin);
                        v_kernelGroupTransactionUnlockAccess(v_objectKernel(group));
                    }
                }
                c_free(transactionAdmin);
            }
        }
        /* If this is a mixed L_DISPOSED | L_UNREGISTER message, inserted because of a disconnect,
         * then not only remove the registration, but also process and store the DISPOSE sample.
         */
        if (v_messageStateTest(message,L_DISPOSED)) {
            result = insertSample(instance, message, FALSE, NULL);
        } else {
            result = V_WRITE_SUCCESS;
        }
        processStreamActions(instance, message, isTransactionFlush, stream);
        return result;
    }

    if (v_messageStateTest(message,L_MARK)) {
        v_stateClear(instance->state, L_MARK);
    }

    if (v_messageStateTest(message,L_MARK)) {
        v_stateClear(instance->state, L_MARK);
    }

    /* Exclusive ownership handling */
    /* Before NULL check on QoS was done here. Instead we now check the writer
       GID for validity and require QoS to be set because QoS is only allowed
       to be NULL in case of a "dispose all", in which case the writer GID must
       be NIL as well. */
    if (v_gidIsValid (message->writerGID)) {
        assert (message->qos != NULL);
        ownership.exclusive = v_messageQos_isExclusive(message->qos);
        ownership.strength = v_messageQos_getOwnershipStrength(message->qos);
    } else {
        assert (message->qos == NULL);
        ownership.exclusive = 0;
        ownership.strength = 0;
    }

    ownership.gid = message->writerGID;

    switch (v_determineOwnershipByStrength (
        &instance->owner, &ownership, TRUE))
    {
        case V_OWNERSHIP_INCOMPATIBLE_QOS:
        case V_OWNERSHIP_NOT_OWNER:
            if (!isTransactionFlush && v_message_isTransaction(message)) {
                transactionAdmin = v__groupGetTransactionAdmin(group);
                if (transactionAdmin) {
                    (void)v_transactionAdminInsertMessage(transactionAdmin, message, NULL, FALSE, &complete);
                    if (complete) {
                        if (!v_kernelGroupTransactionLockAccess(v_objectKernel(group))) {
                            v_kernelGroupTransactionFlush(v_objectKernel(group), transactionAdmin);
                            v_kernelGroupTransactionUnlockAccess(v_objectKernel(group));
                        }
                    }
                    c_free(transactionAdmin);
                }
            }
            return V_WRITE_SUCCESS_NOT_STORED;
            break;
        default:
            break;
    }

    if (!isTransactionFlush && v_message_isTransaction(message)) {
        /* If the sample belongs to an unfinished transaction, then insert it into the
         * transactional administration. Since this is a newly arriving sample, it still
         * needs to make a resource claim.
         */
        v_topicQos topicQos = v_topicGetQos(v_groupTopic(group));
        transactionAdmin = v__groupGetTransactionAdmin(group);
        if (transactionAdmin) {
            result = v_transactionAdminInsertMessage(transactionAdmin, message, v_instance(instance), FALSE, &complete);
            if (complete) {
                if (!v_kernelGroupTransactionLockAccess(v_objectKernel(group))) {
                    v_kernelGroupTransactionFlush(v_objectKernel(group), transactionAdmin);
                    v_kernelGroupTransactionUnlockAccess(v_objectKernel(group));
                }
            }
            c_free(transactionAdmin);
        } else {
            if (v_groupInstanceClaimResource(instance, message)) {
                result = V_WRITE_SUCCESS;
            } else {
                result = V_WRITE_REJECTED;
            };
        }
        if (result != V_WRITE_REJECTED) {
            processStreamActions(instance, message, isTransactionFlush, stream);
        }
        c_free(topicQos);

        return result;
    }

    /* Insert the sample into the history. Since this is a newly arriving
     * sample, it still needs to make a resource claim.
     */
    result = insertSample(instance, message, isTransactionFlush, transaction);

    if (result == V_WRITE_SUCCESS)
    {
        processStreamActions(instance, message, isTransactionFlush, stream);
    }

    return result;
}

void
v_groupInstanceRemove (
    v_groupSample sample)
{
    v_groupInstance instance;
    v_state state;

    assert(C_TYPECHECK(sample,v_groupSample));

    if (sample != NULL) {
        instance = v_groupInstance(sample->instance);
        CHECK_COUNT(instance);
        CHECK_REGISTRATIONS(instance);
        if (sample->newer != NULL) {
            v_groupSample(sample->newer)->older = c_keep(sample->older);
        } else {
            assert(v_groupInstanceHead(instance) == sample);
            v_groupInstanceSetHead(instance,sample->older);
        }
        if (sample->older != NULL) {
            v_groupSample(sample->older)->newer = sample->newer;
        } else {
            assert(v_groupInstanceTail(instance) == sample);
            v_groupInstanceSetTail(instance,sample->newer);
        }
        state = v_nodeState(v_groupSampleMessage(sample));
        if (v_stateTest(state, L_WRITE)) {
            instance->count--;
            instance->historySampleCount--;
            instance->resourceSampleCount--;
            v_groupSampleCountDecrement(instance->group);
        }
        if (v_stateTest(state, L_DISPOSED)) {
            instance->count--;
        }
        CHECK_COUNT(instance);
        c_free(sample);
        if (instance->oldest == NULL) {
            v_stateSet(instance->state, L_EMPTY);
        }
        CHECK_COUNT(instance);
        CHECK_REGISTRATIONS(instance);
        assert((instance->count == 0) == (v_groupInstanceTail(instance) == NULL));
        assert((instance->count == 0) == (v_groupInstanceHead(instance) == NULL));
    }
}

void
v_groupInstancePurge(
    v_groupInstance instance)
{
    v_group group;
    c_long disposeCount;

    assert(C_TYPECHECK(instance,v_groupInstance));
    assert(v_stateTest(instance->state, L_DISPOSED | L_NOWRITERS));
    CHECK_REGISTRATIONS(instance);

    disposeCount = instance->count - instance->historySampleCount;
    assert(disposeCount >= 0);
    group = v_group(instance->group);
    while ((instance->oldest != NULL) && (disposeCount > 0)) {
        v_lifespanAdminRemove(group->lifespanAdmin,
                              v_lifespanSample(instance->oldest));
        v_groupInstanceRemove(instance->oldest);
        disposeCount = instance->count - instance->historySampleCount;
    }
    assert(disposeCount == 0);
    assert(instance->historySampleCount <= instance->count);
    v_stateClear(instance->state, L_DISPOSED);
    CHECK_REGISTRATIONS(instance);
}


os_timeW
v_groupInstanceDisposeTime (
    v_groupInstance instance)
{
    assert(instance != NULL);
    assert(C_TYPECHECK(instance,v_groupInstance));

    return v_groupSampleMessage(v_groupInstanceHead(instance->oldest))->writeTime;
}

c_bool
v_groupInstanceHasRegistration(
    v_groupInstance instance,
    v_registration registration)
{
    v_registration reg;
    c_bool result = FALSE;

    assert(instance != NULL);
    assert(C_TYPECHECK(instance,v_groupInstance));
    CHECK_COUNT(instance);
    CHECK_REGISTRATIONS(instance);

    reg = instance->registrations;
    while ((reg != NULL) && (result == FALSE)) {
        /* TODO: When singleton registrations have been implemented, then
         * the gidCompare can be replaced by an ordinary pointer comparison.
         */
        if (v_gidCompare(reg->writerGID, registration->writerGID) == C_EQ) {
            result = TRUE;
        }
        reg = reg->next;
    }

    return result;
}

v_registration
v_groupInstanceGetRegistration(
    v_groupInstance instance,
    v_gid gidTemplate,
    v_matchIdentityAction predicate)
{
    v_registration reg;
    c_bool found = FALSE;

    assert(instance != NULL);
    assert(C_TYPECHECK(instance,v_groupInstance));
    CHECK_COUNT(instance);
    CHECK_REGISTRATIONS(instance);

    reg = instance->registrations;
    while (reg != NULL && !found)
    {
        if (predicate(reg->writerGID, gidTemplate) == C_EQ)
        {
            found = TRUE;
        }
        else
        {
            reg = reg->next;
        }
    }

    return c_keep(reg);
}

void
v_groupInstancePurgeTimed(
    v_groupInstance instance,
    os_timeE purgeTime)
{
    v_group group;
    v_message message;
    c_bool proceed;

    assert(C_TYPECHECK(instance,v_groupInstance));
    CHECK_REGISTRATIONS(instance);

    group = v_group(instance->group);
    if (instance->oldest != NULL) {
        proceed = TRUE;

        while ((proceed == TRUE) && (instance->oldest)) {
            message = v_groupSampleMessage(instance->oldest);
            /* Purge all messages with a source timestamp less than or equal
             * to purgeTime. The equality is important for the REPLACE merge
             * policy to guarantee that the dispose message that was generated
             * on behalf of the REPLACE merge policy is also purged!
             */
            if ( (os_timeECompare(message->allocTime, purgeTime) == OS_LESS) ||
                 (os_timeECompare(message->allocTime, purgeTime) == OS_EQUAL) ) {
                v_lifespanAdminRemove(group->lifespanAdmin, v_lifespanSample(instance->oldest));
                v_groupInstanceRemove(instance->oldest);
            } else {
                proceed = FALSE;
            }
        }
    }
    CHECK_REGISTRATIONS(instance);
    return;
}

static v_message
groupInstanceCreateMessageEOT(
    v_groupInstance _this)
{
    v_message message;

    message = c_new(v_kernelType(v_objectKernel(_this), K_MESSAGEEOT));
    if (message) {
        message->allocTime = os_timeEGet();
        message->qos = NULL;
        V_MESSAGE_INIT(message);
    } else {
        OS_REPORT(OS_FATAL,
                  "groupInstanceCreateMessageEOT",V_RESULT_INTERNAL_ERROR,
                  "Failed to allocate message.");
        assert(FALSE);
    }
    return message;
}

/*
 * TODO: The code snippet below can be removed when v_groupUnregisterByGidTemplate
 * starts using the part in the #if 0 clause instead of in its #else clause as
 * indicated by scdds2805.
 */
#if 1
void
v_groupInstancecleanup(
    v_groupInstance _this,
    v_registration registration,
    os_timeW timestamp,
    c_bool isImplicit)
{
    v_message unregMsg, eotMsg;
    v_group group;
    v_resendScope resendScope = V_RESEND_NONE;
    struct v_presentationPolicy presentation;
    c_bool transaction = FALSE;
    c_array tidList = NULL;

    assert(registration->qos != NULL);

    if (registration->qos) {
        presentation.access_scope = v_messageQos_presentationKind(registration->qos);
        presentation.coherent_access = v_messageQos_isCoherentAccess(registration->qos);
        presentation.ordered_access = v_messageQos_isOrderedAccess(registration->qos);

        if ((presentation.access_scope != V_PRESENTATION_INSTANCE) &&
            (presentation.coherent_access == TRUE)) {
            transaction = TRUE;

            if (presentation.access_scope == V_PRESENTATION_GROUP) {
                struct v_tid *tid;
                tidList = c_arrayNew(v_kernelType(v_objectKernel(_this), K_TID), 1);
                tid = (struct v_tid *)tidList;
                tid->wgid = registration->writerGID;
                tid->seqnr = 0;
            }
        }
    }

    group = v_groupInstanceGroup(_this);

    unregMsg = v_groupInstanceCreateMessage(_this);
    if (unregMsg) {
        v_stateSet(v_nodeState(unregMsg), L_UNREGISTER);
        if (isImplicit) {
            v_stateSet(v_nodeState(unregMsg), L_IMPLICIT);
        }
        if ((registration->qos != NULL) && (v_messageQos_isAutoDispose(registration->qos))) {
            v_stateSet(v_nodeState(unregMsg), L_DISPOSED);
        }
        if (transaction) {
            v_stateSet(v_nodeState(unregMsg), L_TRANSACTION);
            unregMsg->sequenceNumber = C_MAX_ULONG;
        }
        unregMsg->qos = c_keep(registration->qos); /* since messageQos does not contain refs */
        unregMsg->writerGID = registration->writerGID; /* pretend this message comes from the original writer! */
        unregMsg->writeTime = timestamp;
        v_groupWrite(group, unregMsg, &_this, V_NETWORKID_ANY, &resendScope);
        c_free(unregMsg);
    }

    if (transaction) {
        eotMsg = groupInstanceCreateMessageEOT(_this);
        if (eotMsg) {
            if (isImplicit) {
                v_nodeState(eotMsg) = L_IMPLICIT;
            }
            v_stateSet(v_nodeState(eotMsg), L_TRANSACTION | L_ENDOFTRANSACTION);
            eotMsg->sequenceNumber = C_MAX_ULONG;
            eotMsg->qos = c_keep(registration->qos);
            eotMsg->writerGID = registration->writerGID; /* pretend this message comes from the original writer! */
            eotMsg->writeTime = timestamp;
            v_messageEOT(eotMsg)->publisherId = 0;
            v_messageEOT(eotMsg)->transactionId = 0;
            v_messageEOT(eotMsg)->tidList = c_keep(tidList);
            v_groupWrite(group, eotMsg, &_this, V_NETWORKID_ANY, &resendScope);
            c_free(tidList);
            c_free(eotMsg);
        }
    }
}
#endif

/**
 * Unregister the instance if it has a registration prior to the specified time
 */
void
v_groupInstanceUnregisterByTime (
    v_groupInstance instance,
    os_timeW time)
{
    v_message message;
    v_resendScope resendScope = V_RESEND_NONE;
    v_registration registration;
    v_group group;

    assert(instance);

    group = v_groupInstanceGroup(instance);
    /* Inspect all registrations. If there is a registration
     * for a writer that has registered prior to time
     * then the writer will be unregistered to ensure that
     * that the instance will be destroyed. */
    registration = instance->registrations;
    while (registration != NULL) {
        if (os_timeWCompare(registration->writeTime, time) == OS_LESS) {
            message = v_groupInstanceCreateMessage(instance);
            if (message) {
                v_stateSet(v_nodeState(message), L_UNREGISTER);
                message->writerGID = registration->writerGID;
                message->writeTime = time;
                message->qos = c_keep(registration->qos);
                v_groupWrite(group, message, NULL, V_NETWORKID_ANY, &resendScope);
                c_free(message);
            }
        }
        registration = registration->next;
    }
}

