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

#include "v__transactionGroup.h"
#include "v__transaction.h"
#include "v__kernel.h"
#include "v__spliced.h"
#include "v_public.h"
#include "v_message.h"
#include "v_instance.h"
#include "v__subscriber.h"
#include "v__dataReader.h"
#include "v__builtin.h"
#include "v__policy.h"
#include "os_abstract.h"
#include "os_report.h"
#include "os_heap.h"
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
        (((o) && (v_transactionGroup(o)->publisher)) ? \
         (v_transactionGroupAdmin(v_transactionPublisher(v_transactionGroup(o)->publisher)->admin)->owner) : \
         NULL)

#define v_transactionPublisherCount(p) \
        (p ? (v_transactionPublisher(p)->writers ? c_count(v_transactionPublisher(p)->writers) : 0) : 0)

/****************************************************************************
 * Private New/Free
 ****************************************************************************/
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
        _this->lastRemovedTransactionId = 0;
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
    v_transaction transaction)
{
    v_transactionGroup group;
    v_kernel kernel = v_objectKernel(transaction);

    assert(transaction != NULL);
    assert(C_TYPECHECK(publisher, v_transactionPublisher));
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
        group->matchCount = 0;
        group->triggered = FALSE;
        group->writers = c_tableNew(v_kernelType(kernel, K_TRANSACTIONGROUPWRITER),
                                    "gid.systemId,gid.localId,gid.serial");

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

/****************************************************************************
 * Private functions
 ****************************************************************************/
static void
v_transactionGroupInsertTransaction (
    v_transactionGroup _this,
    v_transaction transaction)
{
    struct c_collectionIter it;
    v_transaction txn;
    c_bool duplicate = FALSE;

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
    transaction->transactionGroup = _this;

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
        c_append(_this->transactions, transaction);
    }
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

static c_bool
v_transactionGroupAllWritersDiscovered(
    v_transactionGroup _this)
{
    return c_walk(_this->writers, writerDiscovered, NULL);
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

/**
 * This operation determines the completeness of a group coherent transaction.
 *
 * A group is complete if it contains all transactions which are defined by
 * the writers listed in the EOT message of any of the available transactions.
 * For each writer in the list a transaction is expected for each matching reader.
 *
 * This operation will first check if the number of discovered DataWriters equals
 * the number of DataWriters listed in the EOT messages, if these numbers are equal then
 * all DataWriters are discovered and match, in that case completeness is reached as soon
 * as the number of available transactions equal the number of known DataWriters.
 * If the number of known DataWriters differ from the number listed in the EOT messages
 * then some DataWriters don't match or are not discovered yet, in that case this
 * operation will ask the spliced to give more information about the missing DataWriters.
 * The spliced will acknowledge if it has discovered the DataWriter and return the number
 * of matching DataReaders for the DataWriter and then report the status which can either
 * be a match, an unmatch or unknown.
 * In case one or more DataWriters are not discovered yet, this operation cannot determine
 * completeness and will therefore return a unknown status.
 */
static c_bool
v_transactionGroupComplete(
    v_transactionGroup _this)
{
    c_bool complete = FALSE;
    c_bool allDiscovered = FALSE;

    assert(_this);
    assert(C_TYPECHECK(_this, v_transactionGroup));

    if (c_count(_this->transactions) == 0) {
        v_transactionPublisher publisher = v_transactionPublisher(_this->publisher);
        OS_UNUSED_ARG(publisher);
        /* The count attribute is the number of available (received) writer transactions,
         * this number is initially one when the transactionGroup is created.
         * When the value is zero it means that the group is empty (i.e. already flushed)
         * so can be ignored.
         * This can occur when the group is added to the pending list because of an
         * undiscovered writer. In that case the group can be flushed when the writer
         * is discovered while remaining in the pending list until the pending list is
         * flushed.
         */
        TRACE_COHERENT_UPDATE(TRANSACTION_OWNER(_this), "NO TRANSACTIONS: Group(0x%"PA_PRIxADDR") PID(%u) TID(%u) "
             "transaction count(%d) == writer count(%d) EOT count (%d) results in (%d) matches\n",
             (os_address)_this, _this->publisherId, _this->transactionId,
             c_count(_this->transactions), v_transactionPublisherCount(publisher),
             0, _this->matchCount);

        return FALSE;
    }

    allDiscovered = v_transactionGroupAllWritersDiscovered(_this);
    if (allDiscovered &&
        (c_count(_this->transactions) == _this->matchCount)) {
        complete = TRUE;
    }

    TRACE_COHERENT_UPDATE(TRANSACTION_OWNER(_this), "Group(0x%"PA_PRIxADDR") PID(%u) TID(%u) "
                          "allDiscovered (%s), number of transactions (%u), "
                          "matchCount (%u) %s\n",
                          (os_address)_this, _this->publisherId, _this->transactionId,
                          allDiscovered ? "TRUE": "FALSE", c_count(_this->transactions),
                          _this->matchCount, complete?"complete":"incomplete");

    return complete;
}

static v_transactionGroup
v_transactionPublisherRemoveTransaction(
    v_transactionPublisher _this,
    v_transactionGroup group)
{
    assert(_this);
    assert(C_TYPECHECK(_this, v_transactionPublisher));
    assert(group);
    assert(C_TYPECHECK(group, v_transactionGroup));

    return c_remove(_this->transactions, group, NULL, NULL);
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
            /* Need to check it the aborted transaction is still part of the group
             * transaction. */
            for (txn = c_collectionIterFirst(_this->transactions, &it); txn; txn = c_collectionIterNext(&it)) {
                abort = v_transactionIsAborted(txn);
                if (abort == TRUE) {
                    /* One of the transactions is still aborted, abort everything */
                    break;
                }
            }
        } else {
            abort = TRUE;
        }
    }

    for (txn = c_collectionIterFirst(_this->transactions, &it); txn; txn = c_collectionIterNext(&it)) {
        if (abort) {
            v_transactionAbort(txn);
            v_transactionNotifySampleLost(txn, owner);
        }
        v_transactionFlush(txn, owner);
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
        group = v_transactionGroupNew(_this, transaction);
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

/**
 * \brief              This operation removes older transactions from the publisher
 *
 * \param _this      : The v_transactionPublisher this operation operates on.
 * \param transaction : Transactions older then this transaction are removed.
 *
 * \return           : List of removed transactions.
 */
static c_iter
v_transactionPublisherRemoveOlderTransactions(
    v_transactionPublisher _this,
    v_transactionGroup transaction)
{
    c_iter list = NULL;
    v_transactionGroup group;
    struct c_collectionIterD it;
    c_bool old;

    assert(_this);
    assert(C_TYPECHECK(_this, v_transactionPublisher));

    for (group = c_collectionIterDFirst(_this->transactions, &it); group; group = c_collectionIterDNext(&it)) {
        old = FALSE;

        if (transaction->transactionId < _this->lastRemovedTransactionId) {
            if ((group->transactionId >= _this->lastRemovedTransactionId) ||
                (group->transactionId < transaction->transactionId)) {
                old = TRUE;
            }
        } else if ((group->transactionId >= _this->lastRemovedTransactionId) &&
                   (group->transactionId < transaction->transactionId)) {
            old = TRUE;
        }

        if (old) {
            if (v_transactionGroupComplete(group) != TRUE) {
                list = c_iterInsert(list, c_keep(group));
                c_collectionIterDRemove(&it);
                c_free(group);
            }
        }
    }

    return list;
}

static v_topic
getTopicByName(
    v_kernel kernel,
    const c_char *name)
{
    v_topic topic;
    c_iter topics;

    topics = v_resolveTopics(kernel, name);
    assert(c_iterLength(topics) == 1);
    topic = v_topic(c_iterTakeFirst(topics));
    c_iterFree(topics);

    return topic;
}

struct processPublicationInfoHelper {
    v_kernel kernel;
    v_rxoData rxo;
    v_topic topic;
};

static v_result
processPublicationInfo(
    struct v_publicationInfo *info,
    void *arg)
{
    struct processPublicationInfoHelper *helper = (struct processPublicationInfoHelper *)arg;

    helper->rxo = v_kernel_rxoDataFromPublicationInfo(helper->kernel, info);
    helper->topic = getTopicByName(helper->kernel, info->topic_name);

    return V_RESULT_OK;
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
        (strcmp(v_topicName(reader->topic), v_topicName(_this->topic)) == 0) &&
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
    assert(helper->writer->topic);

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
        assert(writer->topic);
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
        } else {
            v_topicQos topicQos;
            topicQos = v_topicGetQos(_this->topic);
            if (topicQos->durability.v.kind != V_DURABILITY_VOLATILE) {
                matches = 1;
            }
            c_free(topicQos);
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
     * When publisherId==0, then it should not change. */
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
     * But do not clean KERNEL owned transactions as these are required for alignment. */
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
    v_transactionGroupRecalculateMatchCount(_this);

    /* Sanity check. */
    if (oldComplete) {
        /* We should still be complete if we not became empty... */
        if (v_transactionGroupHasData(_this)) {
            assert(v_transactionGroupComplete(_this));
        }
    }
}

static c_bool
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
        _this->topic = getTopicByName(kernel, info->topic_name); /* pass reference */
    }
    _this->rxo = v_kernel_rxoDataFromPublicationInfo(kernel, info); /* pass reference */
    _this->discovered = TRUE;

    return TRUE;
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

/**
 * \brief              This operation removes groupTransactions from the history
 *                     when the contained transactions are not referenced by samples
 *                     anymore
 *
 * \param _this      : The transactionGroupAdmin this operation operates on.
 *
 * \return           : None
 */
static void
transactionGroupAdminPurgeHistory(
    v_transactionGroupAdmin _this)
{
    v_transactionGroup group;
    v_transaction txn;
    struct c_collectionIterD it;
    struct c_collectionIter jt;
    c_long cnt;

    if (v_objectKind(ADMIN_OWNER(_this)) == K_KERNEL) {
        for (group = c_collectionIterDFirst(_this->history, &it); group; group = c_collectionIterDNext(&it)) {
            cnt = 0;
            for (txn = c_collectionIterFirst(group->transactions, &jt); txn; txn = c_collectionIterNext(&jt)) {
                /* Determine if the this transactionGroup hold the only reference.
                 */
                cnt += (c_refCount(txn) - 1);
            }
            if (cnt == 0) {
                /* There are no longer samples/registrations referencing this
                 * transactionGroup. It's now safe to remove it.
                 */
                TRACE_COHERENT_UPDATE(ADMIN_OWNER(_this), "TID(%u) GroupTransaction(0x%"PA_PRIxADDR") "
                                      "Removed from history list\n",
                                      group->transactionId, (os_address)group);
                c_collectionIterDRemove(&it);
                c_free(group);
            }
        }
    }
}

/**
 * \brief              This operation adds a groupTransaction to the history list
 *
 * \param _this      : The transactionGroupAdmin this operation operates on.
 * \param group      : The transactionGroup to add to the history list
 *
 * \return           : None
 */
static void
v_transactionGroupAdminAddGroupToHistory(
    v_transactionGroupAdmin _this,
    v_transactionGroup group)
{
    if (v_objectKind(_this->owner) == K_KERNEL) {
        (void)c_append(_this->history, group);
    }
}

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
            if (!c_exists(_this->pending, group)) {
                if (v_transactionGroupHasData(group)) {
                    c_append(_this->pending, group);
                    result = TRUE;
                    TRACE_COHERENT_UPDATE(ADMIN_OWNER(_this),
                                          "Add group TID(%u) (0x%"PA_PRIxADDR") to pending list (size:%u)\n",
                                          group->transactionId,
                                          (os_address)group,
                                          c_count(_this->pending));
                } else {
                    TRACE_COHERENT_UPDATE(ADMIN_OWNER(_this),
                                          "Removing complete but empty group TID(%u) (0x%"PA_PRIxADDR")\n",
                                          group->transactionId,
                                          (os_address)group);
                    c_collectionIterDRemove(&it);
                    c_free(group);
                }
            }
        }
    }
    return result;
}

static void
remove_pending_from_publishers(
    c_list list)
{
    struct c_collectionIter it;
    v_transactionGroup group, found;
    v_transactionPublisher pub;

    for (group = c_collectionIterFirst(list, &it); group; group = c_collectionIterNext(&it)) {
        pub = v_transactionPublisher(group->publisher);
        if (pub) {
            found = v_transactionPublisherRemoveTransaction(pub, group);
            c_free(found);
        }
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
    c_iter oldList = NULL;
    c_iter historyList = NULL;

    assert(_this);
    assert(C_TYPECHECK(_this, v_transactionGroupAdmin));
    assert(C_TYPECHECK(admin, v_transactionAdmin));

    c_mutexLock(&_this->mutex);
    if (c_count(_this->pending) > 0) {
        kernel = v_objectKernel(_this);
        pending = _this->pending;
        _this->pending = c_listNew(v_kernelType(kernel, K_TRANSACTIONGROUP));
        assert(_this->pending);
        remove_pending_from_publishers(pending);

        group = c_read(pending);
        if (group) {
            if (group->publisher) {
                oldList = v_transactionPublisherRemoveOlderTransactions(v_transactionPublisher(group->publisher), group);
            }
            c_free(group);
        }
    }
    if (v_objectKind(ADMIN_OWNER(_this)) == K_KERNEL) {
        historyList = c_iterNew(NULL);
    }
    c_mutexUnlock(&_this->mutex);

    while ((group = c_iterTakeFirst(oldList)) != NULL) {
        group->aborted = TRUE;
        TRACE_COHERENT_UPDATE(ADMIN_OWNER(_this), "PID(%u) Abort incomplete old "
                              "group(0x%"PA_PRIxADDR") TID(%u)\n",
                              group->publisherId, (os_address)group,
                              group->transactionId);
        v_transactionGroupFlush(group, admin, FALSE);
        if (historyList) {
            c_iterAppend(historyList, c_keep(group));
        }
        c_free(group);
    }
    c_iterFree(oldList);
    while (pending && ((group = c_take(pending)) != NULL)) {
        TRACE_COHERENT_UPDATE(ADMIN_OWNER(_this),
                              "PID(%u) Flush group (0x%"PA_PRIxADDR") TID(%u) "
                              "from pending list\n",
                              group->publisherId, (os_address)group,
                              group->transactionId);
        v_transactionGroupFlush(group, admin, TRUE);
        if (historyList) {
            c_iterAppend(historyList, c_keep(group));
        }
        c_free(group);
    }

    if (historyList) {
        c_mutexLock(&_this->mutex);
        while ((group = c_iterTakeFirst(historyList)) != NULL) {
            /* Add all emptied (all elements are flushed and freed, only EOTs still
             * exist) transactionGroups to the history list.
             */
            v_transactionGroupAdminAddGroupToHistory(_this, group);
            c_free(group);
        }
        transactionGroupAdminPurgeHistory(_this);
        c_mutexUnlock(&_this->mutex);
        c_iterFree(historyList);
    }

    c_free(pending);
}

void
v_transactionGroupAdminTrigger(
    v_transactionGroupAdmin _this,
    v_reader owner)
{
    c_iter triggerList = c_iterNew(NULL);
    v_dataReader reader;
    v_transactionGroup group;
    v_transaction transaction;
    struct c_collectionIter it, jt;

    c_mutexLock(&_this->mutex);
    if (c_count(_this->pending) > 0) {
        for (group = c_collectionIterFirst(_this->pending, &it); group; group = c_collectionIterNext(&it)) {
            if (group->triggered == FALSE) {
                for (transaction = c_collectionIterFirst(group->transactions, &jt); transaction; transaction = c_collectionIterNext(&jt)) {
                    v_transactionTriggerList(transaction, triggerList);
                }
                group->triggered = TRUE;
            }
        }
        TRACE_COHERENT_UPDATE(ADMIN_OWNER(_this),
                              "Trigger %u readers data available\n", c_iterLength(triggerList));
    }
    c_mutexUnlock(&_this->mutex);
    while ((reader = c_iterTakeFirst(triggerList)) != NULL) {
        if (reader == v_dataReader(owner)) {
            v_dataReaderTriggerNoLock(reader);
        } else {
            v_dataReaderTrigger(reader);
        }
    }
    c_iterFree(triggerList);
}

static c_bool
publisherReaderUpdate(
    c_object o,
    c_voidp arg)
{
    v_transactionPublisher publisher = v_transactionPublisher(o);

    assert(publisher);
    assert(arg == NULL);
    OS_UNUSED_ARG(arg);

    v_transactionPublisherRecalculateMatchCounts(publisher);
    (void)v_transactionGroupAdminPendCompleteTransactions(v_transactionGroupAdmin(publisher->admin), publisher);

    return TRUE;
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
transactionWriterRemoveReader(
    c_object o,
    c_voidp arg)
{
    v_transactionGroupWriter writer = v_transactionGroupWriter(o);
    v_transactionGroupReader found, reader = v_transactionGroupReader(arg);

    found = c_remove(writer->readers, reader, NULL, NULL);
    if (found) {
        TRACE_COHERENT_UPDATE(PUBLISHER_OWNER(v_transactionPublisher(writer->publisher)),
                              "PID(%u) WID(%u) removed reader(%u)\n",
                              v_transactionPublisher(writer->publisher)->publisherId,
                              writer->gid.localId, found->gid.localId);
        c_free(found);
    }

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

    (void)c_walk(publisher->writers, transactionWriterRemoveReader, groupReader);

    publisherReaderUpdate(o, NULL);

    return TRUE;
}


/****************************************************************************
 * Protected functions
 ****************************************************************************/
v_transactionGroupAdmin
v_transactionGroupAdminNew(
    v_object owner)
{
    v_transactionGroupAdmin _this;
    v_kernel kernel;
    c_base base;

    assert(owner);

    kernel = v_objectKernel(owner);
    _this = v_transactionGroupAdmin(v_objectNew(kernel, K_TRANSACTIONGROUPADMIN));
    if (_this) {
        base = c_getBase(kernel);
        c_mutexInit(base, &_this->mutex);

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
    } else {
        OS_REPORT(OS_ERROR, "v_transactionGroupAdminNew",0,
                  "Failed to allocate v_transactionGroupAdmin object");
        assert(FALSE);
    }
    return _this;
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
    v_spliced spliced;
    struct processPublicationInfoHelper helper;
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

        group = v_transactionGroupNew(NULL, transaction);
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
        spliced = v_kernelGetSpliced(kernel);

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
                if ((thisWriter != writer) &&
                    (writer->discovered == FALSE)) {
                    assert(writer->topic == NULL);
                    assert(writer->rxo == NULL);
                    memset(&helper, 0, sizeof(struct processPublicationInfoHelper));
                    helper.kernel = kernel;

                    /* TGwriter can only be discovered here if the PublicationInfo
                     * was received before the TGwriter was created. This also means
                     * that this discovery will not impact any other open transactions.
                     */
                    if (v_splicedLookupPublicationInfo(spliced,
                                                       writer->gid,
                                                       processPublicationInfo,
                                                       &helper) == V_RESULT_OK) {
                        writer->rxo = helper.rxo; /* pass reference */
                        writer->topic = helper.topic; /* pass reference */
                        writer->discovered = TRUE;

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
            assert(writer->topic == NULL);
            assert(writer->rxo == NULL);

            writer->topic = c_keep(topic);
            writer->rxo = v_kernel_rxoDataFromMessageQos(kernel, v_message(transaction->eot)->qos);
            writer->discovered = TRUE;

            (void)v_transactionGroupWriterMatch(writer, _this);

            if (c_refCount(writer) > 3) {
                /* This writer is also referenced by other group transactions,
                 * update all groups including this one. */
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
                if (!c_exists(_this->pending, group)) {
                    if (v_transactionGroupHasData(group)) {
                        c_append(_this->pending, group);
                        result = TRUE;
                        TRACE_COHERENT_UPDATE(ADMIN_OWNER(_this),
                                              "Add group TID(%u) (0x%"PA_PRIxADDR") to pending list (size:%u)\n",
                                              group->transactionId,
                                              (os_address)group,
                                              c_count(_this->pending));
                    } else {
                        TRACE_COHERENT_UPDATE(ADMIN_OWNER(_this),
                                              "Removing complete but empty group TID(%u) (0x%"PA_PRIxADDR")\n",
                                              group->transactionId,
                                              (os_address)group);
                        found = c_remove(publisher->transactions, group, NULL, NULL);
                        assert(found == group);
                        c_free(found);
                    }
                }
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
    v_transactionGroupReader groupReader, added;
    v_transactionGroup group;
    c_list pending;
    v_kernel kernel;
#ifdef TRACE_COHERENT
    v_topic topic;
#endif

    assert(_this);
    assert(C_TYPECHECK(_this, v_transactionGroupAdmin));
    assert(v_objectKind(ADMIN_OWNER(_this)) == K_SUBSCRIBER);

#ifdef TRACE_COHERENT
    topic = v_dataReaderGetTopic(v_dataReader(reader));
    TRACE_COHERENT_UPDATE(ADMIN_OWNER(_this),
                          "add reader(%d) for topic '%s'\n",
                          v_publicGid(v_public(reader)).localId,
                          v_topicName(topic));
    c_free(topic);
#endif

    kernel = v_objectKernel(_this);
    groupReader = v_transactionGroupReader(v_objectNew(kernel, K_TRANSACTIONGROUPREADER));
    if (groupReader) {
        groupReader->gid = v_publicGid(v_public(reader));
        groupReader->topic = v_dataReaderGetTopic(v_dataReader(reader)); /* pass reference */
        groupReader->rxo = v_kernel_rxoDataFromReaderQos(kernel, reader->qos);

        c_mutexLock(&_this->mutex);
        added = ospl_c_insert(_this->readers, groupReader);
        assert(added == groupReader);
        v_transactionGroupAdminMatchReader(_this, groupReader, FALSE);
        (void)c_walk(_this->publishers, publisherRecalculateMatchCounts, NULL);

        pending = _this->pending;
        _this->pending = c_listNew(v_kernelType(kernel, K_TRANSACTIONGROUP));
        while((group = c_take(pending)) != NULL) {
            if (group->publisher == NULL) {
                /* Group was complete before all its writers were deleted, with
                 * new reader it's no longer possible to determine completeness,
                 * remove it from pending list (group transaction is not in a
                 * publisher list anymore so it is removed).
                 */
            } else if (v_transactionGroupComplete(group)) {
                ospl_c_insert(_this->pending, group);
            } else {
                /* Group has become incomplete again, reset trigger flag so it's
                 * triggered again when complete again.
                 */
                group->triggered = FALSE;
            }
            c_free(group);
        }
        c_free(pending);
        c_mutexUnlock(&_this->mutex);

        c_free(added);
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
    v_transactionGroup group, found;
    C_STRUCT(v_transactionGroupReader) dummy;
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

            /* Remove the reader information from this group.
             * It is most likely that this is already done by v_transactionPublisherRemoveReader,
             * but not always (f.i. when the _this->publisher has been cleared and set to NULL). */
            v_transactionGroupRemoveReader(group, groupReader);

            /* If the group is empty: remove it from the pending list. */
            if (v_transactionGroupHasData(group) == FALSE) {
                c_collectionIterDRemove(&it);

                TRACE_COHERENT_UPDATE(ADMIN_OWNER(_this),
                                      "group(0x%"PA_PRIxADDR") TID(%u) removed as it became empty\n",
                                      (os_address)group, group->transactionId);

                /* Sanity check:
                 * The group should not be available in it's own linked publisher anymore. */
                if (group->publisher) {
                    found = c_find(v_transactionPublisher(group->publisher)->transactions, group);
                    assert(found == NULL);
                    if (found) {
                        c_free(found);
                    }
                }

                c_free(group);
            } else {
                assert(v_transactionGroupComplete(group));
            }
        }

        c_free(groupReader);
    }

    c_mutexUnlock(&_this->mutex);
}

void
v_transactionGroupAdminUpdateReader(
    v_transactionGroupAdmin _this,
    v_reader reader)
{
    v_transactionGroupReader groupReader;
    C_STRUCT(v_transactionGroupReader) dummy;
    v_kernel kernel;

    assert(_this);
    assert(C_TYPECHECK(_this, v_transactionGroupAdmin));
    assert(v_objectKind(ADMIN_OWNER(_this)) == K_SUBSCRIBER);

    TRACE_COHERENT_UPDATE(ADMIN_OWNER(_this),
                          "update reader(%d)\n",
                          v_publicGid(v_public(reader)).localId);

    kernel = v_objectKernel(_this);
    memset(&dummy, 0, C_SIZEOF(v_transactionGroupReader));
    dummy.gid = v_publicGid(v_public(reader));

    c_mutexLock(&_this->mutex);
    groupReader = c_find(_this->readers, &dummy);
    assert(groupReader); /* Reader should exist within administration */
    if (groupReader) {
        c_free(groupReader->rxo);
        groupReader->rxo = v_kernel_rxoDataFromReaderQos(kernel, reader->qos);
        if (v_transactionGroupAdminMatchReader(_this, groupReader, TRUE) == FALSE) {
            /* Reader is updated but it did not result in a match change */
        } else {
            (void)c_walk(_this->publishers, publisherReaderUpdate, NULL);
        }
        c_free(groupReader);
    }

    c_mutexUnlock(&_this->mutex);
}

c_bool
v_transactionGroupAdminNotifyGroupCoherentPublication(
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
            groupWriter = v_transactionPublisherRemoveWriter(publisher, writerGID);
            c_free(groupWriter);

            count = c_count(publisher->writers);
            for (group = c_collectionIterDFirst(publisher->transactions, &idt); group; group = c_collectionIterDNext(&idt)) {
                if (v_transactionGroupComplete(group) == FALSE) {
                    /* Remove incomplete group transactions from publisher */
                    c_collectionIterDRemove(&idt);
                    /* Incomplete transactions should not be in the pending list. */
                    assert(c_remove(_this->pending, group, NULL, NULL) == NULL);
                    c_free(group);
                } else {
                    if (count == 0) {
                        /* Remove backref from group. When the group is complete the pending
                         * list also has a reference.
                         */
                        group->publisher = NULL;
                    }
                }
            }
            if (count == 0) {
                found = c_remove(_this->publishers,publisher,NULL,NULL);
                c_free(found);
            }
        } else {
            groupWriter = v_transactionPublisherFindWriter(publisher, writerGID);
            if (groupWriter) {
                if (info) {
                    if (groupWriter->discovered == FALSE) {
                        /* The writer update function (see a few lines below this),
                         * will indicate that the writer is discovered.
                         * Link this discovered writer into the related groups. */
                        for (group = c_collectionIterFirst(publisher->transactions, &it); group; group = c_collectionIterNext(&it)) {
                            (void)ospl_c_insert(group->writers, groupWriter);
                        }
                    }
                    v_transactionGroupWriterUpdate(groupWriter, info);
                }
                if (v_objectKind(ADMIN_OWNER(_this)) == K_SUBSCRIBER) {
                    if (v_transactionGroupWriterMatch(groupWriter, _this) == TRUE) {
                        v_transactionPublisherRecalculateMatchCounts(publisher);
                    }
                } else {
                    v_transactionPublisherRecalculateMatchCounts(publisher);
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

    for (txn = c_collectionIterFirst(group->transactions, &it); txn && result; txn = c_collectionIterNext(&it)) {
        admin = v_transactionGetAdmin(txn);
        if (v_objectKind(admin->owner) == K_GROUP) {
            if (v_group(admin->owner) == a->vgroup) {
                result = a->action (txn, a->arg);
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

    transactionGroupAdminPurgeHistory(_this);
    (void)c_walk(_this->history, groupWalk, &walkArg);
    (void)c_walk(_this->pending, groupWalk, &walkArg);
    (void)c_walk(_this->publishers, publisherWalk, &walkArg);
    c_mutexUnlock(&_this->mutex);
}

/*
 * Call should be protected by v_subscriberLockAccess
 */
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


/**
 * \brief              This operation removes groupTransactions from the history
 *                     when the contained transactions are not referenced by samples
 *                     anymore
 *
 * \param _this      : The transactionGroupAdmin this operation operates on.
 *
 * \return           : None
 */
void
v_transactionGroupAdminPurgeHistory(
    v_transactionGroupAdmin _this)
{
    c_mutexLock(&_this->mutex);
    transactionGroupAdminPurgeHistory(_this);
    c_mutexUnlock(&_this->mutex);
}

