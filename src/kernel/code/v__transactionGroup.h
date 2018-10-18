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

#ifndef V__TRANSACTIONGROUP_H
#define V__TRANSACTIONGROUP_H

/**
 * \file kernel/code/v__transactionGroup.h
 * \brief This file defines the kernel interface of the v_transactionGroup classes.
 *
 * These classes implements the subscriber's administration for managing coherent updates.
 * This administration will register all discovered Publishers having matching coherent
 * DataWriters and for each register all completed DataWriter transactions until all
 * transactions belonging to the group coherent update are complete and then flush all
 * messages from the transactions to the DataReader histories.
 *
 * The implementation of this interface consists of:
 *
 * - A v_transactionGroupAdmin class : The main class implementing the coherent subscribers
 *                                     transaction group administration which will hold all
 *                                     discovered matching coherent Publishers.
 * - A v_transactionPublisher class  : Internal class implementing an administration record
 *                                     for each discovered matching coherent Publisher which
 *                                     will hold all active transaction groups.
 * - A v_transactionGroup class      : Internal class implementing an administration record
 *                                     for each active transaction group which will hold all
 *                                     completed DataWriter transactions.
 */

#if defined (__cplusplus)
extern "C" {
#endif

#include "v_kernel.h"

/**
 * \brief The following v_transactionGroup macro is defined to cast a reference to the correct type.
 *
 * If compiled with the NDEBUG flag not set, this macro will additionally perform
 * runtime type checking, and in case of providing an incorrect type the macro will
 * return a NULL reference.
 */
#define v_transactionGroup(o) (C_CAST(o,v_transactionGroup))
#define v_transactionGroupAdmin(o) (C_CAST(o,v_transactionGroupAdmin))

/**
 * \brief             The <code>v_transactionGroupAdmin</code> constructor.
 *
 * \param owner       : this is the object that will be the owner of the administration,
 *                      this is either a v_subscriber or a v_kernel for durable data.
 *
 * \return            : a reference to a newly created v_transactionGroupAdmin.
 */
_Check_return_
_Ret_notnull_
_Pre_satisfies_(v_objectKind(owner) == K_KERNEL || v_objectKind(owner) == K_SUBSCRIBER)
v_transactionGroupAdmin
v_transactionGroupAdminNew(
    _In_ v_object owner);

/**
 * \brief             This operation inserts a completed transaction belonging to a
 *                    group coherent update into the group administration.
 *
 * \param _this       : The transaction group administration this operation operates on.
 * \param transaction : The completed DataWriter transaction that will be inserted.
 * \param topic       : The topic belonging to the DataWriter for which the transaction is
 *                      inserted.
 *
 * \return            : TRUE when the group transaction is added to the pending list.
 */
c_bool
v_transactionGroupAdminInsertTransaction(
    v_transactionGroupAdmin _this,
    v_transaction transaction,
    v_topic topic);

void
v_transactionGroupAdminAddReader(
    v_transactionGroupAdmin _this,
    v_reader reader);

void
v_transactionGroupAdminRemoveReader(
    v_transactionGroupAdmin _this,
    v_reader reader);

void
v_transactionGroupAdminFlush(
    v_transactionGroupAdmin _this);

c_bool
v_transactionGroupAdminNotifyPublication(
    v_transactionGroupAdmin _this,
    v_transactionWriter writer,
    c_bool dispose,
    struct v_publicationInfo *info);

void
v_transactionGroupAdminWalkTransactions(
    v_transactionGroupAdmin _this,
    v_group group,
    c_action action,
    c_voidp arg);

void
v_transactionGroupAdminFlushPending(
    v_transactionGroupAdmin _this,
    v_transactionAdmin admin);

/**
 * \brief              This operation checks if the transaction group admin does
 *                     not contain samples from a writer for a particular group instance.
 *
 *                     This operation walks over the transactions to check if an message
 *                     from a particular writer (writerGid) exists for a particular
 *                     group instance. When no such message is found the operation
 *                     returns TRUE.
 *
 * \param _this      : The transaction group admin this operation operates on.
 * \param instance   : The group instance.
 * \param writerGid  : The gid of the writer.
 *
 * \return           : TRUE when no message is found related to the writer and the
 *                     group instance, FALSE otherwise.
 */
c_bool
v_transactionGroupAdminNoMessageFromWriterExist(
    v_transactionGroupAdmin _this,
    v_groupInstance instance,
    v_gid writerGid);

void
v_transactionGroupLink(
    v_transactionGroup _this);

void
v_transactionGroupUnlink(
    v_transactionGroup _this);

#if defined (__cplusplus)
}
#endif

#endif /* V__TRANSACTIONGROUP */
