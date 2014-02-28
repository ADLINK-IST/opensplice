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
#ifndef GAPI_COMMON_H
#define GAPI_COMMON_H

#include "gapi.h"
#include "c_typebase.h"
#include "os_if.h"

#ifdef OSPL_BUILD_CORE
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

#ifdef _USE_ENTITY_REGISTER_OBJECT
#define _ENTITY_REGISTER_OBJECT(a,b)    _EntityRegisterObject(_Entity(b), (_Object)a)
#else
#define _ENTITY_REGISTER_OBJECT(a,b)    _DomainParticipantFactoryRegister((_Object)b)
#endif

#define GAPI_MAX(a,b) ((a >= b)?a:b)

#define GAPI_CONTEXT_SET(c,e,m) { c.entity = e; c.methodId = m; }

/* Class forward references needed because classes refer each other */
C_CLASS(_ContentFilteredTopic);
C_CLASS(_DataReader);
C_CLASS(_DataReaderView);
C_CLASS(_DataReaderListener);
C_CLASS(_DataWriter);
C_CLASS(_DataWriterListener);
C_CLASS(_DomainParticipantFactory);
C_CLASS(_DomainParticipant);
C_CLASS(_DomainParticipantListener);
C_CLASS(_Entity);
C_CLASS(_EntityValidity);
C_CLASS(_GarbageBin);
C_CLASS(_Listener);
C_CLASS(_MultiTopic);
C_CLASS(_Publisher);
C_CLASS(_PublisherListener);
C_CLASS(_Subscriber);
C_CLASS(_SubscriberListener);
C_CLASS(_TopicDescription);
C_CLASS(_Topic);
C_CLASS(_TopicListener);
C_CLASS(_TypeSupport);
C_CLASS(_FooTypeSupport);
C_CLASS(_FooDataWriter);
C_CLASS(_FooDataReader);
C_CLASS(_FooDataReaderView);
C_CLASS(_Condition);
C_CLASS(_StatusCondition);
C_CLASS(_ReadCondition);
C_CLASS(_GuardCondition);
C_CLASS(_QueryCondition);
C_CLASS(_WaitSet);
C_CLASS(_WaitSetDomainEntry);
C_CLASS(_Status);
C_CLASS(_ListenerInterestInfo);
C_CLASS(_ErrorInfo);


typedef struct gapi_context_s {
    gapi_entity entity;
    gapi_unsigned_long methodId;
} gapi_context;

typedef enum gapi_equality {
    GAPI_PL = -4,    /* Partial less (structure)    */
    GAPI_EL = -3,    /* Less or Equal (set)         */
    GAPI_LE = -2,    /* Less or Equal               */
    GAPI_LT = -1,    /* Less                        */
    GAPI_EQ = 0,     /* Equal                       */
    GAPI_GT = 1,     /* Greater                     */
    GAPI_GE = 2,     /* Greater or Equal            */
    GAPI_EG = 3,     /* Greater or Equal (set)      */
    GAPI_PG = 4,     /* Partial greater (structure) */
    GAPI_PE = 10,    /* Partial Equal               */
    GAPI_NE = 20,    /* Not equal                   */
    GAPI_ER = 99     /* Error: equality undefined   */
} gapi_equality;

OS_API gapi_equality
gapi_stringCompare (
    gapi_char *str1,
    gapi_char *str2);

OS_API gapi_equality
gapi_objectRefCompare (
    gapi_object obj1,
    gapi_object obj2);

OS_API char *
gapi_strdup (
    const char *src);

OS_API void *
gapi_alloc (
    gapi_unsigned_long len);

OS_API gapi_boolean
gapi_sequence_is_valid (
    const void *seq);

OS_API gapi_boolean
gapi_validDuration(
    const gapi_duration_t *duration);

OS_API gapi_boolean
gapi_validTime(
    const gapi_time_t *time);

OS_API gapi_boolean
gapi_stringSeqValid (
    const gapi_stringSeq *seq);

OS_API gapi_boolean
gapi_sampleStateMaskValid (
    gapi_sampleStateMask mask);

OS_API gapi_boolean
gapi_viewStateMaskValid (
    gapi_viewStateMask mask);

OS_API gapi_boolean
gapi_instanceStateMaskValid (
    gapi_instanceStateMask mask);

OS_API gapi_boolean
gapi_stateMasksValid (
    gapi_sampleStateMask   sampleMask,
    gapi_viewStateMask     viewMask,
    gapi_instanceStateMask instanceMask);

OS_API gapi_boolean
gapi_stringToLongLong (
    const gapi_char *str,
    gapi_long_long *retval);

#undef OS_API

#endif /* GAPI_COMMON_H */
