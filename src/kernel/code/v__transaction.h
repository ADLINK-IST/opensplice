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

#ifndef V__TRANSACTION_H
#define V__TRANSACTION_H

/**
 * \file kernel/code/v__transaction.h
 * \brief This file defines the kernel interface of the v_transaction classes.
 *
 * These classes implements the reader's administration for managing coherent updates.
 * This administration will register all discovered matching coherent DataWriters and
 * for each DataWriter register all active transactions and for each active transaction
 * a list of received messages.
 *
 * The implementation of this interface consists of:
 *
 * - A v_transactionAdmin class   : The main class implementing the coherent readers
 *                                  transaction administration which will hold all
 *                                  discovered matching coherent DataWriters.
 * - A v_transactionWriter class  : Internal class implementing an administration record
 *                                  for each discovered matching coherent DataWriter which
 *                                  will hold all active transactions.
 * - A v_transaction class        : Internal class implementing an administration record 
 *                                  for each active transaction which will hold all
 *                                  received messages.
 * - A v_transactionElement class : Internal class implementing an administration record 
 *                                  for each received transaction message.
 */

#if defined (__cplusplus)
extern "C" {
#endif

#include "v_kernel.h"

/**
 * \brief The following MARCROs are defined to cast references to the correct type.
 *
 * If compiled with the NDEBUG flag not set, these macros will additionally perform
 * runtime type checking, and in case of providing an incorrect type these macros will 
 * return a NULL reference.
 */
#define v_messageEOT(o) (C_CAST(o,v_messageEOT))
#define v_transaction(o) (C_CAST(o,v_transaction))
#define v_transactionWriter(o) (C_CAST(o,v_transactionWriter))
#define v_transactionAdmin(o) (C_CAST(o,v_transactionAdmin))
#define v_transactionElement(o) (C_CAST(o,v_transactionElement))

typedef void (*v_transactionAction) (v_instance instance, v_message message, c_voidp arg);

/**
 * \brief              The <code>v_transactionAdmin</code> constructor.
 *
 * \param owner      : this is the object that will be the owner of the administration,
 *                     this is either a v_dataReader or a v_group for durable data.
 *
 * \param groupAdmin : this is a reference to a group coherence administration which
 *                     is only set for group coherence, for other coherent scopes
 *                     this parameter will be set to NULL.
 *
 * \return           : <code>NULL</code> if this operation fails, otherwise
 *                     a reference to a newly created v_transactionAdmin.
 */
v_transactionAdmin
v_transactionAdminNew(
    v_object owner,
    v_transactionGroupAdmin groupAdmin,
    v_topic topic);

/**
 * \brief              This operation notifies the transactionAdmin about discovered
 *                     matching coherent DataWriters.
 *
 * \param _this       : The v_transactionAdmin this operation operates on.
 * \param writerGID   : The global identifier of the discovered DataWriter.
 * \param dispose     : A boolean specifying whether we discovered the appearance or
 *                      disappearance of a DataWriter.
 * \param info        : Publication information of a DataWriter.
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
    c_bool isImplicit);

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
    c_bool *complete);

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
    c_voidp arg);

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
    c_voidp arg);

/**
 * \brief              This operation flushes all messages into the reader's history.
 *
 *                     This operation is executed when a transaction has become complete
 *                     and will insert all messages belonging to the transaction into the
 *                     reader's history and delete the transaction from this administration. 
 *
 * \param _this      : The transaction this operation operates on.
 *                     
 * \return           : Not applicable.
 */
void
v_transactionFlush(
    v_transaction _this,
    v_transactionAdmin owner);

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
    c_voidp arg);

/**
 * \brief              This operation aborts a transaction.
 *
 *                     This operation will free all reserved resources and
 *                     messages belonging to the transaction.
 *
 * \param _this      : The transaction this operation operates on.
 *
 * \return           : Not applicable.
 */
void
v_transactionAbort(
    v_transaction _this);

/**
 * \brief              This operation test if a transaction is aborted.
 *
 * \param _this      : The transaction this operation operates on.
 *
 * \return           : TRUE if aborted otherwise FALSE.
 */
c_bool
v_transactionIsAborted(
    v_transaction _this);

v_transactionAdmin
v_transactionGetAdmin(
    v_transaction _this);

v_transactionGroupAdmin
v_transactionGetGroupAdmin(
    v_transactionAdmin _this);

void
v_transactionTriggerList(
    v_transaction _this,
    c_iter triggerList);

void
v_transactionAdminTrigger(
    v_transactionAdmin _this);

void
v_transactionNotifySampleLost(
    v_transaction _this,
    v_transactionAdmin admin);

/**
 * \brief              This operation checks if the transaction admin does not contain
 *                     samples from a writer for a particular group instance.
 *
 *                     This operation walks over the transactions to check if an message
 *                     from a particular writer (writerGid) exists for a particular
 *                     group instance. When no such message is found the operation
 *                     returns TRUE.
 *
 * \param _this      : The transaction admin this operation operates on.
 * \param instance   : The group instance.
 * \param writerGid  : The gid of the writer.
 *
 * \return           : TRUE when no message is found related to the writer and the
 *                     group instance, FALSE otherwise.
 */
c_bool
v_transactionAdminNoMessageFromWriterExist(
    v_transactionAdmin _this,
    v_groupInstance instance,
    v_gid writerGid);

void
v_transactionAdminPurgeHistory(
    v_transactionAdmin _this);

#if defined (__cplusplus)
}
#endif

#endif /* V__TRANSACTION_H */
