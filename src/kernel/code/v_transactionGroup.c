/*
 *                         Vortex OpenSplice
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
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

#include "v__transactionGroup.h"
#include "v__transaction.h"
#include "v__kernel.h"
#include "v_public.h"
#include "v_message.h"
#include "v_instance.h"
#include "v__subscriber.h"
#include "v__reader.h"
#include "v__dataReader.h"
#include "v__builtin.h"
#include "v__policy.h"
#include "v_time.h"
#include "os_abstract.h"
#include "os_report.h"
#include "os_heap.h"
#include "os_atomics.h"
#include "c_collection.h"


#define V_TRANSACTION_BLOCKSIZE (50)

#define TRACE_COHERENT 0
#if TRACE_COHERENT
#define TRACE_COHERENT_UPDATE(owner, fmt, ...) \
do { \
    printf("%s:%d(%s): ", OS_FUNCTION, __LINE__, owner?v_objectKindImage(owner):"(unknown)"); \
    printf(fmt, __VA_ARGS__); \
    fflush(stdout); \
} while (0)
#else
#define TRACE_COHERENT_UPDATE(...)
#endif

#define v_transactionPublisher(o) (C_CAST(o,v_transactionPublisher))
#define v_transactionGroupAdmin(o) (C_CAST(o,v_transactionGroupAdmin))
#define v_transactionGroupWriter(o) (C_CAST(o,v_transactionGroupWriter))
#define v_transactionGroupReader(o) (C_CAST(o,v_transactionGroupReader))

#define ADMIN_OWNER(o) \
        (v_transactionGroupAdmin(o)->owner)

#define PUBLISHER_OWNER(o) \
        ((o) ? \
         (v_transactionGroupAdmin(v_transactionPublisher(o)->admin)->owner) : \
         NULL)

#define TRANSACTION_OWNER(o) \
        ((o) ? \
         (v_transactionGroupAdmin(v_transactionGroup(o)->admin)->owner) : \
         NULL)

#define v_transactionPublisherCount(p) \
        (p ? (v_transactionPublisher(p)->writers ? c_count(v_transactionPublisher(p)->writers) : 0) : 0)

static v_transactionPublisher
v_transactionPublisherNew(
    v_transactionGroupAdmin admin,
    c_ulong systemId,
    c_ulong publisherId)
{
    v_kernel kernel = v_objectKernel(admin);
    v_transactionPublisher _this;

    assert(kernel);
    assert(C_TYPECHECK(admin, v_transactionGroupAdmin));

    _this = v_transactionPublisher(v_objectNew(kernel, K_TRANSACTIONPUBLISHER));
    if (_this) {
        _this->systemId = systemId;
        _this->publisherId = publisherId;
        _this->admin = admin;
        _this->transactions = c_listNew(v_kernelType(kernel, K_TRANSACTIONGROUP));
        _this->writers = c_tableNew(v_kernelType(kernel, K_TRANSACTIONGROUPWRITER),
                                    "gid.systemId,gid.localId,gid.serial");
        TRACE_COHERENT_UPDATE(ADMIN_OWNER(admin),
                              "admin(0x%"PA_PRIxADDR") PID(%d) => 0x%"PA_PRIxADDR"\n",
                              (os_address)admin, publisherId,
                              (os_address)_this);
    } else {
        OS_REPORT(OS_ERROR, "v_transactionPublisherNew", OS_ERROR,
                  "Failed to allocate v_transactionPublisher object");
        assert(FALSE);
    }
    return _this;
}

static v_transactionGroup
v_transactionGroupNew(
    v_transactionPublisher publisher,
    v_transactionGroupAdmin admin,
    v_transaction transaction)
{
    v_transactionGroup group;
    v_kernel kernel = v_objectKernel(transaction);

    assert(admin != NULL);
    assert(transaction != NULL);
    assert(C_TYPECHECK(publisher, v_transactionPublisher));
    assert(C_TYPECHECK(admin, v_transactionGroupAdmin));
    assert(C_TYPECHECK(transaction, v_transaction));

    group = v_transactionGroup(v_objectNew(kernel, K_TRANSACTIONGROUP));
    if (group) {
        group->transactionId = transaction->eot->transactionId;
        group->transactions = c_listNew(v_kernelType(kernel, K_TRANSACTION));
        group->aborted = FALSE;
        if (publisher == NULL) {
            group->publisher = NULL;
            group->publisherId = 0;
        } else {
            group->publisher = (c_voidp)publisher;
            group->publisherId = publisher->publisherId;
        }
        group->admin = admin;
        group->matchCount = 0;
        group->writers = c_tableNew(v_kernelType(kernel, K_TRANSACTIONGROUPWRITER),
                                    "gid.systemId,gid.localId,gid.serial");
        group->obsolete = FALSE;
        group->allocTime = os_timeEGet();
        group->writeTime = v_message(transaction->eot)->writeTime;
        pa_st32(&group->historyLinks, 0);

        TRACE_COHERENT_UPDATE(PUBLISHER_OWNER(publisher),
                              "PID(%d) TID(%d) => 0x%"PA_PRIxADDR"\n",
                              group->publisherId,
                              transaction->eot->transactionId, (os_address)group);
    } else {
        OS_REPORT(OS_ERROR, "v_transactionGroupNew", OS_ERROR,
                  "Failed to allocate v_transactionGroup object");
        assert(FALSE);
    }
    return group;
}

static v_transactionGroupWriter
v_transactionGroupWriterNew (
    v_transactionPublisher publisher,
    v_gid writerGID)
{
    v_transactionGroupWriter _this;
    v_kernel kernel = v_objectKernel(publisher);

    assert(publisher);
    assert(C_TYPECHECK(publisher, v_transactionPublisher));

    _this = v_transactionGroupWriter(v_objectNew(kernel, K_TRANSACTIONGROUPWRITER));
    if (_this) {
        _this->gid = writerGID;
        _this->discovered = FALSE;
        _this->publisher = publisher;
        _this->readers = c_tableNew(v_kernelType(kernel, K_TRANSACTIONGROUPREADER),
                                    "gid.systemId,gid.localId,gid.serial");
        _this->rxo = NULL;
        _this->topicName = NULL;

        TRACE_COHERENT_UPDATE(PUBLISHER_OWNER(publisher),
                              "PID(%d) Writer(%u) => 0x%"PA_PRIxADDR"\n",
                              publisher->publisherId,
                              writerGID.localId, (os_address)_this);
    } else {
        OS_REPORT(OS_ERROR, "v_transactionGroupWriterNew", OS_ERROR,
                  "Failed to allocate v_transactionGroupWriter object");
        assert(FALSE);
    }
    return _this;
}

static void
v_transactionGroupInsertTransaction (
    v_transactionGroup _this,
    v_transaction transaction)
{
    struct c_collectionIter it;
    v_transaction txn;
    c_bool duplicate = FALSE;
    os_uint32 oldValue, value;

    assert(_this);
    assert(transaction);
    assert(C_TYPECHECK(_this, v_transactionGroup));
    assert(C_TYPECHECK(transaction, v_transaction));

    TRACE_COHERENT_UPDATE(TRANSACTION_OWNER(_this),
                          "Group(0x%"PA_PRIxADDR") Transaction(0x%"PA_PRIxADDR") TID(%d) PID(%d) %s\n",
                          (os_address)_this,
                          (os_address)transaction,
                          transaction->eot->transactionId,
                          transaction->eot->publisherId,
                          v_topicName(v_transactionGetAdmin(transaction)->topic));
    if (v_transactionIsAborted(transaction)) {
        /* Mark group transaction as aborted, on flush this flag is reevaluated. */
        _this->aborted = TRUE;
    }




    for (txn = c_collectionIterFirst(_this->transactions, &it); txn; txn = c_collectionIterNext(&it)) {
        if (txn->admin == transaction->admin && txn->writer == transaction->writer) {
            TRACE_COHERENT_UPDATE(TRANSACTION_OWNER(_this),
                                  "Received duplicate transaction TID(%d) PID(%d)\n",
                                  transaction->eot->transactionId,
                                  transaction->eot->publisherId);
            duplicate = TRUE;
            break;
        }
    }
    if (!duplicate) {
        if  (v_objectKind(TRANSACTION_OWNER(_this)) == K_KERNEL) {
            oldValue = pa_ld32(&_this->historyLinks);
            value = pa_add32_nv(&_this->historyLinks, pa_ld32(&transaction->historyLinks));

            if ((value > 0) && (oldValue == 0)) {
                v_transactionGroupAdmin admin = v_transactionGroupAdmin(_this->admin);
                v_transactionGroup found;

                /* Transaction already has samples commited in the group instance so
                 * it needs to be added to the history
                 */
                TRACE_COHERENT_UPDATE(TRANSACTION_OWNER(_this), "Group(0x%"PA_PRIxADDR") PID(%u) TID(%u) "
                                      "add to history\n",
                                      (os_address)_this, _this->publisherId, _this->transactionId);

                found = c_append(admin->history, _this);
                assert(found == _this);
                OS_UNUSED_ARG(found);
            }
        }
        transaction->transactionGroup = _this;
        c_append(_this->transactions, transaction);
    }
}

/**
 * \brief              This operation checks for data within the group.
 *
 * \param _this      : The v_transactionGroup this operation operates on.
 *
 * \return           : TRUE when the group still contains data, FALSE otherwise.
 */
static c_bool
v_transactionGroupHasData(
    v_transactionGroup _this)
{
    return (c_count(_this->transactions) > 0);
}

static c_bool
writerDiscovered(
    c_object o,
    c_voidp arg)
{
    v_transactionGroupWriter writer = v_transactionGroupWriter(o);
    OS_UNUSED_ARG(arg);
    return writer->discovered;
}

/**
 * This operation determines the completeness of a group coherent transaction.
 *
 * A group is complete if all the writers listed in the EOT message are discovered
 * and the number of received transactions is equal to the matchCount.
 * The matchCount is calculated differently for kernel and subscriber owned
 * transactionGroupAdmins. For the kernel owned the matchCount equals the number of
 * writers listed in the EOT message. For the subscriber owned the matchCount is
 * equal to total number of readers matching the writers listed in the EOT message,
 * a writer can have 0 to multiple reader matches.
 */
static c_bool
v_transactionGroupComplete(
    v_transactionGroup _this)
{
    c_bool complete = FALSE;

    assert(_this);
    assert(C_TYPECHECK(_this, v_transactionGroup));

    if (c_count(_this->transactions) == _this->matchCount) {
        complete = c_walk(_this->writers, writerDiscovered, NULL);
    }
    TRACE_COHERENT_UPDATE(TRANSACTION_OWNER(_this), "Group(0x%"PA_PRIxADDR") PID(%u) TID(%u) "
                          "number of transactions (%u), matchCount (%u) %s\n",
                          (os_address)_this, _this->publisherId, _this->transactionId,
                          c_count(_this->transactions), _this->matchCount, complete?"complete":"incomplete");

    return complete;
}

/**
 * This operation flushes all transactions belonging to this group.
 * A precondition is that the group is complete.
 */
static void
v_transactionGroupFlush(
    v_transactionGroup _this,
    v_transactionAdmin owner,
    c_bool reevaluateAbort)
{
    v_transaction txn;
    struct c_collectionIter it;
    c_bool abort = FALSE;

    assert(_this);
    assert(C_TYPECHECK(_this, v_transactionGroup));

    if (_this->aborted) {
        if (reevaluateAbort) {
            /* If one of the transactions is aborted then the whole group is aborted. */
            for (txn = c_collectionIterFirst(_this->transactions, &it); txn; txn = c_collectionIterNext(&it)) {
                abort = v_transactionIsAborted(txn);
                if (abort == TRUE) { break; }
            }
        } else {
            abort = TRUE;
        }
    }

    for (txn = c_collectionIterFirst(_this->transactions, &it); txn; txn = c_collectionIterNext(&it)) {
        if (abort) {
            v_transactionAbort(txn);
            if (!_this->deleted) {
                v_transactionNotifySampleLost(txn, owner);
            }
        }
        if (txn->admin == owner) {
            v_transactionFlush_nl(txn);
        } else {
            v_transactionFlush(txn);
        }
    }
}

/**
 * \brief              This operation will lookup the transactionPublisher object for
 *                     the given publisher id, if the object doesn't exist it will
 *                     first create one.
 *
 * \param _this      : The transactioniGroupAdmin this operation operates on.
 * \param systemId   : The publishers systemId, the publisher id is not system wide unique
 *                     so we need the systemId to uniquely identify publishers.
 * \param publisherId: The publisher id to look for.
 *
 * \return           : The found or newly created transactionPublisher object.
 */
static v_transactionPublisher
v_transactionGroupAdminLookupPublisher(
    v_transactionGroupAdmin _this,
    c_ulong systemId,
    c_ulong publisherId)
{
    C_STRUCT(v_transactionPublisher) template;
    v_transactionPublisher publisher;

    assert(_this);
    assert(C_TYPECHECK(_this, v_transactionGroupAdmin));

    template.systemId = systemId;
    template.publisherId = publisherId;
    publisher = c_find(_this->publishers, &template);
    if (publisher == NULL) {
        publisher = v_transactionPublisherNew(_this, template.systemId, template.publisherId);
        ospl_c_insert(_this->publishers, publisher);
    }
    TRACE_COHERENT_UPDATE(ADMIN_OWNER(_this),
                          "Publisher(0x%"PA_PRIxADDR") PID(%d) has %d writers\n",
                          (os_address)publisher, publisherId, v_transactionPublisherCount(publisher));
    return publisher;
}

static v_transactionPublisher
v_transactionGroupAdminFindPublisher(
    v_transactionGroupAdmin _this,
    c_ulong systemId,
    c_ulong publisherId)
{
    C_STRUCT(v_transactionPublisher) template;

    assert(_this);
    assert(C_TYPECHECK(_this, v_transactionGroupAdmin));

    template.systemId = systemId;
    template.publisherId = publisherId;

    return c_find(_this->publishers, &template);
}

struct findPublisherHelper {
    v_gid writerGID;
    v_transactionPublisher publisher;
};

static c_bool
findPublisher(
    c_object o,
    c_voidp arg)
{
    c_bool result = TRUE;
    v_transactionPublisher publisher = v_transactionPublisher(o);
    struct findPublisherHelper *helper = (struct findPublisherHelper*) arg;
    C_STRUCT(v_transactionGroupWriter) dummy;
    v_transactionGroupWriter writer;

    memset(&dummy, 0, C_SIZEOF(v_transactionGroupWriter));
    dummy.gid = helper->writerGID;
    writer = c_find(publisher->writers, &dummy);
    if (writer) {
        helper->publisher = c_keep(publisher);
        c_free(writer);
        result = FALSE;
    }

    return result;
}

static v_transactionPublisher
v_transactionGroupAdminFindPublisherWithWriterGID(
    v_transactionGroupAdmin _this,
    v_gid writerGID)
{
    struct findPublisherHelper helper;

    assert(_this);
    assert(C_TYPECHECK(_this, v_transactionGroupAdmin));

    helper.writerGID = writerGID;
    helper.publisher = NULL;
    (void)c_walk(_this->publishers, findPublisher, &helper);

    if (helper.publisher) {
        TRACE_COHERENT_UPDATE(ADMIN_OWNER(_this),
                              "Publisher(0x%"PA_PRIxADDR") PID(%d) has %d writers\n",
                              (os_address)helper.publisher, helper.publisher->publisherId, v_transactionPublisherCount(helper.publisher));
    }

    return helper.publisher;
}

static v_transactionGroupWriter
v_transactionPublisherLookupWriter(
    v_transactionPublisher _this,
    v_gid writerGID)
{
    C_STRUCT(v_transactionGroupWriter) dummy;
    v_transactionGroupWriter writer;

    memset(&dummy, 0, C_SIZEOF(v_transactionGroupWriter));
    dummy.gid = writerGID;
    writer = v_transactionGroupWriter(c_find(_this->writers, &dummy));
    if (writer == NULL) {
        writer = v_transactionGroupWriterNew(_this, writerGID);
        writer = ospl_c_insert(_this->writers, writer);
        assert(writer);
    }
    TRACE_COHERENT_UPDATE(PUBLISHER_OWNER(_this),
                          "Writer(0x%"PA_PRIxADDR") WID(%u) %sdiscovered\n",
                          (os_address)writer, writerGID.localId,
                          writer->discovered?"":"un");

    return writer;
}

static v_transactionGroupWriter
v_transactionPublisherFindWriter(
    v_transactionPublisher _this,
    v_gid writerGID)
{
    v_transactionGroupWriter writer;
    C_STRUCT(v_transactionGroupWriter) dummy;
    memset(&dummy, 0, C_SIZEOF(v_transactionGroupWriter));
    dummy.gid = writerGID;

    if (_this->writers ==  NULL) {
        assert(v_objectKind(PUBLISHER_OWNER(_this)) != K_SUBSCRIBER);
        writer = NULL;
    } else {
        writer = v_transactionGroupWriter(c_find(_this->writers, &dummy));
    }
    return writer;
}

static v_transactionGroupWriter
v_transactionPublisherRemoveWriter(
    v_transactionPublisher _this,
    v_gid writerGID)
{
    v_transactionGroupWriter writer;
    C_STRUCT(v_transactionGroupWriter) dummy;
    memset(&dummy, 0, C_SIZEOF(v_transactionGroupWriter));
    dummy.gid = writerGID;

    if (_this->writers ==  NULL) {
        assert(v_objectKind(PUBLISHER_OWNER(_this)) != K_SUBSCRIBER);
        writer = NULL;
    } else {
        writer = v_transactionGroupWriter(c_remove(_this->writers, &dummy, NULL, NULL));
    }
    return writer;
}

static c_bool
compareTransactionId(
    c_object o,
    c_voidp arg)
{
    v_transactionGroup transaction = v_transactionGroup(o);
    v_transactionGroup template = (v_transactionGroup)arg; /* simple cast without type checking because template is no necessarily a database object. */

    return (transaction->transactionId == template->transactionId);
}

static v_transactionGroup
v_transactionPublisherLookupTransaction(
    v_transactionPublisher _this,
    v_transaction transaction)
{
    C_STRUCT(v_transactionGroup) template;
    v_transactionGroup group;
#if TRACE_COHERENT
    c_bool inserted = FALSE;
#endif

    assert(_this != NULL);
    assert(transaction != NULL);
    assert(C_TYPECHECK(_this, v_transactionPublisher));
    assert(C_TYPECHECK(transaction, v_transaction));

    template.transactionId = transaction->eot->transactionId;
    group = v_transactionGroup(c_listTemplateFind(_this->transactions, compareTransactionId, &template));
    if (group == NULL) {
        group = v_transactionGroupNew(_this, _this->admin, transaction);
        group = ospl_c_insert(_this->transactions, group);
#if TRACE_COHERENT
        inserted = TRUE;
#endif
    }

    TRACE_COHERENT_UPDATE(PUBLISHER_OWNER(_this), "PID(%d) TID(%d) "
                          "GroupTransaction(0x%"PA_PRIxADDR") and insert "
                          "Writer(0x%"PA_PRIxADDR") WGID(%d)%s\n",
                          transaction->eot->publisherId,
                          transaction->eot->transactionId, (os_address)group,
                          (os_address)transaction->writer,
                          v_message(transaction->eot)->writerGID.localId, inserted?" NEW":"");

    return group;
}

static c_bool
v_transactionGroupWriterMatchReader(
    v_transactionGroupWriter _this,
    v_transactionGroupReader reader,
    c_bool dispose)
{
    c_bool result = FALSE;
    v_transactionGroupReader r;

    if ((dispose == FALSE) &&
        (strcmp(v_topicName(reader->topic), _this->topicName) == 0) &&
        (v_rxoDataCompatible(_this->rxo, reader->rxo) == TRUE)) {
        r = c_find(_this->readers, reader);
        if (r == NULL) {
            ospl_c_insert(_this->readers, reader);
            result = TRUE;
        }
        c_free(r);
    } else {
        r = c_remove(_this->readers, reader, NULL, NULL);
        if (r) {
            result = TRUE;
            c_free(r);
        }
    }
    return result;
}

struct matchWriterHelper {
    v_transactionGroupWriter writer;
    c_bool updated;
};

static c_bool
matchWriter(
    c_object o,
    c_voidp arg)
{
    v_transactionGroupReader reader = v_transactionGroupReader(o);
    struct matchWriterHelper *helper = (struct matchWriterHelper *) arg;

    assert(reader->topic);
    assert(helper->writer->topicName);

    helper->updated |= v_transactionGroupWriterMatchReader(helper->writer, reader, FALSE);

    return TRUE;
}

struct matchReaderHelper {
    v_transactionGroupReader reader;
    c_bool dispose;
    c_bool updated;
};

static c_bool
matchReader(
    c_object o,
    c_voidp arg)
{
    v_transactionGroupWriter writer = v_transactionGroupWriter(o);
    struct matchReaderHelper *helper = (struct matchReaderHelper *) arg;

    assert(helper->reader->topic);
    if (writer->discovered) {
        assert(writer->rxo);
        assert(writer->topicName);
        helper->updated |= v_transactionGroupWriterMatchReader(writer, helper->reader, helper->dispose);
    }

    return TRUE;
}

static c_bool
v_transactionGroupWriterMatch(
    v_transactionGroupWriter _this,
    v_transactionGroupAdmin admin)
{
    struct matchWriterHelper helper;

    assert(_this);
    assert(_this->publisher);

    helper.updated = FALSE;
    helper.writer = _this;

    if (v_objectKind(PUBLISHER_OWNER(_this->publisher)) == K_SUBSCRIBER) {
        (void)c_walk(admin->readers, matchWriter, &helper);
    }

    return helper.updated;
}

static c_ulong
v_transactionGroupWriterGetMatches(
    v_transactionGroupWriter _this)
{
    c_ulong matches = 0;

    assert(_this);
    assert(_this->publisher);

    if (_this->discovered == TRUE) {
        if (v_objectKind(PUBLISHER_OWNER(_this->publisher)) == K_SUBSCRIBER) {
            matches = c_count(_this->readers);
        } else if (_this->rxo->durability.v.kind != V_DURABILITY_VOLATILE) {
            matches = 1;
        }
    }
    return matches;
}

static c_bool
publisherMatch(
    c_object o,
    c_voidp arg)
{
    v_transactionPublisher publisher = v_transactionPublisher(o);

    (void)c_walk(publisher->writers, matchReader, arg);

    return TRUE;
}

static c_bool
v_transactionGroupAdminMatchReader(
    v_transactionGroupAdmin _this,
    v_transactionGroupReader reader,
    c_bool dispose)
{
    struct matchReaderHelper helper;

    helper.updated = FALSE;
    helper.dispose = dispose;
    helper.reader = reader;
    (void)c_walk(_this->publishers, publisherMatch, &helper);

    return helper.updated;
}

static c_bool
calculateMatchCount(
    c_object o,
    c_voidp arg)
{
    v_transactionGroupWriter writer = v_transactionGroupWriter(o);
    c_ulong *cnt = (c_ulong*)arg;

    assert(cnt);
    assert(writer);
    assert(writer->publisher);

    (*cnt) += v_transactionGroupWriterGetMatches(writer);

    return TRUE;
}

/**
 * \brief              This operation will re-calculate the matchCount of
 *                     the given transactionGroup.
 *                     It does this by walking over all writers and add the
 *                     number of connected readers of these writers to get
 *                     to the new matchCount.
 *
 * \param _this      : The v_transactionGroup this operation operates on.
 *
 * \return           : void
 */
static void
v_transactionGroupRecalculateMatchCount(
    v_transactionGroup _this)
{
    c_ulong oldCount;

    assert(_this);
    assert(C_TYPECHECK(_this, v_transactionGroup));

    oldCount = _this->matchCount;

    /* Run through the discovered writers list to determine how many matches there are.
     * When publisherId==0, then it should not change.
     */
    if ((_this->publisherId != 0) && (_this->publisher != NULL)) {
        _this->matchCount = 0;
        (void)c_walk(_this->writers, calculateMatchCount, &_this->matchCount);
    }

    if (oldCount != _this->matchCount) {
        TRACE_COHERENT_UPDATE(TRANSACTION_OWNER(_this), "PID(%u) Group(0x%"PA_PRIxADDR") TID(%u) writers(%d) matchCount recalculated %u->%u\n",
                              _this->publisherId, (os_address)_this, _this->transactionId,
                              (int)c_count(_this->writers),
                              oldCount, _this->matchCount);
    }
}

/**
 * \brief              This operation removes reader related info (like reader/writer
 *                     match and transactions) from the given group.
 *
 * \param _this      : The v_transactionGroup this operation operates on.
 * \param reader     : Reader for which to remove the information
 *
 * \return           : void
 */
static void
v_transactionGroupRemoveReader(
    v_transactionGroup       _this,
    v_transactionGroupReader reader)
{
    struct matchReaderHelper helper;
    struct c_collectionIterD it;
    v_transaction txn;
    c_bool  oldComplete;

    /* Used for sanity check. */
    oldComplete = v_transactionGroupComplete(_this);

    TRACE_COHERENT_UPDATE(TRANSACTION_OWNER(_this), "PID(%u) Group(0x%"PA_PRIxADDR") TID(%u) remove reader(%d)\n",
                          _this->publisherId, (os_address)_this, _this->transactionId,
                          reader->gid.localId);

    /* First, remove all transactions that are related to this reader.
     * But do not clean KERNEL owned transactions as these are required for alignment.
     */
    if ((_this->publisher != NULL) &&
        (v_objectKind(TRANSACTION_OWNER(_this)) != K_KERNEL)) {

        for (txn = c_collectionIterDFirst(_this->transactions, &it); txn; txn = c_collectionIterDNext(&it)) {
            v_gid ownerGid = v_publicGid(v_public(v_transactionAdmin(txn->admin)->owner));
            if (v_gidEqual(ownerGid, reader->gid)) {
                TRACE_COHERENT_UPDATE(TRANSACTION_OWNER(_this), "Group(0x%"PA_PRIxADDR") PID(%u) TID(%u) "
                                      "remove transaction from reader(%d)\n",
                                      (os_address)_this, _this->publisherId, _this->transactionId,
                                      reader->gid.localId);
                c_collectionIterDRemove(&it);
                c_free(txn);
                break;
            }
        }
    }

    /* Second, remove the links between this reader and the discovered writers. */
    helper.updated = FALSE;
    helper.dispose = TRUE;
    helper.reader = reader;
    (void)c_walk(_this->writers, matchReader, &helper);

    /* Third, re-calculate the matchCount. */
    if (helper.updated) {
        v_transactionGroupRecalculateMatchCount(_this);
    }

    /* Sanity check. */
    if (oldComplete) {
        /* We should still be complete if we not became empty... */
        if (v_transactionGroupHasData(_this)) {
            assert(v_transactionGroupComplete(_this));
        }
    }
}

static void
v_transactionGroupWriterUpdate(
    v_transactionGroupWriter _this,
    const struct v_publicationInfo *info)
{
    v_kernel kernel;

    assert(_this);
    assert(C_TYPECHECK(_this, v_transactionGroupWriter));
    assert(info);

    kernel = v_objectKernel(_this);
    if (_this->discovered) {
        c_free(_this->rxo);
    } else {
        /* Topic cannot be changed so only set it once */
        _this->topicName = c_keep(info->topic_name);
    }
    _this->rxo = v_kernel_rxoDataFromPublicationInfo(kernel, info); /* pass reference */
    _this->discovered = TRUE;
}

static void
v_transactionPublisherRecalculateMatchCounts(
    v_transactionPublisher _this)
{
    v_transactionGroup group;
    struct c_collectionIter it;

    for (group = c_collectionIterFirst(_this->transactions, &it); group; group = c_collectionIterNext(&it)) {
        v_transactionGroupRecalculateMatchCount(group);
    }
}

static void
v_transactionPublisherMarkObsolete(
    v_transactionPublisher _this,
    os_timeW writeTime,
    c_ulong transactionId)
{
    struct c_collectionIterD it;
    v_transactionGroup tg;
    os_compare eq;

    assert(_this);
    assert(C_TYPECHECK(_this, v_transactionPublisher));

    for (tg = c_collectionIterDFirst(_this->transactions, &it); tg; tg = c_collectionIterDNext(&it)) {
        eq = os_timeWCompare(tg->writeTime, writeTime);
        if (eq == OS_EQUAL) {
            if (tg->transactionId < transactionId) {
                eq = OS_LESS;
            }
        }
        if (eq == OS_LESS) {
            tg->obsolete = TRUE;
        }
    }
}

static c_bool
v_transactionGroupAdminAddGroupToPending(
    v_transactionGroupAdmin _this,
    v_transactionGroup group)
{
    c_bool result = FALSE;
    v_transactionGroup found;
    v_transactionPublisher publisher;

    assert(_this);
    assert(C_TYPECHECK(_this, v_transactionGroupAdmin));
    assert(group);
    assert(C_TYPECHECK(group, v_transactionGroup));

    publisher = v_transactionPublisher(group->publisher);
    if ((group->obsolete == FALSE) &&
        (group->aborted == FALSE)){
        v_transactionPublisherMarkObsolete(publisher, group->writeTime, group->transactionId);
    }
    group->publisher = NULL;

    if (v_transactionGroupHasData(group)) {
        found = c_append(_this->pending, group);
        assert(found == group);
        OS_UNUSED_ARG(found);
        result = TRUE;
        TRACE_COHERENT_UPDATE(ADMIN_OWNER(_this),
                              "Add %sgroup TID(%u) (0x%"PA_PRIxADDR") to pending list (size:%u)\n",
                              (group->aborted ?"aborted ":""),
                              group->transactionId,
                              (os_address)group,
                              c_count(_this->pending));
    } else {
        TRACE_COHERENT_UPDATE(ADMIN_OWNER(_this),
                              "Removing complete but empty group TID(%u) (0x%"PA_PRIxADDR")\n",
                              group->transactionId,
                              (os_address)group);
    }
    return result;
}

/**
 * \brief              This operation moves all complete transactions from the
 *                     transactionPublisher to the pending list. This operation
 *                     is required because of the asynchronous detection of writers.
 *
 * \param _this      : The transactionGroupAdmin this operation operates on.
 * \param publisher  : The transactionPublisher to check for complete transactions
 *
 * \return           : TRUE when at least one transaction has become complete
 */
static c_bool
v_transactionGroupAdminPendCompleteTransactions(
    v_transactionGroupAdmin _this,
    v_transactionPublisher publisher)
{
    c_bool result = FALSE;
    v_transactionGroup group;
    struct c_collectionIterD it;

    for (group = c_collectionIterDFirst(publisher->transactions, &it); group; group = c_collectionIterDNext(&it)) {
        if (v_transactionGroupComplete(group) == TRUE) {
            result = v_transactionGroupAdminAddGroupToPending(_this, group);
            c_collectionIterDRemove(&it);
            c_free(group);
        }
    }
    return result;
}

static c_bool
publisherPurgeObsolete(
    c_object o,
    c_voidp arg);

/**
 * \brief              This operation moves all obsolete transactions from the
 *                     transactionPublishers to the pending list when durability
 *                     is not aligning data.
 *
 * \param _this      : The transactionGroupAdmin this operation operates on.
 *
 * \return           : void
 */
static void
purgeObsolete (
    v_transactionGroupAdmin _this)
{
    v_kernel kernel = v_objectKernel(v_object(_this));

    if ((v_kernelGetDurabilitySupport(kernel) == TRUE) &&
        (v_kernelGetAlignedState(kernel) == FALSE)) {
        /* Durability is currently not aligned, do not purge obsolete groups as
         * they can still become complete. */
    } else {
        (void)c_walk(_this->publishers, publisherPurgeObsolete, _this->pending);
    }
}

/**
 * This operation will flush any pending transactions.
 * Should only be called when protected by an AccessLock
 */
void
v_transactionGroupAdminFlushPending(
    v_transactionGroupAdmin _this,
    v_transactionAdmin admin)
{
    v_transactionGroup group;
    c_list pending = NULL;
    v_kernel kernel;

    assert(_this);
    assert(C_TYPECHECK(_this, v_transactionGroupAdmin));
    assert(C_TYPECHECK(admin, v_transactionAdmin));

    c_mutexLock(&_this->mutex);
    kernel = v_objectKernel(_this);

    purgeObsolete(_this);

    if (c_count(_this->pending) > 0) {
        pending = _this->pending;
        _this->pending = c_listNew(v_kernelType(kernel, K_TRANSACTIONGROUP));
        assert(_this->pending);
    }

    c_mutexUnlock(&_this->mutex);

    while (pending && ((group = c_take(pending)) != NULL)) {
        TRACE_COHERENT_UPDATE(ADMIN_OWNER(_this),
                              "PID(%u) Flush group (0x%"PA_PRIxADDR") TID(%u) "
                              "from pending list\n",
                              group->publisherId, (os_address)group,
                              group->transactionId);

        v_transactionGroupFlush(group, admin, TRUE);
        c_free(group);
    }
    c_free(pending);
}

static c_bool
publisherRecalculateMatchCounts(
    c_object o,
    c_voidp arg)
{
    v_transactionPublisher publisher = v_transactionPublisher(o);
    OS_UNUSED_ARG(arg);
    v_transactionPublisherRecalculateMatchCounts(publisher);
    return TRUE;
}

static c_bool
v_transactionPublisherRemoveReader(
    c_object o,
    c_voidp arg)
{
    v_transactionGroup    group;
    struct c_collectionIterD it;
    v_transactionPublisher   publisher = v_transactionPublisher(o);
    v_transactionGroupReader groupReader = v_transactionGroupReader(arg);

    assert(publisher);
    assert(groupReader);

    for (group = c_collectionIterDFirst(publisher->transactions, &it); group; group = c_collectionIterDNext(&it)) {
        v_transactionGroupRemoveReader(group, groupReader);
        if (v_transactionGroupHasData(group) == FALSE) {
            /* Group is empty so remove it */
            c_collectionIterDRemove(&it);
            TRACE_COHERENT_UPDATE(PUBLISHER_OWNER(publisher),
                                  "PID(%u) group(0x%"PA_PRIxADDR") TID(%u) removed as it became empty\n",
                                  publisher->publisherId, (os_address)group, group->transactionId);
            c_free(group);
        }
    }
    (void)v_transactionGroupAdminPendCompleteTransactions(v_transactionGroupAdmin(publisher->admin), publisher);

    return TRUE;
}

_Check_return_
_Ret_notnull_
_Pre_satisfies_(v_objectKind(owner) == K_KERNEL || v_objectKind(owner) == K_SUBSCRIBER)
v_transactionGroupAdmin
v_transactionGroupAdminNew(
    _In_ v_object owner)
{
    v_transactionGroupAdmin _this;
    v_kernel kernel;
    c_base base;

    assert(owner);

    kernel = v_objectKernel(owner);
    _this = v_transactionGroupAdmin(v_objectNew(kernel, K_TRANSACTIONGROUPADMIN));
    base = c_getBase(kernel);
    (void)c_mutexInit(base, &_this->mutex);

    if (v_objectKind(owner) == K_SUBSCRIBER) {
        _this->readers = c_tableNew(v_kernelType(kernel, K_TRANSACTIONGROUPREADER),
                                    "gid.systemId,gid.localId,gid.serial");
    } else {
        assert(v_objectKind(owner) == K_KERNEL);
        _this->readers = NULL;
    }
    _this->publishers = c_tableNew(v_kernelType(kernel, K_TRANSACTIONPUBLISHER),
                                   "systemId, publisherId");
    _this->pending = c_listNew(v_kernelType(kernel, K_TRANSACTIONGROUP));
    if (v_objectKind(owner) == K_KERNEL) {
        /* Only kernel owned transactionGroupAdmins need keep to history as this
         * is a list of EOTs (wrapped in v_transactionGroups and v_transactions)
         * which are required for alignment.
         */
        _this->history = c_listNew(v_kernelType(kernel, K_TRANSACTIONGROUP));
    } else {
        _this->history = NULL;
    }
    _this->owner = owner;

    TRACE_COHERENT_UPDATE(owner, "=> 0x%"PA_PRIxADDR"\n", (os_address)_this);

    return _this;
}

struct processPublicationInfoHelper {
    v_transactionGroupWriter writer;
    v_kernel kernel;
};

static os_boolean
processPublicationInfo(
    const v_message msg,
    c_voidp arg)
{
    struct processPublicationInfoHelper *a = (struct processPublicationInfoHelper *)arg;
    struct v_publicationInfo *info;

    info = v_builtinPublicationInfoData(msg);
    if (v_gidEqual(info->key, a->writer->gid)) {
        a->writer->rxo = v_kernel_rxoDataFromPublicationInfo(a->kernel, info);
        a->writer->topicName = c_keep(info->topic_name);
        a->writer->discovered = TRUE;
        return OS_FALSE;
    }
    return OS_TRUE;
}


/**
 * This operation inserts complete transactions into the group admin.
 * The transaction administration will move transactions that become complete
 * and belong to a group coherent update into this transactionGroup and thereby
 * handover the responsibility for flushing the transactions to this group when
 * the whole group has become complete.
 */
c_bool
v_transactionGroupAdminInsertTransaction(
    v_transactionGroupAdmin _this,
    v_transaction transaction,
    v_topic topic)
{
    c_bool result = FALSE;
    v_transactionPublisher publisher;
    v_transactionGroup group, found;
    v_transactionGroupWriter writer, thisWriter = NULL;
    v_kernel kernel;
    c_iter list = NULL;
    c_ulong i;
    c_ulong length;

    assert(_this);
    assert(C_TYPECHECK(_this, v_transactionGroupAdmin));
    assert(transaction);
    assert(C_TYPECHECK(transaction, v_transaction));
    assert(topic);
    assert(C_TYPECHECK(topic, v_topic));

    c_mutexLock(&_this->mutex);

    if (transaction->eot->publisherId == 0) {
        assert(c_arraySize(transaction->eot->tidList) == 1);

        group = v_transactionGroupNew(NULL, _this, transaction);
        if (group) {
            group->matchCount = 1;
            v_transactionGroupInsertTransaction(group, transaction);
            assert(v_transactionGroupComplete(group) == TRUE);
            c_append(_this->pending, group);
            c_free(group);
        }
        c_mutexUnlock(&_this->mutex);
        return TRUE;
    }

    publisher = v_transactionGroupAdminLookupPublisher(_this,
                                                       v_message(transaction->eot)->writerGID.systemId,
                                                       transaction->eot->publisherId);

    TRACE_COHERENT_UPDATE(ADMIN_OWNER(_this),
                          "PID(%d) transaction(0x%"PA_PRIxADDR") TID(%d) %d writers, topic %s\n",
                          transaction->eot->publisherId,
                          (os_address)transaction, transaction->eot->transactionId,
                          c_arraySize((c_array)transaction->eot->tidList),
                          v_topicName(topic));

    group = v_transactionPublisherLookupTransaction(publisher, transaction);
    if (group) {
        kernel = v_objectKernel(_this);
        if (group->matchCount == 0) {
            /* matchCount can only be zero when the transactionGroup is just created */
            struct v_tid *tidList = (struct v_tid *)transaction->eot->tidList;
            length = c_arraySize(transaction->eot->tidList);

            /* Create/find all writers part of the transaction */
            for (i=0; i<length; i++) {
                writer = v_transactionPublisherLookupWriter(publisher, tidList[i].wgid);
                /* Link discovered writer to the transactionGroup. */
                (void)ospl_c_insert(group->writers, writer);
                if (v_gidCompare(tidList[i].wgid, v_message(transaction->eot)->writerGID) == C_EQ) {
                    thisWriter = c_keep(writer);
                }
                /* discovery of thisWriter is done later */
                if ((thisWriter != writer) && (writer->discovered == FALSE))
                {
                    struct processPublicationInfoHelper helper;
                    assert(writer->topicName == NULL);
                    assert(writer->rxo == NULL);
                    helper.kernel = kernel;
                    helper.writer = writer;

                    /* TGwriter can only be discovered here if the PublicationInfo
                     * was received before the TGwriter was created. This also means
                     * that this discovery will not impact any other open transactions.
                     */
                    (void) v_kernelWalkPublications(kernel, processPublicationInfo, &helper);
                    if (writer->discovered) {
                        (void)v_transactionGroupWriterMatch(writer, _this);
                        TRACE_COHERENT_UPDATE(ADMIN_OWNER(_this),
                                              "TID(%d) PID(%d) discovered %sWID(%u) matches %u\n",
                                              transaction->eot->transactionId,
                                              transaction->eot->publisherId,
                                              (writer->rxo->durability.v.kind == V_DURABILITY_VOLATILE)?"volatile ":"",
                                              writer->gid.localId, v_transactionGroupWriterGetMatches(writer));
                    } else {
                        TRACE_COHERENT_UPDATE(ADMIN_OWNER(_this),
                                              "TID(%d) PID(%d) undiscovered WID(%u)\n",
                                              transaction->eot->transactionId,
                                              transaction->eot->publisherId,
                                              writer->gid.localId);
                    }
                }
                if (writer->discovered == TRUE) {
                    group->matchCount += v_transactionGroupWriterGetMatches(writer);
                }
                c_free(writer);
            }
        }

        if (thisWriter) {
            writer = thisWriter;
        } else {
            writer = v_transactionPublisherLookupWriter(publisher, v_message(transaction->eot)->writerGID);
        }
        if (writer->discovered == FALSE) {
            assert(writer->topicName == NULL);
            assert(writer->rxo == NULL);

            writer->topicName = c_keep(v_topicName(topic));
            writer->rxo = v_kernel_rxoDataFromMessageQos(kernel, v_message(transaction->eot)->qos);
            writer->discovered = TRUE;

            (void)v_transactionGroupWriterMatch(writer, _this);

            if (c_refCount(writer) > 3) {
                /* This writer is also referenced by other group transactions,
                 * update all groups including this one.
                 */
                v_transactionPublisherRecalculateMatchCounts(publisher);
            } else {
                group->matchCount += v_transactionGroupWriterGetMatches(writer);
            }

            TRACE_COHERENT_UPDATE(ADMIN_OWNER(_this),
                                  "TID(%d) PID(%d) implicitly discovered %sWID(%u) matches %u\n",
                                  transaction->eot->transactionId,
                                  transaction->eot->publisherId,
                                  (writer->rxo->durability.v.kind == V_DURABILITY_VOLATILE)?"volatile ":"",
                                  writer->gid.localId, v_transactionGroupWriterGetMatches(writer));
        }

        c_free(writer);

        TRACE_COHERENT_UPDATE(ADMIN_OWNER(_this),
                              "TID(%d) PID(%d) LookupGroup(0x%"PA_PRIxADDR") matches %u\n",
                              transaction->eot->transactionId,
                              transaction->eot->publisherId, (os_address)group,
                              group->matchCount);
        v_transactionGroupInsertTransaction(group, transaction);
        if (v_transactionGroupComplete(group) == TRUE) {
            result = v_transactionGroupAdminAddGroupToPending(_this, group);
            found = c_remove(publisher->transactions, group, NULL, NULL);
            assert(found == group);
            c_free(found);
        }

        c_free(group);
    }
    c_free(publisher);
    c_mutexUnlock(&_this->mutex);

    c_iterFree(list);
    return result;
}

void
v_transactionGroupAdminAddReader(
    v_transactionGroupAdmin _this,
    v_reader reader)
{
    v_transactionGroupReader groupReader, found = NULL;
    v_kernel kernel;
#ifdef TRACE_COHERENT
    v_topic topic;
#endif

    assert(_this);
    assert(C_TYPECHECK(_this, v_transactionGroupAdmin));
    assert(v_objectKind(ADMIN_OWNER(_this)) == K_SUBSCRIBER);

#ifdef TRACE_COHERENT
    topic = v_readerGetTopic(reader);
    if(topic){
        TRACE_COHERENT_UPDATE(ADMIN_OWNER(_this),
                              "add reader(%d) for topic '%s'\n",
                              v_publicGid(v_public(reader)).localId,
                              v_topicName(topic));
        c_free(topic);
    } else {
        TRACE_COHERENT_UPDATE(ADMIN_OWNER(_this),
              "add reader(%d), but it has already been deleted\n",
              v_publicGid(v_public(reader)).localId);
    }
#endif

    kernel = v_objectKernel(_this);
    groupReader = v_transactionGroupReader(v_objectNew(kernel, K_TRANSACTIONGROUPREADER));
    if (groupReader) {
        groupReader->gid = v_publicGid(v_public(reader));
        groupReader->topic = v_readerGetTopic(reader); /* pass reference */
        groupReader->rxo = v_kernel_rxoDataFromReaderQos(kernel, reader->qos);

        if(groupReader->topic){
            c_mutexLock(&_this->mutex);
            found = ospl_c_insert(_this->readers, groupReader);
            assert(found == groupReader);
            if (v_transactionGroupAdminMatchReader(_this, groupReader, FALSE)) {
                (void)c_walk(_this->publishers, publisherRecalculateMatchCounts, NULL);
            }
            c_mutexUnlock(&_this->mutex);
            c_free(groupReader->topic);
            c_free(found);
        }
    } else {
        OS_REPORT(OS_ERROR, OS_FUNCTION, OS_ERROR,
                  "Failed to allocate v_transactionGroupWriter object");
        assert(FALSE);
    }
}

void
v_transactionGroupAdminRemoveReader(
    v_transactionGroupAdmin _this,
    v_reader reader)
{
    v_transactionGroupReader groupReader;
    C_STRUCT(v_transactionGroupReader) dummy;
    v_transactionGroup group;
    struct c_collectionIterD it;

    assert(_this);
    assert(C_TYPECHECK(_this, v_transactionGroupAdmin));
    assert(v_objectKind(ADMIN_OWNER(_this)) == K_SUBSCRIBER);
    assert(reader);

    TRACE_COHERENT_UPDATE(ADMIN_OWNER(_this),
                          "remove reader(%d)\n",
                          v_publicGid(v_public(reader)).localId);

    memset(&dummy, 0, C_SIZEOF(v_transactionGroupReader));
    dummy.gid = v_publicGid(v_public(reader));

    c_mutexLock(&_this->mutex);
    groupReader = c_remove(_this->readers, &dummy, NULL, NULL);
    if (groupReader) {
        /* Remove all transactions related to this reader from all publishers. */
        (void)c_walk(_this->publishers, v_transactionPublisherRemoveReader, groupReader);

        /* Now, remove all transactions related to this reader from the pending list. */
        for (group = c_collectionIterDFirst(_this->pending, &it); group; group = c_collectionIterDNext(&it)) {
            /* Sanity check, publisher should always be cleared for pending transactions */
            assert(group->publisher == NULL);

            /* Remove the reader information from this group. */
            v_transactionGroupRemoveReader(group, groupReader);

            /* If the group is empty: remove it from the pending list. */
            if (v_transactionGroupHasData(group) == FALSE) {
                c_collectionIterDRemove(&it);

                TRACE_COHERENT_UPDATE(ADMIN_OWNER(_this),
                                      "group(0x%"PA_PRIxADDR") TID(%u) removed as it became empty\n",
                                      (os_address)group, group->transactionId);
                c_free(group);
            } else if (group->aborted) {
                c_collectionIterDRemove(&it);

                TRACE_COHERENT_UPDATE(ADMIN_OWNER(_this),
                                      "group(0x%"PA_PRIxADDR") TID(%u) removed as it became aborted\n",
                                      (os_address)group, group->transactionId);
                c_free(group);
            } else {
                assert(v_transactionGroupComplete(group));
            }
        }
        c_free(groupReader);
    }

    c_mutexUnlock(&_this->mutex);
}

c_bool
v_transactionGroupAdminNotifyPublication(
    v_transactionGroupAdmin _this,
    v_transactionWriter writer,
    c_bool dispose,
    struct v_publicationInfo *info)
{
    c_bool result = FALSE;
    struct c_collectionIter it;
    struct c_collectionIterD idt;
    v_transactionPublisher publisher = NULL;
    v_transactionPublisher found;
    v_transactionGroupWriter groupWriter;
    v_transactionGroup group;
    v_gid writerGID;
    c_ulong count;

    assert(_this);
    assert(C_TYPECHECK(_this, v_transactionGroupAdmin));
    assert(C_TYPECHECK(writer, v_transactionWriter));

    c_mutexLock(&_this->mutex);
    if ((writer) && (writer->publisherId != 0)) {
        publisher = v_transactionGroupAdminFindPublisher(_this,
                                                         writer->writerGID.systemId,
                                                         writer->publisherId);
        writerGID = writer->writerGID;
    } else if (info) {
        /* Received an update but do not know for which transaction writer */
        publisher = v_transactionGroupAdminFindPublisherWithWriterGID(_this, info->key);
        writerGID = info->key;
    } else {
        assert(FALSE);
    }

    if (publisher) {
        TRACE_COHERENT_UPDATE(ADMIN_OWNER(_this),
                              "Notify WID(%d) writer %p, info %p, dispose %s\n",
                              writerGID.localId, writer, info, dispose?"TRUE":"FALSE");

        assert(C_TYPECHECK(publisher, v_transactionPublisher));

        if (dispose) {
            C_STRUCT(v_transactionGroupWriter) dummy;
            memset(&dummy, 0, C_SIZEOF(v_transactionGroupWriter));
            dummy.gid = writerGID;
            /* Removal of a writer cannot make a previous incomplete transaction
             * complete. Removal of a writer does ensure that we'll never receive
             * data from it again and thus incomplete transactions will never
             * become complete.
             */
            groupWriter = v_transactionPublisherRemoveWriter(publisher, writerGID);
            c_free(groupWriter);

            count = c_count(publisher->writers);
            for (group = c_collectionIterDFirst(publisher->transactions, &idt); group; group = c_collectionIterDNext(&idt)) {
                if (v_transactionGroupComplete(group) == FALSE) {
                    /* Only remove groups that are affected by the disposal of this writer. */
                    if (c_exists(group->writers, &dummy)) {
                        group->aborted = TRUE;
                        group->obsolete = TRUE;
                        group->deleted = TRUE;
                        group->publisher = NULL;

                        c_append(_this->pending, group);
                        /* Remove incomplete group transactions from publisher */
                        c_collectionIterDRemove(&idt);
                        c_free(group);
                        result = TRUE;
                    }
                }
            }
            if (count == 0) {
                /* Publisher not longer has writer so it can be removed */
                found = c_remove(_this->publishers,publisher,NULL,NULL);
                c_free(found);
            }
        } else {
            groupWriter = v_transactionPublisherFindWriter(publisher, writerGID);
            if (groupWriter) {
                if (info) {
                    /* For newly discovered writers link info to the relaed groups. */
                    v_transactionGroupWriterUpdate(groupWriter, info);
                    (void)v_transactionGroupWriterMatch(groupWriter, _this);
                    for (group = c_collectionIterFirst(publisher->transactions, &it); group; group = c_collectionIterNext(&it)) {
                        /* Get the EOT writer list and verify that the group is affected by the discovered writer
                         * before updating the group admin.
                         */
                        v_transaction t = c_read(group->transactions);
                        if (t) {
                            c_ulong i, length;
                            struct v_tid *tidList = (struct v_tid *)t->eot->tidList;
                            length = c_arraySize(t->eot->tidList);
                            for (i=0; i<length; i++) {
                                if (v_gidCompare(tidList[i].wgid, writerGID) == C_EQ) {
                                    v_transactionGroupRecalculateMatchCount(group);
                                    break;
                                }
                            }
                        }
                    }
                }
                result = v_transactionGroupAdminPendCompleteTransactions(_this, publisher);
                c_free(groupWriter);
            }
        }
        c_free(publisher);
    }

    c_mutexUnlock(&_this->mutex);

    return result;
}

struct actionWalkArg {
    c_action action;
    c_voidp arg;
    v_group vgroup;
};

static c_bool
groupWalk(
    c_object o,
    c_voidp arg)
{
    c_bool result = TRUE;
    v_transactionGroup group = v_transactionGroup(o);
    struct actionWalkArg *a = (struct actionWalkArg *)arg;
    v_transaction txn;
    v_transactionAdmin admin;
    struct c_collectionIter it;

    if (!group->aborted) {
        for (txn = c_collectionIterFirst(group->transactions, &it); txn && result; txn = c_collectionIterNext(&it)) {
            admin = v_transactionGetAdmin(txn);
            if (v_objectKind(admin->owner) == K_GROUP) {
                if (v_group(admin->owner) == a->vgroup) {
                    result = a->action (txn, a->arg);
                }
            }
        }
    }
    return result;
}

static c_bool
publisherWalk(
    c_object o,
    c_voidp arg)
{
    v_transactionPublisher p = v_transactionPublisher(o);

    return c_walk(p->transactions, groupWalk, arg);
}

void
v_transactionGroupAdminWalkTransactions(
    v_transactionGroupAdmin _this,
    v_group vgroup,
    c_action action,
    c_voidp arg)
{
    struct actionWalkArg walkArg;

    assert(_this);
    assert(C_TYPECHECK(_this, v_transactionGroupAdmin));

    c_mutexLock(&_this->mutex);

    walkArg.action = action;
    walkArg.arg = arg;
    walkArg.vgroup = vgroup;

    (void)c_walk(_this->history, groupWalk, &walkArg);
    (void)c_walk(_this->pending, groupWalk, &walkArg);
    (void)c_walk(_this->publishers, publisherWalk, &walkArg);
    c_mutexUnlock(&_this->mutex);
}

void
v_transactionGroupAdminFlush(
    v_transactionGroupAdmin _this)
{
    assert(_this);
    assert(C_TYPECHECK(_this, v_transactionGroupAdmin));
    assert(v_objectKind(ADMIN_OWNER(_this)) == K_SUBSCRIBER);

    v_transactionGroupAdminFlushPending(_this, NULL);
}

struct transactionWriterMessageFind {
    v_instance instance;
    v_gid writerGid;
};

static c_bool
v_transactionGroupWriterMessageNotExist(
    c_object o,
    c_voidp arg)
{
    v_transaction transaction = v_transaction(o);
    struct transactionWriterMessageFind *info = arg;
    c_bool result = TRUE;
    c_ulong nrElements, i;

    nrElements = c_arraySize(transaction->elements);

    for (i = 0; (i < nrElements) && result; i++) {
        v_transactionElement element = transaction->elements[i];
        if (element) {
            if ((element->instance == info->instance) &&
                (v_gidCompare(element->message->writerGID, info->writerGid) == C_EQ)) {
                result = FALSE;
            }
        }
    }

    return result;
}

c_bool
v_transactionGroupAdminNoMessageFromWriterExist(
    v_transactionGroupAdmin _this,
    v_groupInstance instance,
    v_gid writerGid)
{
    c_bool result;
    struct transactionWriterMessageFind info;
    struct actionWalkArg walkArg;

    assert(_this);
    assert(C_TYPECHECK(_this, v_transactionGroupAdmin));
    assert(instance);

    info.instance = v_instance(instance);
    info.writerGid = writerGid;

    walkArg.action = v_transactionGroupWriterMessageNotExist;
    walkArg.arg = &info;
    walkArg.vgroup = v_group(instance->group);

    c_mutexLock(&_this->mutex);
    result = c_walk(_this->publishers, publisherWalk, &walkArg);
    c_mutexUnlock(&_this->mutex);

    return result;
}

static c_bool
publisherPurgeObsolete(
    c_object o,
    c_voidp arg)
{
    v_transactionPublisher publisher = v_transactionPublisher(o);
    struct c_collectionIterD it;
    v_transactionGroup group, found;
    c_list list = (c_list)arg;

    for (group = c_collectionIterDFirst(publisher->transactions, &it); group; group = c_collectionIterDNext(&it)) {
        if (group->obsolete == TRUE) {
            found = c_append(list, group);
            assert(found == group);
            OS_UNUSED_ARG(found);
            group->publisher = NULL;
            group->aborted = TRUE;
            c_collectionIterDRemove(&it);
            c_free(group);
        }
    }

    return TRUE;
}

/**
 * \brief              This operation links to the groupTransactions.
 *                     When this is the first link it is added to the history.
 *
 * \param _this      : The transactionGroupAdmin this operation operates on.
 *
 * \return           : None
 */
void
v_transactionGroupLink(
    v_transactionGroup _this)
{
    os_uint32 value;
    v_transactionGroupAdmin admin;
    v_transactionGroup found;

    if (_this) {
        value = pa_inc32_nv(&_this->historyLinks);

        if (value == 1) {
            admin = v_transactionGroupAdmin(_this->admin);
            assert(v_objectKind(admin->owner) == K_KERNEL);

            TRACE_COHERENT_UPDATE(TRANSACTION_OWNER(_this), "Group(0x%"PA_PRIxADDR") PID(%u) TID(%u) "
                                  "add to history\n",
                                  (os_address)_this, _this->publisherId, _this->transactionId);

            c_mutexLock(&admin->mutex);
            found = c_append(admin->history, _this);
            assert(found == _this);
            OS_UNUSED_ARG(found);
            c_mutexUnlock(&admin->mutex);
        }
    }
}

/**
 * \brief              This operation unlinks from the groupTransactions.
 *                     When there are no more links to the groupTransaction it is
 *                     removed from the history.
 *
 * \param _this      : The transactionGroupAdmin this operation operates on.
 *
 * \return           : None
 */
void
v_transactionGroupUnlink(
    v_transactionGroup _this)
{
    os_uint32 value;
    v_transactionGroupAdmin admin;
    v_transactionGroup found;

    if (_this) {
        value = pa_dec32_nv(&_this->historyLinks);

        if (value == 0) {
            admin = v_transactionGroupAdmin(_this->admin);
            assert(v_objectKind(admin->owner) == K_KERNEL);

            TRACE_COHERENT_UPDATE(TRANSACTION_OWNER(_this), "Group(0x%"PA_PRIxADDR") PID(%u) TID(%u) "
                                  "remove from history\n",
                                  (os_address)_this, _this->publisherId, _this->transactionId);

            c_mutexLock(&admin->mutex);

#ifndef NDEBUG
            /* Sanity check, when value equals zero non of the transactions are
             * allowed to have a link remaining.
             */
            {
                struct c_collectionIter it;
                v_transaction txn;
                for (txn = c_collectionIterFirst(_this->transactions, &it); txn; txn = c_collectionIterNext(&it)) {
                    assert(pa_ld32(&txn->historyLinks) == 0);
                }
            }
#endif

            found = c_remove(admin->history, _this, NULL, NULL);
            assert(found == _this);
            c_mutexUnlock(&admin->mutex);
            c_free(found);
        }
    }
}
