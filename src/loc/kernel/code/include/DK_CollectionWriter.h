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
#ifndef DLRL_KERNEL_COLLECTION_WRITER_H
#define DLRL_KERNEL_COLLECTION_WRITER_H

/* user layer includes */
#include "u_entity.h"

/* DLRL util includes */
#include "DLRL_Exception.h"
#include "DLRL_Types.h"

/* DLRL MetaModel includes */
#include "DMM_DLRLMultiRelation.h"

/* DLRL Kernel includes */
#include "DK_Entity.h"
#include "DK_Types.h"

#if defined (__cplusplus)
extern "C" {
#endif

struct DK_CollectionWriter_s
{
    DK_Entity entity;
    LOC_boolean alive;
    u_writer writer;
    DLRL_LS_object ls_writer;
    DK_TopicInfo* topicInfo;
};

DK_CollectionWriter*
DK_CollectionWriter_new(
    DLRL_Exception* exception,
    u_writer writer,
    DLRL_LS_object ls_writer,
    DK_TopicInfo* topicInfo);

void
DK_CollectionWriter_us_delete(
    DK_CollectionWriter* _this,
    void* userData);

void
DK_CollectionWriter_us_write(
    DK_CollectionWriter* _this,
    DLRL_Exception* exception,
    DK_Collection* collection);

void
DK_CollectionWriter_us_dispose(
    DK_CollectionWriter* _this,
    DLRL_Exception* exception,
    DK_Collection* collection);

/* NOT IN DESIGN - check entire class... */
void
DK_CollectionWriter_us_enable(
    DK_CollectionWriter* _this,
    DLRL_Exception* exception,
    void* userData);

DK_TopicInfo*
DK_CollectionWriter_us_getTopicInfo(
    DK_CollectionWriter* _this);

DLRL_LS_object
DK_CollectionWriter_us_getLSWriter(
    DK_CollectionWriter* _this);

#if defined (__cplusplus)
}
#endif

#endif /* DLRL_KERNEL_COLLECTION_WRITER_H */
