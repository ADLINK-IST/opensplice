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
#include "v__group.h"
#include "v__groupInstance.h"
#include "v__dataReaderEntry.h"
#include "v__dataReaderInstance.h"
#include "v__lease.h"
#include "v__leaseManager.h"
#include "v__subscriber.h"
#include "v__reader.h"
#include "v__observer.h"
#include "v_message.h"
#include "v_public.h"
#include "v_topic.h"
#include "v_messageQos.h"
#include "c_stringSupport.h"
#include "os_abstract.h"
#include "os_report.h"
#include "os_heap.h"

/* Pick a block size that is small enough to fit in the 'small slab'categorie.
 * Be aware that additional headers may be inserted by the c_array, by the
 * database and by the memory manager.
 */
#define V_TRANSACTION_BLOCKSIZE 50

#if 0
#define TRACE_COHERENT_UPDATE(fmt, ...) \
do { \
    printf(fmt, __VA_ARGS__); \
    fflush(stdout); \
} while (0)
#else
#define TRACE_COHERENT_UPDATE(...)
#endif


#define ADMIN_OWNER(o) v_transactionAdmin(o)->owner
#define ADMIN_SCOPE(o) v_objectKindImage(ADMIN_OWNER(o))

#define WRITER_OWNER(o) ADMIN_OWNER(v_transactionWriter(o)->admin)
#define WRITER_SCOPE(o) v_objectKindImage(WRITER_OWNER(o))

#define TRANSACTION_OWNER(o) ADMIN_OWNER(v_transaction(o)->admin)
#define TRANSACTION_SCOPE(o) v_objectKindImage(TRANSACTION_OWNER(o))

/**
 * The constructor of the v_transactionAdmin class.
 *
 * param owner      : The owner of the admin object which is either the DataReader's entry
 *                    or the kernel group which acts as the durability reader for non
 *                    volatile topics.
 * param groupAdmin : reference to optional groupAdmin, only required for Group Coherent
 *                    scope. For group coherence this admin will pass complete transactions
 *                    to groupAdmin instead of flushing the data to the DataReader's history,
 *                    in that case the groupAdmin will take over flushing when the group
 *                    becomes complete.
 */
v_transactionAdmin
v_transactionAdminNew(
    v_object owner,
    v_transactionGroupAdmin groupAdmin,
    v_topic topic)
{
    v_transactionAdmin _this;
    v_kernel kernel;

    assert(owner);
    assert(C_TYPECHECK(owner, v_object));
    assert(C_TYPECHECK(groupAdmin, v_transactionGroupAdmin));

    kernel = v_objectKernel(owner);

    _this = v_transactionAdmin(v_objectNew(kernel, K_TRANSACTIONADMIN));
    if (_this == NULL) {
        OS_REPORT(OS_ERROR, "v_transactionAdminNew", 0,
                  "Failed to allocate v_transactionAdmin object");
        goto err_admin_alloc;
    }

    _this->groupAdmin = groupAdmin;
    _this->owner = owner;
    _this->topic = c_keep(topic);
    _this->writers = c_tableNew(v_kernelType(kernel, K_TRANSACTIONWRITER),
                                "writerGID.systemId, writerGID.localId, writerGID.serial");
    if (_this->writers == NULL) {
        OS_REPORT(OS_ERROR, "v_transactionAdminNew", 0,
                  "Failed to allocate v_transactionAdmin writers list");
        goto err_writers_alloc;
    }

    if (v_objectKind(owner) == K_GROUP) {
        /* Only group owned transactionAdmins need keep to history as this
         * is a list of EOTs (wrapped in v_transactions) which are required
         * for alignment.
         */
        _this->history = c_listNew(v_kernelType(kernel, K_TRANSACTION));
        if (_this->history == NULL) {
            OS_REPORT(OS_ERROR, "v_transactionAdminNew", 0,
                      "Failed to allocate v_transactionAdmin history list");
            goto err_history_alloc;
        }
    } else {
        _this->history = NULL;
    }

    TRACE_COHERENT_UPDATE("v_transactionAdminNew(%s): "
                          "groupAdmin(0x%"PA_PRIxADDR") topic(%s) => admin(0x%"PA_PRIxADDR")\n",
                          v_objectKindImage(owner), (os_address)groupAdmin,
                          v_topicName(topic), (os_address)_this);

    return _this;

err_history_alloc:
err_writers_alloc:
    c_free(_this);
err_admin_alloc:
    assert(FALSE);
    return NULL;
}

/**
 * The constructor of the v_transactionWriter class.
 *
 * Object if this class represents a discovered matching writer.
 * Objects of this class are created for each discovered matching and added to
 * the v_transactionAdmin.
 * This class implements a DataWriter specific container in which all active incomplete
 * transactions are maintained.
 */

static v_transactionWriter
v_transactionWriterNew(
    v_transactionAdmin admin,
    v_gid writerGID)
{
    v_transactionWriter _this;
    v_kernel kernel;

    assert(admin);
    assert(C_TYPECHECK(admin, v_transactionAdmin));

    kernel = v_objectKernel(admin);
    _this = v_transactionWriter(v_objectNew(kernel, K_TRANSACTIONWRITER));
    if (_this) {
        TRACE_COHERENT_UPDATE("v_transactionWriterNew(%s): "
                              "admin(0x%"PA_PRIxADDR") WGID(%d) => 0x%"PA_PRIxADDR"\n",
                              ADMIN_SCOPE(admin), (os_address)admin, writerGID.localId,
                              (os_address)_this);
        _this->writerGID = writerGID;
        _this->publisherId = 0;
        _this->admin = admin;
        if (v_objectKind(ADMIN_OWNER(admin)) == K_GROUP) {
            _this->matchCount = 1;
        } else {
            _this->matchCount = 0;
        }
        _this->transactions = c_listNew(v_kernelType(kernel, K_TRANSACTION));
    }
    return _this;
}

/**
 * The constructor of the v_transaction class.
 *
 * This class represents a DataWriter specific transaction in which all messages belonging
 * to the transaction are maintained.
 * Objects of this class are created for each transaction and is uniquely identified by its
 * transactionId. Transaction objects are maintained by the related transactionWriter object
 * for as long the transaction is incomplete, as soon as it becomes complete it will be
 * either flushed to the DataReaders history (for Topic and Instance coherence) or moved
 * to the transactionGroupAdmin in case of group coherence scope.
 *
 * param writer        : The transactionWriter to which this transaction belongs.
 * param transactionId : The transaction's unique identifier.
 */

static v_transaction
v_transactionNew(
    v_transactionWriter writer,
    c_ulong transactionId)
{
    v_transaction transaction;
    v_kernel kernel = v_objectKernel(writer);

    assert(writer);
    assert(C_TYPECHECK(writer, v_transactionWriter));

    transaction = v_transaction(v_objectNew(kernel, K_TRANSACTION));
    if (transaction) {
        transaction->writer = writer;
        transaction->admin = writer->admin;
        transaction->count = 0;
        transaction->eotCount = 0;
        transaction->size = 0;
        transaction->elementZero = FALSE;
        transaction->aborted = FALSE;
        transaction->sampleLostNotified = FALSE;
        transaction->transactionId = transactionId;
        transaction->isMarked = FALSE;
        transaction->transactionGroup = NULL;
        transaction->elements = c_arrayNew(v_kernelType(kernel, K_TRANSACTIONELEMENT),
                                           V_TRANSACTION_BLOCKSIZE);
        if (transaction->elements == NULL) {
            OS_REPORT(OS_ERROR, "v_transactionNew", OS_ERROR,
                      "Failed to allocate v_transaction->elements object");
            c_free(transaction);
            transaction = NULL;
            OS_REPORT(OS_ERROR, "v_transactionNew", OS_ERROR,
                      "Failed to allocate v_transaction->elements array object");
            assert(FALSE);
        }
        TRACE_COHERENT_UPDATE("v_transactionNew(%s) Writer(0x%"PA_PRIxADDR") WGID(%d) TID(%d) => 0x%"PA_PRIxADDR"\n",
                              WRITER_SCOPE(writer), (os_address)writer, 
                              writer->writerGID.localId, transactionId,
                              (os_address)transaction);
    } else {
        OS_REPORT(OS_ERROR, "v_transactionNew", OS_ERROR,
                  "Failed to allocate v_transaction object");
        assert(FALSE);
    }
    return transaction;
}

/**
 * This function returns a reference to the message+instance element from the
 * transaction's element list based on the given index.
 * The element list is implemented as an array, this function will reallocate
 * the array if the index exceeds the array size.
 * Reallocation is performed once every V_TRANSACTION_BLOCKSIZE elements to minimize
 * performance impact.
 */

static v_transactionElement
getTransactionElement(
    v_transaction transaction,
    c_ulong index)
{
    c_type elementType = NULL;
    c_array replacementArray = NULL;
    c_ulong currentSize;
    c_ulong allocSize;

    assert(transaction);
    assert(C_TYPECHECK(transaction, v_transaction));

    if (transaction->elements == NULL) {
        return NULL;
    }

    currentSize = c_arraySize(transaction->elements);
    if (index >= currentSize) {
        elementType = v_kernelType(v_objectKernel(transaction), K_TRANSACTIONELEMENT);
        allocSize = ((index / V_TRANSACTION_BLOCKSIZE)+1)*V_TRANSACTION_BLOCKSIZE;
        replacementArray = c_arrayNew(elementType, allocSize);
        if (replacementArray != NULL) {
            memcpy(replacementArray, transaction->elements,
                   currentSize * sizeof(v_transactionElement));
            memset(transaction->elements, 0, currentSize * sizeof(v_transactionElement));
            c_free(transaction->elements);
            transaction->elements = replacementArray;
        } else {
            /* Fatal: Out of resources */
            OS_REPORT(OS_ERROR, "v_transaction::getTransactionElement", OS_ERROR,
                      "Failed to reallocate v_transaction->elements object");
            return NULL;
        }
    }
    if (transaction->elements[index] == NULL) {
        transaction->elements[index] = c_new(v_kernelType(v_objectKernel(transaction),
                                                          K_TRANSACTIONELEMENT));
    }
    return transaction->elements[index];
}

void
v_transactionAbort(
    v_transaction _this)
{
    c_ulong count, i;
    v_transactionElement e;
    v_kind kind;

    assert(_this != NULL);
    assert(C_TYPECHECK(_this, v_transaction));

    kind = v_objectKind(TRANSACTION_OWNER(_this));

    if (!_this->aborted) {
        TRACE_COHERENT_UPDATE("v_transactionAbort(%s) Transaction(0x%"PA_PRIxADDR"): TID(%d) Start\n",
                              TRANSACTION_SCOPE(_this),
                              (os_address)_this,
                              _this->transactionId);
        _this->aborted = TRUE;
        /* Release previously claimed resources. */
        count = c_arraySize(_this->elements);
        for (i=0; i<count; i++) {
            e = _this->elements[i];
            if (e && e->message) {
                if ((v_messageStateTest(e->message, L_REGISTER)) ||
                    (v_messageStateTest(e->message, L_UNREGISTER)) ||
                    (v_messageStateTest(e->message, L_DISPOSED | L_AUTO))) {
                    assert(!v_messageStateTest(e->message, L_WRITE));
                    /* Can never be lost. */
                    continue;
                } else if (v_messageStateTest(e->message, L_WRITE)) {
                    /* resources are only claimed for messages having set the L_WRITE flag */
                    switch (kind) {
                    case K_DATAREADER:
                        v_dataReaderInstanceReleaseResource(v_dataReaderInstance(e->instance));
                    break;
                    case K_GROUP:
                        v_groupInstanceReleaseResource(v_groupInstance(e->instance));
                    break;
                    default:
                        assert(0);
                    break;
                    }
                }
                c_free(e->message);
                c_free(e->instance);
                e->message = NULL;
                e->instance = NULL;
            }
        }
    }
}

c_bool
v_transactionIsAborted(
    v_transaction _this)
{
    assert(_this != NULL);
    assert(C_TYPECHECK(_this, v_transaction));

    return _this->aborted;
}

/**
 * \brief              This operation inserts a received coherent message into the
 *                     transaction.
 *
 * \param _this      : The transaction this operation operates on.
 * \param message    : The coherent message that will be inserted.
 * \param instance   : The instance in which the message must be inserted as soon as
 *                     the transaction becomes complete. If instance == NULL then
 *                     messages are not inserted but only registered to verify
 *                     completeness of the transaction.
 *
 * \return           : The write status, whether it succeeded or not and for what reason.
 */

static v_message
transactionAdminCreateSingleMessageEOT(
    v_transactionAdmin _this,
    v_message tmpl)
{
    v_message message;

    message = c_new(v_kernelType(v_objectKernel(_this), K_MESSAGEEOT));
    if (message) {
        V_MESSAGE_INIT(message);
        v_stateSet(v_nodeState(message), L_TRANSACTION | L_ENDOFTRANSACTION);
        message->allocTime = tmpl->allocTime;
        message->writeTime = tmpl->writeTime;
        message->writerGID = tmpl->writerGID;
        v_gidSetNil(message->writerInstanceGID);
        message->qos = c_keep(tmpl->qos);
        message->sequenceNumber = tmpl->sequenceNumber + 1;
        message->transactionId = tmpl->transactionId;
        v_messageEOT(message)->publisherId = 0;
        v_messageEOT(message)->transactionId = 0;
        if (v_messageQos_presentationKind(tmpl->qos) == V_PRESENTATION_GROUP) {
            struct v_tid *tid;
            v_messageEOT(message)->tidList = c_arrayNew(v_kernelType(v_objectKernel(_this), K_TID), 1);
            tid = (struct v_tid *)v_messageEOT(message)->tidList;
            tid->wgid = tmpl->writerGID;
            tid->seqnr = tmpl->transactionId;
        } else {
            v_messageEOT(message)->tidList = NULL;
        }
    } else {
        OS_REPORT(OS_FATAL,OS_FUNCTION,V_RESULT_INTERNAL_ERROR,
                  "Failed to allocate message.");
        assert(FALSE);
    }
    return message;
}

static void
storeMessageEOT(
    v_transaction _this,
    c_ulong index,
    v_message message)
{
    v_transactionElement targetElement;
    c_bool storeEOT = FALSE;

    if ((_this->eotCount > 0) &&
        (v_objectKind(v_object(TRANSACTION_OWNER(_this))) == K_GROUP)) {
        assert(_this->eot);
        if ((!v_messageStateTest(message, L_TRANSACTION)) &&
            (v_messageStateTest(_this->eot, L_TRANSACTION))) {
            /* EOT messages without the L_TRANSACTION flag indicate completeness.
             * These messages are always more important than the messages with
             * the flag. This can only happen for K_GROUP owned transactions as
             * the group forwards the EOT to the readers when the group has become
             * complete.
             */
            storeEOT = TRUE;
        }
    } else {
        assert(!_this->eot);
        storeEOT = TRUE;
    }

    if (storeEOT) {
        if (v_objectKind(v_object(TRANSACTION_OWNER(_this))) == K_GROUP) {
            _this->eotCount = 1;
        } else {
            _this->eotCount++;
        }

        targetElement = getTransactionElement(_this, index);
        if (targetElement) {
            c_free(targetElement->message);
            c_free(targetElement->instance);
            targetElement->instance = NULL;
            targetElement->message = c_keep(message);
        }
        _this->size = (c_long)index;
        _this->eot = c_keep(message);
    }
}

static v_writeResult
v_transactionInsertMessage (
    v_transaction _this,
    v_instance instance,
    v_message message,
    c_bool abort)
{
    v_transactionElement targetElement;
    v_writeResult result = V_WRITE_SUCCESS;
    c_ulong index;

    assert(_this);
    assert(C_TYPECHECK(_this, v_transaction));
    assert(C_TYPECHECK(instance, v_instance));
    assert(message);
    assert(C_TYPECHECK(message, v_message));
    assert(_this->elements);

    TRACE_COHERENT_UPDATE("v_transactionInsertMessage(%s): Transaction(0x%"PA_PRIxADDR") TID(%d) WGID(%d) "
                          "msg(0x%"PA_PRIxADDR") instance(0x%"PA_PRIxADDR") SEQ(%u)%s\n",
                          TRANSACTION_SCOPE(_this), (os_address)_this,
                          message->transactionId, message->writerGID.localId,
                          (os_address)message, (os_address)instance,
                          message->sequenceNumber,
                          v_messageStateTest(message, L_ENDOFTRANSACTION)?" EOT":"");

    if ((message->sequenceNumber == C_MAX_ULONG) &&
        (message->transactionId == 0)) {
        index = (c_ulong)_this->count;
        _this->isMarked = TRUE;
    } else {
        assert(_this->isMarked == FALSE);
        index = (message->sequenceNumber >= message->transactionId) ?
                message->sequenceNumber - message->transactionId :
                C_MAX_ULONG - message->transactionId + message->sequenceNumber + 1;
    }

    if (index == 0) {
        _this->elementZero = TRUE;
    }

    /* If this is the End Of Transaction Marker, then calculate
     * the amount of expected samples and subtract it from the
     * amount of samples already received. If the end result
     * is 0, then all expected samples must have arrived and
     * the transaction can be considered 'complete'.
     */
    if (v_messageStateTest(message, L_ENDOFTRANSACTION)) {
        storeMessageEOT(_this, index, message);
        TRACE_COHERENT_UPDATE("v_transactionInsertMessage(%s) TID(%d) END OF TRANSACTION ok (cnt:%u)\n",
                              TRANSACTION_SCOPE(_this), message->transactionId, _this->eotCount);
        return V_WRITE_SUCCESS_NOT_STORED;
    }

    /* Handle message */
    _this->count++; // increase message count
    if ((v_messageStateTest(message, L_REGISTER)) ||
        (v_messageStateTest(message, L_UNREGISTER)) ||
        (v_messageStateTest(message, L_DISPOSED | L_AUTO))) {
        /* Can never be lost, should always be added and don't require a claim. */
    } else if (_this->aborted || abort) {
        /* If this transaction was aborted then this message is lost. */
        TRACE_COHERENT_UPDATE("v_transactionInsertMessage(%s) TID(%d) SAMPLE_LOST\n",
                              TRANSACTION_SCOPE(_this), message->transactionId);
        result = V_WRITE_SUCCESS_NOT_STORED;
    } else if (instance) {
        /* An instance is specified so now claim the required resources. */
        v_dataReaderResult rr;
        switch(v_objectKind(v_object(instance))) {
        case K_DATAREADERINSTANCE:
            rr = v_dataReaderInstanceClaimResource(v_dataReaderInstance(instance), message, V_CONTEXT_GROUPWRITE);
            switch (rr) {
            case V_DATAREADER_INSTANCE_FULL:
                result = V_WRITE_SUCCESS_NOT_STORED;
            break;
            case V_DATAREADER_MAX_SAMPLES:
                if (v_dataReaderHistoryCount(v_dataReaderInstanceReader(instance)) == 0) { /* reader can't free resources. */
                    result = V_WRITE_SUCCESS_NOT_STORED;
                } else {
                    result = V_WRITE_REJECTED;
                    _this->count--; // revert previous increase
                }
            break;
            default:
                assert(rr == V_DATAREADER_INSERTED);
            break;
            }
        break;
        case K_GROUPINSTANCE:
            if (!v_groupInstanceClaimResource(v_groupInstance(instance), message)) {
                result = V_WRITE_SUCCESS_NOT_STORED;
            }
        break;
        default:
            result = V_WRITE_UNDEFINED;
            (void)result;
            assert(0);
        break;
        }
    }
    /* If SUCCESS then message is accepted and can be stored in the element list. */
    if (result == V_WRITE_SUCCESS) {
        targetElement = getTransactionElement(_this, index);
        if (targetElement) {
            if (targetElement->message) {
                _this->count--;
            }
            c_free(targetElement->message);
            c_free(targetElement->instance);
            if (instance) {
                targetElement->message = c_keep(message);
                targetElement->instance = c_keep(instance);
            } else {
                targetElement->message = NULL;
                targetElement->instance = NULL;
            }
        }
    }
    /* If message is NOT_STORED then abort the transaction. */
    if (result == V_WRITE_SUCCESS_NOT_STORED) {
        v_transactionAbort(_this);
        result = V_WRITE_SUCCESS;
    }
    /* For Kernel Groups create and insert an additional EOT for single message transactions */
    if ((result == V_WRITE_SUCCESS) &&
        (v_objectKind(v_object(TRANSACTION_OWNER(_this))) == K_GROUP) &&
        (v_message_isSingleTransaction(message)))
    {
        v_message eotMsg;
        TRACE_COHERENT_UPDATE("v_transactionInsertMessage(%s) TID(%d) "
                              "single message transaction generate EOT\n",
                              TRANSACTION_SCOPE(_this), message->transactionId);
        eotMsg = transactionAdminCreateSingleMessageEOT(_this->admin, message);
        index = (eotMsg->sequenceNumber >= eotMsg->transactionId) ?
                eotMsg->sequenceNumber - eotMsg->transactionId :
                C_MAX_ULONG - eotMsg->transactionId + eotMsg->sequenceNumber + 1;
        storeMessageEOT(_this, index, eotMsg);
        c_free(eotMsg);
    }
    return result;
}

/**
 * \brief              This operation will lookup the transactionWriter object for
 *                     the given DataWriter GID, if the object doesn't exist it will
 *                     first create one.
 *
 * \param _this      : The transactionAdmin this operation operates on.
 * \param writerGID  : The DataWriter's GID to look for.
 *
 * \return           : The found or newly created transactionWriter object.
 */

static v_transactionWriter
v_transactionAdminLookupWriter(
    v_transactionAdmin _this,
    v_gid writerGID)
{
    C_STRUCT(v_transactionWriter) template;
    v_transactionWriter writer;
    v_transactionWriter inserted = NULL;

    assert(_this);
    assert(C_TYPECHECK(_this,v_transactionAdmin));

    template.writerGID = writerGID;
    writer = c_find(_this->writers, &template);
    if (writer == NULL) {
        writer = v_transactionWriterNew(_this, writerGID);
        inserted = ospl_c_insert(_this->writers, writer);
        OS_UNUSED_ARG(inserted);
    }
    TRACE_COHERENT_UPDATE("v_transactionAdminLookupWriter(%s) Admin(0x%"PA_PRIxADDR") WGID(%d) found Writer(0x%"PA_PRIxADDR") in %d writers%s\n",
                          ADMIN_SCOPE(_this), (os_address)_this, writerGID.localId, (os_address)writer, c_count(_this->writers),
                          inserted?" NEW":"");
    return writer;
}

/**
 * \brief              This operation will remove the transactionWriter object specified
 *                     by the given DataWriter GID from the transactionAdmin.
 *
 * \param _this      : The transactionAdmin this operation operates on.
 * \param writerGID  : The DataWriter's GID to look for.
 *
 * \return           : The removed transactionWriter object.
 */

static v_transactionWriter
v_transactionAdminRemoveWriter(
    v_transactionAdmin _this,
    v_gid writerGID)
{
    v_transactionWriter found;
    C_STRUCT(v_transactionWriter) template;

    assert(_this);
    assert(C_TYPECHECK(_this,v_transactionAdmin));

    template.writerGID = writerGID;
    found = c_remove(_this->writers, &template, NULL, NULL);
    TRACE_COHERENT_UPDATE("v_transactionAdminRemoveWriter(%s) Admin(0x%"PA_PRIxADDR") WGID(%d) found Writer(0x%"PA_PRIxADDR") in %d writers\n",
                          ADMIN_SCOPE(_this), (os_address)_this, writerGID.localId, (os_address)found, c_count(_this->writers));
    return found;
}

/**
 * \brief              This operation will flush the complete transaction or forward
 *                     it to the transactionGroupAdmin if it's a group coherent transaction.
 *
 * \param _this      : The transactionAdmin this operation operates on.
 * \param transaction : Transaction to flush
 *
 * \return           : TRUE if complete or FALSE if incomplete.
 */
static c_bool
v_transactionAdminFlush(
    v_transactionAdmin _this,
    v_transaction transaction)
{
    c_bool result = TRUE;

    assert(_this);
    assert(transaction);
    assert(C_TYPECHECK(_this,v_transactionAdmin));
    assert(C_TYPECHECK(transaction,v_transaction));

    if (transaction->aborted) {
        /* Samples will not be delivered because the transaction is aborted, notify
         * the number of lost samples.
         */
        v_transactionNotifySampleLost(transaction, _this);
    }
    if (v_objectKind(v_object(_this->owner)) == K_GROUP) {
        v__groupDataReaderEntriesWriteEOTNoLock(_this->owner, v_message(transaction->eot));
    }
    if (_this->groupAdmin &&
        (v_messageQos_presentationKind(v_message(transaction->eot)->qos) == V_PRESENTATION_GROUP)) {
        v_topicQos topicQos = NULL;
        if (v_objectKind(v_object(_this->owner)) == K_GROUP) {
            topicQos = v_topicGetQos(_this->topic);
            assert(topicQos);
            result = FALSE;
        }
        if ((topicQos != NULL) && (topicQos->durability.v.kind == V_DURABILITY_VOLATILE)) {
            /* Do not insert transaction in TGA when group owned and volatile.
             * Do abort and flush the transaction so that unregisters are processed.
             */
            v_transactionAbort(transaction);
            v_transactionFlush(transaction, _this);
        } else {
            result = v_transactionGroupAdminInsertTransaction(_this->groupAdmin, transaction, _this->topic);
        }
        c_free(topicQos);
    } else {
        v_transactionFlush(transaction, _this);
        if (v_objectKind(v_object(_this->owner)) == K_GROUP) {
            assert(_this->history != NULL);
            (void)c_append(_this->history, transaction);

            v_transactionAdminPurgeHistory(_this);
        }
    }
    return result;
}

/**
 * This operation returns the completeness of a transaction.
 *
 * param _this   : The transaction this operation operates on.
 * return        : TRUE if complete or FALSE if incomplete.
 *
 * The count represents the number of messages for as long the transaction is
 * incomplete, as soon as a transaction becomes complete the count is set to
 * zero to indicate completeness.
 */
static c_bool
v_transactionComplete(
    v_transaction _this)
{
    c_ulong partitions = 0;
    c_bool result = FALSE;
    v_object owner;

    assert(_this);
    assert(C_TYPECHECK(_this,v_transaction));

    owner = TRANSACTION_OWNER(_this);
    if (_this->eot) {
        if (v_objectKind(owner) == K_DATAREADER) {
            if ((_this->elementZero) ||
                (!v_messageStateTest(_this->eot, L_TRANSACTION))) {
                partitions = v_transactionWriter(_this->writer)->matchCount;
                if (partitions == 0) {
                    if ((v_readerSubscriber(v_reader(owner))) &&
                        (v_subscriberPartitionCount(v_readerSubscriber(v_reader(owner))) == 1)) {
                        partitions = 1; /* no need to wait for discovery if subscriber has only one partition. */
                    }
                }
                result = (_this->eotCount == (c_long)partitions);
            }
        } else {
            /* If L_TRANSACTION is not set on an EOT message then its complete */
            if (!v_messageStateTest(_this->eot, L_TRANSACTION)) {
                assert(v_messageStateTest(_this->eot, L_ENDOFTRANSACTION));
                result = TRUE;
            } else {
                result = (_this->count == _this->size);
            }
        }
    }

    TRACE_COHERENT_UPDATE("v_transactionComplete(%s) Transaction(0x%"PA_PRIxADDR") TID(%u) "
                          "is %s (count = %d/%d, eotCount = %d/%d) eot(0x%"PA_PRIxADDR") %s\n",
                          TRANSACTION_SCOPE(_this), (os_address)_this, _this->transactionId,
                          (result ? "Complete" : "Incomplete"), _this->count, _this->size, _this->eotCount,
                          (v_objectKind(owner) == K_DATAREADER) ? (c_long)partitions : 1,
                          (os_address)_this->eot, v_topicName(v_transactionAdmin(_this->admin)->topic));
    return result;
}

static c_bool
compareTransactionId(
    c_object o,
    c_voidp arg)
{
    v_transaction transaction = v_transaction(o);
    v_transaction template = (v_transaction)arg; /* simple cast without type checking because template is no necessarily a database object. */

    return (transaction->transactionId == template->transactionId);
}

/**
 * \brief              This operation will remove the transaction object specified
 *                     by the given transactionId from the transactionWriter.
 *
 * \param _this         : The transactionWriter this operation operates on.
 * \param transactionId : The transaction id to look for.
 *
 * \return              : The removed transaction object.
 */

static v_transaction
v_transactionWriterRemoveTransaction(
    v_transactionWriter _this,
    c_ulong transactionId)
{
    C_STRUCT(v_transaction) template;

    assert(_this);
    assert(C_TYPECHECK(_this,v_transactionWriter));

    template.transactionId = transactionId;
    return c_listTemplateRemove(_this->transactions, compareTransactionId, &template);
}

static c_bool
getCompleteTransactions(
    c_object o,
    c_voidp arg)
{
    v_transaction trs = v_transaction(o);
    c_iter *list = (c_iter *)arg;

    if (v_transactionComplete(trs)) {
        *list = c_iterInsert(*list, trs);
    }
    return TRUE;
}

static c_iter
v_transactionWriterGetCompleteTransactions(
    v_transactionWriter _this)
{
    c_iter list = NULL;
    (void)c_walk(_this->transactions, getCompleteTransactions, &list);
    return list;
}

/**
 * \brief              This operation notifies the transactionAdmin about discovered
 *                     matching coherent DataWriters.
 *
 * \param _this       : The v_transactionAdmin this operation operates on.
 * \param writerGID   : The global identifier of the discovered DataWriter.
 * \param dispose     : A boolean specifying whether we discovered the appearance or
 *                      disappearance of a DataWriter.
 * \param info        : Publication information of a DataWriter.
 * \param isImplicit  : A boolean specifying if this notification is implicitly
 *                      generated.
 *
 * \return            : TRUE when at least one transaction has become complete as a result
 *                      of this notify.
 */
c_bool
v_transactionAdminNotifyPublication(
    v_transactionAdmin _this,
    v_gid writerGID,
    c_bool dispose,
    struct v_publicationInfo *info,
    c_bool isImplicit)
{
    c_bool result = FALSE;
    v_transactionWriter writer;
    v_transaction txn;

    assert(_this);
    assert(C_TYPECHECK(_this,v_transactionAdmin));

    TRACE_COHERENT_UPDATE("v_transactionAdminNotifyPublication(%s): WGID(%d)\n",
                          ADMIN_SCOPE(_this), writerGID.localId);
    if (dispose) {
        writer = v_transactionAdminRemoveWriter(_this, writerGID);
        if (writer) {
            if (_this->groupAdmin) {
                switch (v_objectKind(_this->owner)) {
                case K_DATAREADER:
                    if (v_readerSubscriber(v_reader(_this->owner)) == NULL) {
                        break;
                    }
                    /* Fall through */
                default:
                    result = v_transactionGroupAdminNotifyGroupCoherentPublication(_this->groupAdmin, writer, dispose, info);
                }
            }
            while ((txn = c_take(writer->transactions)) != NULL) {
                v_transactionAbort(txn);
                if (isImplicit) {
                    /* Only when the transaction is aborted because of a implicit
                     * dispose send out a sampleLost notification.
                     * When the dispose is none implicit the writer was deleted
                     * during a transaction (by the federation that created the
                     * transaction) so then it's expected that the samples are
                     * not delivered.
                     */
                    v_transactionNotifySampleLost(txn, _this);
                }
                v_transactionFlush(txn, _this);
                c_free(txn);
            }
            v_transactionAdminPurgeHistory(_this);
        }
    } else {
        writer = v_transactionAdminLookupWriter(_this, writerGID);
        if (writer != NULL) {
            if (v_objectKind(_this->owner) != K_GROUP) {
                c_bool proceed = TRUE;
                c_ulong i, wpc, matchCount;
                os_char *head, *tail;
                os_size_t length;
                matchCount = writer->matchCount;
                wpc = c_arraySize(info->partition.name);
                head = v_reader(_this->owner)->subQos->partition.v;
                tail = c_skipUntil(head,",");
                length = (os_size_t) (tail - head);
                while (proceed) {
                    for (i=0; i<wpc; i++) {
                        if (length == 0 && strlen(info->partition.name[i]) == 0) {
                            writer->matchCount++;
                        } else {
                            c_value str1, str2;
                            c_value val;
                            c_char *partitionElement = os_malloc(length + 1);

                            partitionElement[0] = '\0';
                            strncat(partitionElement, head, length);
                            str1 = c_stringValue(info->partition.name[i]);
                            str2 = c_stringValue(partitionElement);
                            val = c_valueStringMatch(str1, str2);
                            if (!val.is.Boolean) {
                                val = c_valueStringMatch(str2, str1);
                            }
                            if (val.is.Boolean) {
                                writer->matchCount++;
                            }
                            os_free(partitionElement);
                        }
                    }
                    if (*tail == ',') tail++;
                    if (*tail == 0) {
                        proceed = FALSE;
                    } else {
                        head = tail;
                        tail = c_skipUntil(head,",");
                    }
                }
                /* new match(es) found so check if we can determine if any of the
                 * transactions are complete and if we can flush them.
                 */
                if (matchCount < writer->matchCount) {
                    v_transaction transaction;
                    c_iter list = NULL;
                    list = v_transactionWriterGetCompleteTransactions(writer);
                    while ((transaction = c_iterTakeFirst(list))) {
                        assert(transaction->eot);
                        v_transactionWriterRemoveTransaction(writer, v_message(transaction->eot)->transactionId);
                        if (transaction->eot->publisherId != 0) {
                            writer->publisherId = transaction->eot->publisherId;
                        }
                        (void)v_transactionAdminFlush(_this, transaction);
                        c_free(transaction);
                    }
                    c_iterFree(list);
                }
            }
            if (_this->groupAdmin) {
                switch (v_objectKind(_this->owner)) {
                case K_DATAREADER:
                    if (v_readerSubscriber(v_reader(_this->owner)) == NULL) {
                        break;
                    }
                    /* Fall through */
                default:
                    result = v_transactionGroupAdminNotifyGroupCoherentPublication(_this->groupAdmin, writer, dispose, info);
                }
            }
        }
    }
    if (writer) {
        c_free(writer);
    }

    return result;
}

/**
 * \brief              This operation will lookup the transaction object for
 *                     the given transactionId, if the object doesn't exist it will
 *                     first create one.
 *
 * \param _this         : The transactionWriter this operation operates on.
 * \param transactionId : The transaction id to look for.
 *
 * \return              : The found or newly created transaction object.
 */

static v_transaction
v_transactionWriterLookupTransaction(
    v_transactionWriter _this,
    c_ulong transactionId)
{
    C_STRUCT(v_transaction) template;
    v_transaction transaction;
    v_transaction inserted = NULL;

    assert(_this);
    assert(C_TYPECHECK(_this,v_transactionWriter));

    template.transactionId = transactionId;
    transaction = c_listTemplateFind(_this->transactions, compareTransactionId, &template);
    if (transaction == NULL) {
        transaction = v_transactionNew(_this, transactionId);
        inserted = ospl_c_insert(_this->transactions, transaction);
        OS_UNUSED_ARG(inserted);
    }
    TRACE_COHERENT_UPDATE("v_transactionWriterLookupTransaction(%s) Writer(0x%"PA_PRIxADDR") "
                          "TID(%d) Transaction(0x%"PA_PRIxADDR")%s\n",
                          WRITER_SCOPE(_this), (os_address)_this,
                          transactionId, (os_address)transaction,
                          inserted?" NEW":"");
    return transaction;
}

/**
 * \brief              This operation will invoke the given action routine for all
 *                     elements belonging to the transaction.
 *
 * \param _this      : The transaction this operation operates on.
 * \param action     : The user action that is executed on each element.
 * \param arg        : The user argument that is passed to the action on each invocation.
 *
 * \return           : Not applicable.
 */
void
v_transactionWalk(
    v_transaction _this,
    v_transactionAction action,
    c_voidp arg)
{
    c_ulong nrElements, i;
    v_transactionElement element;

    assert(_this);
    assert(C_TYPECHECK(_this,v_transaction));
    assert(action);

    if (_this->elements) {
        nrElements = c_arraySize(_this->elements);
        for (i = 0; i < nrElements; i++) {
            element = _this->elements[i];
            if (element && element->message) {
                action(element->instance, element->message, arg);
                if (v_messageStateTest(element->message, L_ENDOFTRANSACTION)) {
                    /* Elements are allocated in blocks of x elements. The last
                     * element of a transaction is the EOT. */
                    break;
                }
            }
        }
    } else if (_this->eot) {
        action(NULL, v_message(_this->eot), arg);
    }
}

static v_messageEOT
clone_messageEOT(
    v_messageEOT tmpl)
{
    v_messageEOT message = NULL;

    assert(tmpl);

    message = c_new(c_getType(tmpl));
    if (message) {
        V_MESSAGE_INIT(message);
        v_nodeState(message) = v_nodeState(tmpl);
        v_message(message)->allocTime = v_message(tmpl)->allocTime;
        v_message(message)->writeTime = v_message(tmpl)->writeTime;
        v_message(message)->writerGID = v_message(tmpl)->writerGID;
        v_gidSetNil(v_message(message)->writerInstanceGID);
        v_message(message)->qos = c_keep(v_message(tmpl)->qos);
        v_message(message)->sequenceNumber = v_message(tmpl)->sequenceNumber;
        v_message(message)->transactionId = v_message(tmpl)->transactionId;
        message->publisherId = tmpl->publisherId;
        message->transactionId = tmpl->transactionId;
        message->tidList = c_keep(tmpl->tidList);
    } else {
        OS_REPORT(OS_FATAL,OS_FUNCTION,V_RESULT_INTERNAL_ERROR,
                  "Failed to allocate message.");
        assert(FALSE);
    }
    return message;
}

/**
 * \brief              This operation removes all elements from the transaction
 *                     and set EOT to complete.
 *
 * \param _this      : The transaction this operation operates on.
 *
 * \return           : Not applicable.
 */
static void
v_transactionPurge(
    v_transaction _this)
{
    v_messageEOT eot;

    assert(_this->elements != NULL);

    c_free(_this->elements);
    _this->elements = NULL;

    if (_this->eot) {
        eot = _this->eot;
        /* Make a copy of the EOT message so that the L_TRANSACTION flag can be
         * reset to indicate that this is an EOT message for a complete transaction.
         */
        _this->eot = clone_messageEOT(eot);
        if (_this->eot) {
            v_stateClear(v_messageState(v_message(_this->eot)), L_TRANSACTION);
        }
        c_free(eot);
    }
}

/**
 * \brief              This operation flushes all messages into the reader's history.
 *
 *                     This operation is executed when a transaction has become complete
 *                     and will insert all messages belonging to the transaction into the
 *                     reader's or group history depending on the owner kind and delete
 *                     the transaction from this administration.
 *
 * \param _this      : The transaction this operation operates on.
 *
 * \return           : Not applicable.
 */
void
v_transactionFlush(
    v_transaction _this,
    v_transactionAdmin owner)
{
    v_transactionAdmin admin;
    struct v_groupFlushTransactionArg arg;

    assert(_this);
    assert(C_TYPECHECK(_this,v_transaction));

    admin = v_transactionAdmin(_this->admin);
    switch(v_objectKind(v_object(admin->owner))) {
    case K_GROUP:
        arg.group = admin->owner;
        arg.txn = _this;
        if (admin != owner) {
            c_mutexLock(&v_group(admin->owner)->mutex);
        }
        v_transactionWalk(_this, v_groupFlushTransactionNoLock, &arg);
        if (admin != owner) {
            c_mutexUnlock(&v_group(admin->owner)->mutex);
        }
        v_transactionPurge(_this);
    break;
    case K_DATAREADER:
        if (admin != owner) {
            v_observerLock(v_observer(admin->owner));
        }
        v_transactionWalk(_this, v_dataReaderEntryFlushTransactionNoLock, NULL);
        if (admin != owner) {
            v_observerUnlock(v_observer(admin->owner));
        }
    break;
    default: assert(0);
    break;
    }
}

void
v_transactionTriggerList(
    v_transaction _this,
    c_iter triggerList)
{
    v_transactionAdmin admin;

    assert(_this);
    assert(C_TYPECHECK(_this,v_transaction));

    admin = v_transactionAdmin(_this->admin);
    if (v_objectKind(v_object(admin->owner)) == K_DATAREADER) {
        if (_this->size > 0) {
            c_iterInsert(triggerList, admin->owner);
        }
    }
}

/**
 * \brief              This operation inserts a received coherent message into the
 *                     administration.
 *
 * \param _this      : The transaction administration this operation operates on.
 * \param msg        : The coherent message that will be inserted.
 * \param instance   : The instance in which the message must be inserted as soon as
 *                     the transaction becomes complete. If instance == NULL then
 *                     messages are not inserted but only registered to verify
 *                     completeness of the transaction.
 * \param abort      :
 * \param[out] complete : Completeness of transaction is written to this variable.
 *
 * \return           : The write status, whether it succeeded or not and for what reason.
 *
 * This function will lookup or create the transactionWriter and transaction object for the
 * given coherent message and register the message in the transaction object.
 * If this message causes a transaction to become complete this operation will either flush
 * all the transaction to the DataReader's history or move the transaction to the groupAdmin
 * in case the transaction is part of a group coherent update.
 */
v_writeResult
v_transactionAdminInsertMessage(
    v_transactionAdmin _this,
    v_message msg,
    v_instance instance,
    c_bool abort,
    c_bool *complete)
{
    v_transactionWriter writer;
    v_transaction transaction;
    v_writeResult result = V_WRITE_UNDEFINED;
    c_bool complete_ = FALSE;

    assert(_this);
    assert(C_TYPECHECK(_this,v_transactionAdmin));
    assert(C_TYPECHECK(instance,v_instance));
    assert(v_message_isTransaction(msg));

    TRACE_COHERENT_UPDATE("v_transactionAdminInsertMessage(%s): admin(%"PA_PRIxADDR") TID(%d) WGID(%d) "
                          "msg(0x%"PA_PRIxADDR") instance(0x%"PA_PRIxADDR") SEQ(%u) topic(%s)%s\n",
                          ADMIN_SCOPE(_this), (os_address)_this,
                          msg->transactionId,
                          msg->writerGID.localId, (os_address)msg,
                          (os_address)instance, msg->sequenceNumber,
                          v_topicName(_this->topic),
                          v_messageStateTest(msg,L_ENDOFTRANSACTION)?" EOT":"");

    writer = v_transactionAdminLookupWriter(_this, msg->writerGID);
    if (writer) {
        transaction = v_transactionWriterLookupTransaction(writer, msg->transactionId);
        if (transaction) {
            result = v_transactionInsertMessage(transaction, instance, msg, abort);
            complete_ = v_transactionComplete(transaction);
            if (complete_) {
                assert(transaction->eot);
                v_transactionWriterRemoveTransaction(writer, msg->transactionId);
                if (transaction->eot->publisherId != 0) {
                    writer->publisherId = transaction->eot->publisherId;
                }
                complete_ = v_transactionAdminFlush(_this, transaction);
                c_free(transaction);
            }
            c_free(transaction);
        }
        c_free(writer);
    }
    if (complete) {
        *complete = complete_;
    }
    return result;
}

struct writerWalkArg {
    c_action action;
    c_voidp arg;
};

static c_bool
writerWalk(
    c_object o,
    c_voidp arg)
{
    v_transactionWriter w = v_transactionWriter(o);
    struct writerWalkArg *a = (struct writerWalkArg *)arg;

    return c_walk(w->transactions, a->action, a->arg);
}

/**
 * \brief              This operation will perform a user action on all transactions.
 *
 * \param _this      : The transaction administration this operation operates on.
 * \param action     : The user action that is executed on each transaction.
 * \param arg        : The user argument that is passed to the action on each invocation.
 *
 * \return           : Not applicable.
 */
void
v_transactionAdminWalkTransactions(
    v_transactionAdmin _this,
    c_action action,
    c_voidp arg)
{
    struct writerWalkArg walkArg;

    assert(_this);
    assert(C_TYPECHECK(_this,v_transactionAdmin));

    walkArg.action = action;
    walkArg.arg = arg;

    v_transactionAdminPurgeHistory(_this);
    (void)c_walk(_this->history, action, arg);
    (void)c_walk(_this->writers, writerWalk, &walkArg);
}

/**
 * \brief              This operation will perform a user action on all discovered DataWriters.
 *
 * \param _this      : The transaction administration this operation operates on.
 * \param action     : The user action that is executed on each discovered DataWriter.
 * \param arg        : The user argument that is passed to the action on each invocation.
 *
 * \return           : Not applicable.
 */
void
v_transactionAdminWalkWriters(
    v_transactionAdmin _this,
    c_action action,
    c_voidp arg)
{
    assert(_this);
    assert(C_TYPECHECK(_this,v_transactionAdmin));

    (void)c_walk(_this->writers, action, arg);
}

v_transactionAdmin
v_transactionGetAdmin(
    v_transaction _this)
{
    return v_transactionAdmin(_this->admin);
}

v_transactionGroupAdmin
v_transactionGetGroupAdmin(
    v_transactionAdmin _this)
{
    return (v_transactionGroupAdmin)_this->groupAdmin;
}

void
v_transactionAdminTrigger(
    v_transactionAdmin _this)
{
    if (_this && _this->groupAdmin) {
        v_transactionGroupAdminTrigger((v_transactionGroupAdmin)_this->groupAdmin, NULL);
    }
}

/**
 * \brief              This operation will send a SampleLost notification to the owner.
 *
 * \param _this      : The transaction this operation operates on.
 * \param admin      : The admin responsible for this notification, the notify call for
 *                     the owner which matches the admin is done without owner lock.
 *
 * \return           : Not applicable.
 */
void
v_transactionNotifySampleLost(
    v_transaction _this,
    v_transactionAdmin admin)
{
    v_object owner;
    c_ulong partitions;
    c_ulong nrSamplesLost;

    assert(_this != NULL);
    assert(C_TYPECHECK(_this,v_transaction));
    assert(C_TYPECHECK(admin,v_transactionAdmin));

    owner = TRANSACTION_OWNER(_this);
    if ((_this->aborted) &&
        (_this->sampleLostNotified == FALSE)) {
        /* Samples will not be delivered because the transaction is aborted so notify
         * the number of lost samples.
         */
        if (v_objectKind(owner) == K_DATAREADER) {
            partitions = v_transactionWriter(_this->writer)->matchCount;
            if (partitions == 0) {
                if ((v_readerSubscriber(v_reader(owner))) &&
                    (v_subscriberPartitionCount(v_readerSubscriber(v_reader(owner))) == 1)) {
                    partitions = 1; /* no need to wait for discovery if subscriber has only one partition. */
                }
            }
            if (_this->eot) {
                nrSamplesLost = (c_ulong)_this->size * partitions;
            } else {
                assert(_this->count > 0);
                /* Since we have not yet received an eot message we are not sure
                 * how many samples were actually lost. Best guess is to use the
                 * count as this is increased for every message (part of the
                 * transaction) received. So messages which are part of the transaction
                 * but not yet received are not part of the count.
                 */
                nrSamplesLost = (c_ulong)_this->count;
            }

            if (nrSamplesLost > 0) {
                if (_this->admin == admin) {
                    v_dataReaderNotifySampleLost(v_dataReader(owner), nrSamplesLost);
                } else {
                    v_dataReaderNotifySampleLostLock(v_dataReader(owner), nrSamplesLost);
                }
            }
        }
        _this->sampleLostNotified = TRUE;
    }
}

struct transactionWriterMessageFind {
    v_instance instance;
    v_gid writerGid;
};


static c_bool
v_transactionWriterMessageNotExist(
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
v_transactionAdminNoMessageFromWriterExist(
    v_transactionAdmin _this,
    v_groupInstance instance,
    v_gid writerGid)
{
    struct transactionWriterMessageFind info;
    struct writerWalkArg walkArg;

    assert(_this);
    assert(instance);

    info.instance = v_instance(instance);
    info.writerGid = writerGid;

    walkArg.action = v_transactionWriterMessageNotExist;
    walkArg.arg = &info;

    return c_walk(_this->writers, writerWalk, &walkArg);
}


/**
 * \brief              This operation removes transactions from the history when
 *                     the contained transactions are not referenced by samples
 *                     anymore
 *
 * \param _this      : The v_transactionAdmin this operation operates on.
 *
 * \return           : None
 */
void
v_transactionAdminPurgeHistory(
    v_transactionAdmin _this)
{
    v_transaction txn;
    struct c_collectionIterD it;

    if (v_objectKind(ADMIN_OWNER(_this)) == K_GROUP) {
        for (txn = c_collectionIterDFirst(_this->history, &it); txn; txn = c_collectionIterDNext(&it)) {
            if (c_refCount(txn) == 1) {
                TRACE_COHERENT_UPDATE("%s(%s) Admin(0x%"PA_PRIxADDR") Removed TID(%u) from history\n",
                                      OS_FUNCTION, ADMIN_SCOPE(_this), (os_address)_this, txn->transactionId);
                c_collectionIterDRemove(&it);
                c_free(txn);
            }
        }
    }
}

