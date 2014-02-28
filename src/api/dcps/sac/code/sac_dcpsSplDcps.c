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

#include "dds_dcpsSplDcps.h"
#include "dds_dcps.h"
#include "dds_dcps_private.h"
#include "os_report.h"

c_bool
__DDS_Duration_t__copyIn(c_base base, void *_from, void *_to)
{
    DDS_Duration_t *from = (DDS_Duration_t *)_from;
    struct _DDS_Duration_t *to = (struct _DDS_Duration_t *)_to;
    (void)base; /* avoid warning */
    to->sec = (c_long)from->sec;
    to->nanosec = (c_ulong)from->nanosec;
    
    return TRUE;
}

c_bool
__DDS_Time_t__copyIn(c_base base, void *_from, void *_to)
{
    DDS_Time_t *from = (DDS_Time_t *)_from;
    struct _DDS_Time_t *to = (struct _DDS_Time_t *)_to;
    (void)base; /* avoid warning */
    to->sec = (c_long)from->sec;
    to->nanosec = (c_ulong)from->nanosec;
    
    return TRUE;
}

c_bool
__DDS_octSeq__copyIn(c_base base, void *_from, void *_to)
{
    c_bool result = TRUE;

    DDS_octSeq *from = (DDS_octSeq *)_from;
    _DDS_octSeq *to = (_DDS_octSeq *)_to;

    static c_type type0 = NULL;
    c_type subtype0 = NULL;
    c_long length0;
    c_octet *dst0;
    DDS_octSeq *from0 = from;
    (void) base;

    subtype0 = c_type(c_metaResolve (c_metaObject(base), "c_octet"));
    type0 = c_metaSequenceTypeNew(c_metaObject(base),"C_SEQUENCE<c_octet>",subtype0,0);
    c_free(subtype0);

    length0 = (c_long)(*from)._length;
    dst0 = (c_octet *)c_newSequence(c_collectionType(type0),length0);
    memcpy (dst0,(*from0)._buffer,length0* sizeof(*dst0));
    *to = (_DDS_octSeq)dst0;

    return result;
}

c_bool
__DDS_BuiltinTopicKey_t__copyIn(c_base base, void *_from, void *_to)
{
    DDS_BuiltinTopicKey_t *from = (DDS_BuiltinTopicKey_t *)_from;
    _DDS_BuiltinTopicKey_t *to = (_DDS_BuiltinTopicKey_t *)_to;
    (void) base;

    memcpy (to, *from, sizeof (*to));

    return TRUE;
}

c_bool
__DDS_StringSeq__copyIn(c_base base, void *_from, void *_to)
{
    c_bool result = TRUE;

    DDS_StringSeq *from = (DDS_StringSeq *)_from;
    _DDS_StringSeq *to = (_DDS_StringSeq *)_to;

    static c_type type0 = NULL;
    c_type subtype0 = NULL;
    c_long length0;
    c_string *dst0;
    DDS_StringSeq *from0 = from;

    subtype0 = c_type(c_metaResolve (c_metaObject(base), "c_string"));
    type0 = c_metaSequenceTypeNew(c_metaObject(base),"C_SEQUENCE<c_string>",subtype0,0);
    c_free(subtype0);

    length0 = (c_long)(*from)._length;
    dst0 = (c_string *)c_newSequence(c_collectionType(type0),length0);

    if((*from0)._buffer) {
        unsigned int i0;
        for (i0 = 0; (i0 < (unsigned int)length0) && result; i0++) {
#ifdef OSPL_BOUNDS_CHECK
            if(!(*from0)._buffer[i0]){
                OS_REPORT (OS_ERROR, "copyIn", 0,"Element of 'DDS.StringSeq' of type 'c_string' is NULL.");
                result = FALSE;
            }
#endif
            if (result) {
                dst0[i0] = c_stringNew(base, (*from0)._buffer[i0]);
            }
        }
    } else if(length0) {
        OS_REPORT (OS_ERROR, "copyIn", 0,"Element of 'DDS.StringSeq' of type 'c_string' is out of range.");
        result = FALSE;
    }
    *to = (_DDS_StringSeq)dst0;

    return result;
}

c_bool
__DDS_EntityFactoryQosPolicy__copyIn(c_base base, void *_from, void *_to)
{
    DDS_EntityFactoryQosPolicy *from = (DDS_EntityFactoryQosPolicy *)_from;
    struct _DDS_EntityFactoryQosPolicy *to = (struct _DDS_EntityFactoryQosPolicy *)_to;

    OS_UNUSED_ARG(base);

    to->autoenable_created_entities = (c_bool)from->autoenable_created_entities;

    return TRUE;
}

c_bool
__DDS_WriterDataLifecycleQosPolicy__copyIn(c_base base, void *_from, void *_to)
{
    c_bool result = TRUE;

    DDS_WriterDataLifecycleQosPolicy *from = (DDS_WriterDataLifecycleQosPolicy *)_from;
    struct _DDS_WriterDataLifecycleQosPolicy *to = (struct _DDS_WriterDataLifecycleQosPolicy *)_to;
    to->autodispose_unregistered_instances = (c_bool)from->autodispose_unregistered_instances;

    if (result) {
        result = __DDS_Duration_t__copyIn(base, &from->autopurge_suspended_samples_delay, &to->autopurge_suspended_samples_delay);
    }
    if (result) {
        result = __DDS_Duration_t__copyIn(base, &from->autounregister_instance_delay, &to->autounregister_instance_delay);
    }

    return result;
}

c_bool
__DDS_InvalidSampleVisibilityQosPolicy__copyIn(c_base base, void *_from, void *_to)
{
    c_bool result = TRUE;

    DDS_InvalidSampleVisibilityQosPolicy *from = (DDS_InvalidSampleVisibilityQosPolicy *)_from;
    struct _DDS_InvalidSampleVisibilityQosPolicy *to = (struct _DDS_InvalidSampleVisibilityQosPolicy *)_to;

    OS_UNUSED_ARG(base);

#ifdef OSPL_BOUNDS_CHECK
    if((((c_long)from->kind) < 0) || (((c_long)from->kind) >= 3) ) {
        OS_REPORT (OS_ERROR, "copyIn", 0,"Member 'DDS::InvalidSampleVisibilityQosPolicy.kind' of type 'InvalidSampleVisibilityQosPolicyKind' is out of range.");
        result = FALSE;
    }
#endif
    if (result) {
        to->kind = (enum _DDS_InvalidSampleVisibilityQosPolicyKind)from->kind;
    }

    return result;
}

c_bool
__DDS_ReaderDataLifecycleQosPolicy__copyIn(c_base base, void *_from, void *_to)
{
    c_bool result = TRUE;

    DDS_ReaderDataLifecycleQosPolicy *from = (DDS_ReaderDataLifecycleQosPolicy *)_from;
    struct _DDS_ReaderDataLifecycleQosPolicy *to = (struct _DDS_ReaderDataLifecycleQosPolicy *)_to;

    to->enable_invalid_samples = (c_bool)from->enable_invalid_samples;
    if (result) {
        result = __DDS_Duration_t__copyIn(base, &from->autopurge_nowriter_samples_delay, &to->autopurge_nowriter_samples_delay);
    }
    if (result) {
        result = __DDS_Duration_t__copyIn(base, &from->autopurge_disposed_samples_delay, &to->autopurge_disposed_samples_delay);
    }
    if (result) {
        result = __DDS_InvalidSampleVisibilityQosPolicy__copyIn(base, &from->invalid_sample_visibility, &to->invalid_sample_visibility);
    }

    return result;
}

c_bool
__DDS_SubscriptionKeyQosPolicy__copyIn(c_base base, void *_from, void *_to)
{
    c_bool result = TRUE;

    DDS_SubscriptionKeyQosPolicy *from = (DDS_SubscriptionKeyQosPolicy *)_from;
    struct _DDS_SubscriptionKeyQosPolicy *to = (struct _DDS_SubscriptionKeyQosPolicy *)_to;

    to->use_key_list = (c_bool)from->use_key_list;
    if (result) {
        result = __DDS_StringSeq__copyIn(base, &from->key_list, &to->key_list);
    }

    return result;
}

c_bool
__DDS_ReaderLifespanQosPolicy__copyIn(c_base base, void *_from, void *_to)
{
    c_bool result = TRUE;

    DDS_ReaderLifespanQosPolicy *from = (DDS_ReaderLifespanQosPolicy *)_from;
    struct _DDS_ReaderLifespanQosPolicy *to = (struct _DDS_ReaderLifespanQosPolicy *)_to;

    to->use_lifespan = (c_bool)from->use_lifespan;
    if (result) {
        result = __DDS_Duration_t__copyIn(base, &from->duration, &to->duration);
    }

    return result;
}

c_bool
__DDS_ShareQosPolicy__copyIn(c_base base, void *_from, void *_to)
{
    c_bool result = TRUE;

    DDS_ShareQosPolicy *from = (DDS_ShareQosPolicy *)_from;
    struct _DDS_ShareQosPolicy *to = (struct _DDS_ShareQosPolicy *)_to;

#ifdef OSPL_BOUNDS_CHECK
    if (!from->name) {
        OS_REPORT (OS_ERROR, "copyIn", 0,"Member 'DDS::ShareQosPolicy.name' of type 'c_string' is NULL.");
        result = FALSE;
    }
#endif
    if (result) {
        to->name = c_stringNew(base, from->name);
    }
    to->enable = (c_bool)from->enable;

    return result;
}

c_bool
__DDS_SchedulingClassQosPolicy__copyIn(c_base base, void *_from, void *_to)
{
    c_bool result = TRUE;

    DDS_SchedulingClassQosPolicy *from = (DDS_SchedulingClassQosPolicy *)_from;
    struct _DDS_SchedulingClassQosPolicy *to = (struct _DDS_SchedulingClassQosPolicy *)_to;

    OS_UNUSED_ARG(base);

#ifdef OSPL_BOUNDS_CHECK
    if((((c_long)from->kind) < 0) || (((c_long)from->kind) >= 3) ) {
        OS_REPORT (OS_ERROR, "copyIn", 0,"Member 'DDS::SchedulingClassQosPolicy.kind' of type 'SchedulingClassQosPolicyKind' is out of range.");
        result = FALSE;
    }
#endif
    if (result) {
        to->kind = (enum _DDS_SchedulingClassQosPolicyKind)from->kind;
    }

    return result;
}

c_bool
__DDS_SchedulingPriorityQosPolicy__copyIn(c_base base, void *_from, void *_to)
{
    c_bool result = TRUE;

    DDS_SchedulingPriorityQosPolicy *from = (DDS_SchedulingPriorityQosPolicy *)_from;
    struct _DDS_SchedulingPriorityQosPolicy *to = (struct _DDS_SchedulingPriorityQosPolicy *)_to;

    OS_UNUSED_ARG(base);

#ifdef OSPL_BOUNDS_CHECK
    if((((c_long)from->kind) < 0) || (((c_long)from->kind) >= 2) ) {
        OS_REPORT (OS_ERROR, "copyIn", 0,"Member 'DDS::SchedulingPriorityQosPolicy.kind' of type 'SchedulingPriorityQosPolicyKind' is out of range.");
        result = FALSE;
    }
#endif
    if (result) {
        to->kind = (enum _DDS_SchedulingPriorityQosPolicyKind)from->kind;
    }

    return result;
}

c_bool
__DDS_SchedulingQosPolicy__copyIn(c_base base, void *_from, void *_to)
{
    c_bool result = TRUE;

    DDS_SchedulingQosPolicy *from = (DDS_SchedulingQosPolicy *)_from;
    struct _DDS_SchedulingQosPolicy *to = (struct _DDS_SchedulingQosPolicy *)_to;

    to->scheduling_priority = (c_long)from->scheduling_priority;
    if (result) {
        result = __DDS_SchedulingClassQosPolicy__copyIn(base, &from->scheduling_class, &to->scheduling_class);
    }
    if (result) {
        result = __DDS_SchedulingPriorityQosPolicy__copyIn(base, &from->scheduling_priority_kind, &to->scheduling_priority_kind);
    }

    return result;
}

c_bool
__DDS_ViewKeyQosPolicy__copyIn(c_base base, void *_from, void *_to)
{
    c_bool result = TRUE;

    DDS_ViewKeyQosPolicy *from = (DDS_ViewKeyQosPolicy *)_from;
    struct _DDS_ViewKeyQosPolicy *to = (struct _DDS_ViewKeyQosPolicy *)_to;

    to->use_key_list = (c_bool)from->use_key_list;
    if (result) {
        result = __DDS_StringSeq__copyIn(base, &from->key_list, &to->key_list);
    }

    return result;
}

c_bool
__DDS_DataReaderViewQos__copyIn(c_base base, void *_from, void *_to)
{
    c_bool result = TRUE;

    DDS_DataReaderViewQos *from = (DDS_DataReaderViewQos *)_from;
    struct _DDS_DataReaderViewQos *to = (struct _DDS_DataReaderViewQos *)_to;

    if (result) {
        result = __DDS_ViewKeyQosPolicy__copyIn(base, &from->view_keys, &to->view_keys);
    }

    return result;
}

c_bool
__DDS_OwnershipStrengthQosPolicy__copyIn(c_base base, void *_from, void *_to)
{
    DDS_OwnershipStrengthQosPolicy *from = (DDS_OwnershipStrengthQosPolicy *)_from;
    struct _DDS_OwnershipStrengthQosPolicy *to = (struct _DDS_OwnershipStrengthQosPolicy *)_to;
    (void) base;

    to->value = (c_long)from->value;

    return TRUE;
}

c_bool
__DDS_DurabilityServiceQosPolicy__copyIn(c_base base, void *_from, void *_to)
{
    c_bool result = TRUE;

    DDS_DurabilityServiceQosPolicy *from = (DDS_DurabilityServiceQosPolicy *)_from;
    struct _DDS_DurabilityServiceQosPolicy *to = (struct _DDS_DurabilityServiceQosPolicy *)_to;

#ifdef OSPL_BOUNDS_CHECK
    if((((c_long)from->history_kind) < 0) || (((c_long)from->history_kind) >= 2) ){
        OS_REPORT (OS_ERROR, "copyIn", 0,"Member 'DDS::DurabilityServiceQosPolicy.history_kind' of type 'HistoryQosPolicyKind' is out of range.");
        result = FALSE;
    }
#endif
    if (result) {
        to->history_kind = (enum _DDS_HistoryQosPolicyKind)from->history_kind;
    }
    if (result) {
        result = __DDS_Duration_t__copyIn(base, &from->service_cleanup_delay, &to->service_cleanup_delay);
    }

    to->history_depth = (c_long)from->history_depth;
    to->max_samples = (c_long)from->max_samples;
    to->max_instances = (c_long)from->max_instances;
    to->max_samples_per_instance = (c_long)from->max_samples_per_instance;

    return result;
}

c_bool
__DDS_ResourceLimitsQosPolicy__copyIn(c_base base, void *_from, void *_to)
{
    DDS_ResourceLimitsQosPolicy *from = (DDS_ResourceLimitsQosPolicy *)_from;
    struct _DDS_ResourceLimitsQosPolicy *to = (struct _DDS_ResourceLimitsQosPolicy *)_to;
    (void) base;

    to->max_samples = (c_long)from->max_samples;
    to->max_instances = (c_long)from->max_instances;
    to->max_samples_per_instance = (c_long)from->max_samples_per_instance;

    return TRUE;
}

c_bool
__DDS_LifespanQosPolicy__copyIn(c_base base, void *_from, void *_to)
{
    c_bool result = TRUE;

    DDS_LifespanQosPolicy *from = (DDS_LifespanQosPolicy *)_from;
    struct _DDS_LifespanQosPolicy *to = (struct _DDS_LifespanQosPolicy *)_to;

    if (result) {
        result = __DDS_Duration_t__copyIn(base, &from->duration, &to->duration);
    }

    return result;
}

c_bool
__DDS_TopicDataQosPolicy__copyIn(c_base base, void *_from, void *_to)
{
    c_bool result = TRUE;

    DDS_TopicDataQosPolicy *from = (DDS_TopicDataQosPolicy *)_from;
    struct _DDS_TopicDataQosPolicy *to = (struct _DDS_TopicDataQosPolicy *)_to;

    if (result) {
        result = __DDS_octSeq__copyIn(base, &from->value, &to->value);
    }

    return result;
}

c_bool
__DDS_OwnershipQosPolicy__copyIn(c_base base, void *_from, void *_to)
{
    c_bool result = TRUE;

    DDS_OwnershipQosPolicy *from = (DDS_OwnershipQosPolicy *)_from;
    struct _DDS_OwnershipQosPolicy *to = (struct _DDS_OwnershipQosPolicy *)_to;

    OS_UNUSED_ARG(base);

#ifdef OSPL_BOUNDS_CHECK
    if((((c_long)from->kind) < 0) || (((c_long)from->kind) >= 2) ){
        OS_REPORT (OS_ERROR, "copyIn", 0,"Member 'DDS::OwnershipQosPolicy.kind' of type 'OwnershipQosPolicyKind' is out of range.");
        result = FALSE;
    }
#endif
    if (result) {
        to->kind = (enum _DDS_OwnershipQosPolicyKind)from->kind;
    }

    return result;
}

c_bool
__DDS_PartitionQosPolicy__copyIn(c_base base, void *_from, void *_to)
{
    c_bool result = TRUE;

    DDS_PartitionQosPolicy *from = (DDS_PartitionQosPolicy *)_from;
    struct _DDS_PartitionQosPolicy *to = (struct _DDS_PartitionQosPolicy *)_to;

    if (result) {
        result = __DDS_StringSeq__copyIn(base, &from->name, &to->name);
    }

    return result;
}

c_bool
__DDS_ReliabilityQosPolicy__copyIn(c_base base, void *_from, void *_to)
{
    c_bool result = TRUE;

    DDS_ReliabilityQosPolicy *from = (DDS_ReliabilityQosPolicy *)_from;
    struct _DDS_ReliabilityQosPolicy *to = (struct _DDS_ReliabilityQosPolicy *)_to;

#ifdef OSPL_BOUNDS_CHECK
    if((((c_long)from->kind) < 0) || (((c_long)from->kind) >= 2) ){
        OS_REPORT (OS_ERROR, "copyIn", 0,"Member 'DDS::ReliabilityQosPolicy.kind' of type 'ReliabilityQosPolicyKind' is out of range.");
        result = FALSE;
    }
#endif
    if (result) {
        to->kind = (enum _DDS_ReliabilityQosPolicyKind)from->kind;
    }
    if (result) {
        result = __DDS_Duration_t__copyIn(base, &from->max_blocking_time, &to->max_blocking_time);
    }

    to->synchronous = (c_bool)from->synchronous;

    return result;
}

c_bool
__DDS_GroupDataQosPolicy__copyIn(c_base base, void *_from, void *_to)
{
    c_bool result = TRUE;

    DDS_GroupDataQosPolicy *from = (DDS_GroupDataQosPolicy *)_from;
    struct _DDS_GroupDataQosPolicy *to = (struct _DDS_GroupDataQosPolicy *)_to;

    if (result) {
        result = __DDS_octSeq__copyIn(base, &from->value, &to->value);
    }

    return result;
}

c_bool
__DDS_TimeBasedFilterQosPolicy__copyIn(c_base base, void *_from, void *_to)
{
    c_bool result = TRUE;

    DDS_TimeBasedFilterQosPolicy *from = (DDS_TimeBasedFilterQosPolicy *)_from;
    struct _DDS_TimeBasedFilterQosPolicy *to = (struct _DDS_TimeBasedFilterQosPolicy *)_to;

    if (result) {
        result = __DDS_Duration_t__copyIn(base, &from->minimum_separation, &to->minimum_separation);
    }

    return result;
}

c_bool
__DDS_PresentationQosPolicy__copyIn(c_base base, void *_from, void *_to)
{
    c_bool result = TRUE;

    DDS_PresentationQosPolicy *from = (DDS_PresentationQosPolicy *)_from;
    struct _DDS_PresentationQosPolicy *to = (struct _DDS_PresentationQosPolicy *)_to;
    (void) base;

#ifdef OSPL_BOUNDS_CHECK
    if((((c_long)from->access_scope) < 0) && (((c_long)from->access_scope) >= 3) ){
        OS_REPORT (OS_ERROR, "copyIn", 0,"Member 'DDS::PresentationQosPolicy.access_scope' of type 'PresentationQosPolicyAccessScopeKind' is out of range.");
        result = FALSE;
    }
#endif
    if (result) {
        to->access_scope = (enum _DDS_PresentationQosPolicyAccessScopeKind)from->access_scope;
    }
    to->coherent_access = (c_bool)from->coherent_access;
    to->ordered_access = (c_bool)from->ordered_access;

    return result;
}

c_bool
__DDS_DurabilityQosPolicy__copyIn(c_base base, void *_from, void *_to)
{
    c_bool result = TRUE;

    DDS_DurabilityQosPolicy *from = (DDS_DurabilityQosPolicy *)_from;
    struct _DDS_DurabilityQosPolicy *to = (struct _DDS_DurabilityQosPolicy *)_to;
    (void) base;

#ifdef OSPL_BOUNDS_CHECK
    if((((c_long)from->kind) < 0) && (((c_long)from->kind) >= 4) ){
        OS_REPORT (OS_ERROR, "copyIn", 0,"Member 'DDS::DurabilityQosPolicy.kind' of type 'DurabilityQosPolicyKind' is out of range.");
        result = FALSE;
    }
#endif
    if (result) {
        to->kind = (enum _DDS_DurabilityQosPolicyKind)from->kind;
    }

    return result;
}

c_bool
__DDS_LivelinessQosPolicy__copyIn(c_base base, void *_from, void *_to)
{
    c_bool result = TRUE;

    DDS_LivelinessQosPolicy *from = (DDS_LivelinessQosPolicy *)_from;
    struct _DDS_LivelinessQosPolicy *to = (struct _DDS_LivelinessQosPolicy *)_to;

#ifdef OSPL_BOUNDS_CHECK
    if((((c_long)from->kind) < 0) && (((c_long)from->kind) >= 3) ){
        OS_REPORT (OS_ERROR, "copyIn", 0,"Member 'DDS::LivelinessQosPolicy.kind' of type 'LivelinessQosPolicyKind' is out of range.");
        result = FALSE;
    }
#endif
    if (result) {
        to->kind = (enum _DDS_LivelinessQosPolicyKind)from->kind;
    }
    if (result) {
        result = __DDS_Duration_t__copyIn(base, &from->lease_duration, &to->lease_duration);
    }

    return result;
}

c_bool
__DDS_HistoryQosPolicy__copyIn(c_base base, void *_from, void *_to)
{
    c_bool result = TRUE;

    DDS_HistoryQosPolicy *from = (DDS_HistoryQosPolicy *)_from;
    struct _DDS_HistoryQosPolicy *to = (struct _DDS_HistoryQosPolicy *)_to;
    (void) base;

#ifdef OSPL_BOUNDS_CHECK
    if((((c_long)from->kind) < 0) && (((c_long)from->kind) >= 2) ){
        OS_REPORT (OS_ERROR, "copyIn", 0,"Member 'DDS::HistoryQosPolicy.kind' of type 'HistoryQosPolicyKind' is out of range.");
        result = FALSE;
    }
#endif
    if (result) {
        to->kind = (enum _DDS_HistoryQosPolicyKind)from->kind;
    }
    to->depth = (c_long)from->depth;

    return result;
}

c_bool
__DDS_UserDataQosPolicy__copyIn(c_base base, void *_from, void *_to)
{
    c_bool result = TRUE;

    DDS_UserDataQosPolicy *from = (DDS_UserDataQosPolicy *)_from;
    struct _DDS_UserDataQosPolicy *to = (struct _DDS_UserDataQosPolicy *)_to;

    if (result) {
        result = __DDS_octSeq__copyIn(base, &from->value, &to->value);
    }

    return result;
}

c_bool
__DDS_TransportPriorityQosPolicy__copyIn(c_base base, void *_from, void *_to)
{
    DDS_TransportPriorityQosPolicy *from = (DDS_TransportPriorityQosPolicy *)_from;
    struct _DDS_TransportPriorityQosPolicy *to = (struct _DDS_TransportPriorityQosPolicy *)_to;
    (void) base;
    to->value = (c_long)from->value;

    return TRUE;
}

c_bool
__DDS_DestinationOrderQosPolicy__copyIn(c_base base, void *_from, void *_to)
{
    c_bool result = TRUE;

    DDS_DestinationOrderQosPolicy *from = (DDS_DestinationOrderQosPolicy *)_from;
    struct _DDS_DestinationOrderQosPolicy *to = (struct _DDS_DestinationOrderQosPolicy *)_to;
    (void) base;

#ifdef OSPL_BOUNDS_CHECK
    if((((c_long)from->kind) < 0) || (((c_long)from->kind) >= 2) ){
        OS_REPORT (OS_ERROR, "copyIn", 0,"Member 'DDS::DestinationOrderQosPolicy.kind' of type 'DestinationOrderQosPolicyKind' is out of range.");
        result = FALSE;
    }
#endif
    if (result) {
        to->kind = (enum _DDS_DestinationOrderQosPolicyKind)from->kind;
    }

    return result;
}

c_bool
__DDS_DomainParticipantFactoryQos__copyIn(c_base base, void *_from, void *_to)
{
    c_bool result = TRUE;

    DDS_DomainParticipantFactoryQos *from = (DDS_DomainParticipantFactoryQos *)_from;
    struct _DDS_DomainParticipantFactoryQos *to = (struct _DDS_DomainParticipantFactoryQos *)_to;

    if (result) {
        result = __DDS_EntityFactoryQosPolicy__copyIn(base, &from->entity_factory, &to->entity_factory);
    }

    return result;
}

c_bool
__DDS_DeadlineQosPolicy__copyIn(c_base base, void *_from, void *_to)
{
    c_bool result = TRUE;

    DDS_DeadlineQosPolicy *from = (DDS_DeadlineQosPolicy *)_from;
    struct _DDS_DeadlineQosPolicy *to = (struct _DDS_DeadlineQosPolicy *)_to;

    if (result) {
        result = __DDS_Duration_t__copyIn(base, &from->period, &to->period);
    }

    return result;
}

c_bool
__DDS_LatencyBudgetQosPolicy__copyIn(c_base base, void *_from, void *_to)
{
    c_bool result = TRUE;

    DDS_LatencyBudgetQosPolicy *from = (DDS_LatencyBudgetQosPolicy *)_from;
    struct _DDS_LatencyBudgetQosPolicy *to = (struct _DDS_LatencyBudgetQosPolicy *)_to;

    if (result) {
        result = __DDS_Duration_t__copyIn(base, &from->duration, &to->duration);
    }

    return result;
}

c_bool
__DDS_DomainParticipantQos__copyIn(c_base base, void *_from, void *_to)
{
    c_bool result = TRUE;

    DDS_DomainParticipantQos *from = (DDS_DomainParticipantQos *)_from;
    struct _DDS_DomainParticipantQos *to = (struct _DDS_DomainParticipantQos *)_to;

    if(result){
        result = __DDS_UserDataQosPolicy__copyIn(base, &from->user_data, &to->user_data);
    }
    if(result){
        result = __DDS_EntityFactoryQosPolicy__copyIn(base, &from->entity_factory, &to->entity_factory);
    }
    if(result){
        result = __DDS_SchedulingQosPolicy__copyIn(base, &from->watchdog_scheduling, &to->watchdog_scheduling);
    }
    if(result){
        result = __DDS_SchedulingQosPolicy__copyIn(base, &from->listener_scheduling, &to->listener_scheduling);
    }

    return result;
}

c_bool
__DDS_TopicQos__copyIn(c_base base, void *_from, void *_to)
{
    c_bool result = TRUE;

    DDS_TopicQos *from = (DDS_TopicQos *)_from;
    struct _DDS_TopicQos *to = (struct _DDS_TopicQos *)_to;

    if(result){
        result = __DDS_TopicDataQosPolicy__copyIn(base, &from->topic_data, &to->topic_data);
    }
    if(result){
        result = __DDS_DurabilityQosPolicy__copyIn(base, &from->durability, &to->durability);
    }
    if(result){
        result = __DDS_DurabilityServiceQosPolicy__copyIn(base, &from->durability_service, &to->durability_service);
    }
    if(result){
        result = __DDS_DeadlineQosPolicy__copyIn(base, &from->deadline, &to->deadline);
    }
    if(result){
        result = __DDS_LatencyBudgetQosPolicy__copyIn(base, &from->latency_budget, &to->latency_budget);
    }
    if(result){
        result = __DDS_LivelinessQosPolicy__copyIn(base, &from->liveliness, &to->liveliness);
    }
    if(result){
        result = __DDS_ReliabilityQosPolicy__copyIn(base, &from->reliability, &to->reliability);
    }
    if(result){
        result = __DDS_DestinationOrderQosPolicy__copyIn(base, &from->destination_order, &to->destination_order);
    }
    if(result){
        result = __DDS_HistoryQosPolicy__copyIn(base, &from->history, &to->history);
    }
    if(result){
        result = __DDS_ResourceLimitsQosPolicy__copyIn(base, &from->resource_limits, &to->resource_limits);
    }
    if(result){
        result = __DDS_TransportPriorityQosPolicy__copyIn(base, &from->transport_priority, &to->transport_priority);
    }
    if(result){
        result = __DDS_LifespanQosPolicy__copyIn(base, &from->lifespan, &to->lifespan);
    }
    if(result){
        result = __DDS_OwnershipQosPolicy__copyIn(base, &from->ownership, &to->ownership);
    }

    return result;
}

c_bool
__DDS_DataWriterQos__copyIn(c_base base, void *_from, void *_to)
{
    c_bool result = TRUE;

    DDS_DataWriterQos *from = (DDS_DataWriterQos *)_from;
    struct _DDS_DataWriterQos *to = (struct _DDS_DataWriterQos *)_to;

    if(result){
        result = __DDS_DurabilityQosPolicy__copyIn(base, &from->durability, &to->durability);
    }
    if(result){
        result = __DDS_DeadlineQosPolicy__copyIn(base, &from->deadline, &to->deadline);
    }
    if(result){
        result = __DDS_LatencyBudgetQosPolicy__copyIn(base, &from->latency_budget, &to->latency_budget);
    }
    if(result){
        result = __DDS_LivelinessQosPolicy__copyIn(base, &from->liveliness, &to->liveliness);
    }
    if(result){
        result = __DDS_ReliabilityQosPolicy__copyIn(base, &from->reliability, &to->reliability);
    }
    if(result){
        result = __DDS_DestinationOrderQosPolicy__copyIn(base, &from->destination_order, &to->destination_order);
    }
    if(result){
        result = __DDS_HistoryQosPolicy__copyIn(base, &from->history, &to->history);
    }
    if(result){
        result = __DDS_ResourceLimitsQosPolicy__copyIn(base, &from->resource_limits, &to->resource_limits);
    }
    if(result){
        result = __DDS_TransportPriorityQosPolicy__copyIn(base, &from->transport_priority, &to->transport_priority);
    }
    if(result){
        result = __DDS_LifespanQosPolicy__copyIn(base, &from->lifespan, &to->lifespan);
    }
    if(result){
        result = __DDS_UserDataQosPolicy__copyIn(base, &from->user_data, &to->user_data);
    }
    if(result){
        result = __DDS_OwnershipQosPolicy__copyIn(base, &from->ownership, &to->ownership);
    }
    if(result){
        result = __DDS_OwnershipStrengthQosPolicy__copyIn(base, &from->ownership_strength, &to->ownership_strength);
    }
    if(result){
        result = __DDS_WriterDataLifecycleQosPolicy__copyIn(base, &from->writer_data_lifecycle, &to->writer_data_lifecycle);
    }

    return result;
}

c_bool
__DDS_PublisherQos__copyIn(c_base base, void *_from, void *_to)
{
    c_bool result = TRUE;

    DDS_PublisherQos *from = (DDS_PublisherQos *)_from;
    struct _DDS_PublisherQos *to = (struct _DDS_PublisherQos *)_to;

    if(result){
        result = __DDS_PresentationQosPolicy__copyIn(base, &from->presentation, &to->presentation);
    }
    if(result){
        result = __DDS_PartitionQosPolicy__copyIn(base, &from->partition, &to->partition);
    }
    if(result){
        result = __DDS_GroupDataQosPolicy__copyIn(base, &from->group_data, &to->group_data);
    }
    if(result){
        result = __DDS_EntityFactoryQosPolicy__copyIn(base, &from->entity_factory, &to->entity_factory);
    }

    return result;
}

c_bool
__DDS_DataReaderQos__copyIn(c_base base, void *_from, void *_to)
{
    c_bool result = TRUE;

    DDS_DataReaderQos *from = (DDS_DataReaderQos *)_from;
    struct _DDS_DataReaderQos *to = (struct _DDS_DataReaderQos *)_to;

    if(result){
        result = __DDS_DurabilityQosPolicy__copyIn(base, &from->durability, &to->durability);
    }
    if(result){
        result = __DDS_DeadlineQosPolicy__copyIn(base, &from->deadline, &to->deadline);
    }
    if(result){
        result = __DDS_LatencyBudgetQosPolicy__copyIn(base, &from->latency_budget, &to->latency_budget);
    }
    if(result){
        result = __DDS_LivelinessQosPolicy__copyIn(base, &from->liveliness, &to->liveliness);
    }
    if(result){
        result = __DDS_ReliabilityQosPolicy__copyIn(base, &from->reliability, &to->reliability);
    }
    if(result){
        result = __DDS_DestinationOrderQosPolicy__copyIn(base, &from->destination_order, &to->destination_order);
    }
    if(result){
        result = __DDS_HistoryQosPolicy__copyIn(base, &from->history, &to->history);
    }
    if(result){
        result = __DDS_ResourceLimitsQosPolicy__copyIn(base, &from->resource_limits, &to->resource_limits);
    }
    if(result){
        result = __DDS_UserDataQosPolicy__copyIn(base, &from->user_data, &to->user_data);
    }
    if(result){
        result = __DDS_OwnershipQosPolicy__copyIn(base, &from->ownership, &to->ownership);
    }
    if(result){
        result = __DDS_TimeBasedFilterQosPolicy__copyIn(base, &from->time_based_filter, &to->time_based_filter);
    }
    if(result){
        result = __DDS_ReaderDataLifecycleQosPolicy__copyIn(base, &from->reader_data_lifecycle, &to->reader_data_lifecycle);
    }
    if(result){
        result = __DDS_SubscriptionKeyQosPolicy__copyIn(base, &from->subscription_keys, &to->subscription_keys);
    }
    if(result){
        result = __DDS_ReaderLifespanQosPolicy__copyIn(base, &from->reader_lifespan, &to->reader_lifespan);
    }
    if(result){
        result = __DDS_ShareQosPolicy__copyIn(base, &from->share, &to->share);
    }

    return result;
}

c_bool
__DDS_SubscriberQos__copyIn(c_base base, void *_from, void *_to)
{
    c_bool result = TRUE;

    DDS_SubscriberQos *from = (DDS_SubscriberQos *)_from;
    struct _DDS_SubscriberQos *to = (struct _DDS_SubscriberQos *)_to;

    if(result){
        result = __DDS_PresentationQosPolicy__copyIn(base, &from->presentation, &to->presentation);
    }
    if(result){
        result = __DDS_PartitionQosPolicy__copyIn(base, &from->partition, &to->partition);
    }
    if(result){
        result = __DDS_GroupDataQosPolicy__copyIn(base, &from->group_data, &to->group_data);
    }
    if(result){
        result = __DDS_EntityFactoryQosPolicy__copyIn(base, &from->entity_factory, &to->entity_factory);
    }
    if(result){
        result = __DDS_ShareQosPolicy__copyIn(base, &from->share, &to->share);
    }

    return result;
}



void
__DDS_Duration_t__copyOut(void *_from, void *_to)
{
    struct _DDS_Duration_t *from = (struct _DDS_Duration_t *)_from;
    DDS_Duration_t *to = (DDS_Duration_t *)_to;
    to->sec = (DDS_long)from->sec;
    to->nanosec = (DDS_unsigned_long)from->nanosec;
    
    return;
}

void
__DDS_Time_t__copyOut(void *_from, void *_to)
{
    struct _DDS_Time_t *from = (struct _DDS_Time_t *)_from;
    DDS_Time_t *to = (DDS_Time_t *)_to;
    to->sec = (DDS_long)from->sec;
    to->nanosec = (DDS_unsigned_long)from->nanosec;
    
    return;
}

void
__DDS_octSeq__copyOut(void *_from, void *_to)
{
    _DDS_octSeq *from = (_DDS_octSeq *)_from;
    DDS_octSeq *to = (DDS_octSeq *)_to;
    long size0;
    c_octet *src0 = (c_octet *)(*from);
    DDS_octSeq *dst0 = to;

    size0 = c_arraySize((c_sequence)(src0));
    DDS_sequence_replacebuf (dst0, (bufferAllocatorType)DDS_sequence_octet_allocbuf, size0);
    dst0->_length = size0;
    memcpy ((*dst0)._buffer,src0,size0* sizeof(*((*dst0)._buffer)));
}

void
__DDS_BuiltinTopicKey_t__copyOut(void *_from, void *_to)
{
    _DDS_BuiltinTopicKey_t *from = (_DDS_BuiltinTopicKey_t *)_from;
    DDS_BuiltinTopicKey_t *to = (DDS_BuiltinTopicKey_t *)_to;
    memcpy (*to, from, sizeof (*from));
}

void
__DDS_StringSeq__copyOut(void *_from, void *_to)
{
    _DDS_StringSeq *from = (_DDS_StringSeq *)_from;
    DDS_StringSeq *to = (DDS_StringSeq *)_to;
    long size0;
    c_string *src0 = (c_string *)(*from);
    DDS_StringSeq *dst0 = to;

    size0 = c_arraySize((c_sequence)(src0));
    DDS_sequence_replacebuf (dst0, (bufferAllocatorType)DDS_sequence_string_allocbuf, size0);
    dst0->_length = size0;
    {
        long i0;
        for (i0 = 0; i0 < size0; i0++) {
            DDS_string_replace (src0[i0] ? src0[i0] : "", &(*dst0)._buffer[i0]);
        }
    }
}

void
__DDS_EntityFactoryQosPolicy__copyOut(void *_from, void *_to)
{
    struct _DDS_EntityFactoryQosPolicy *from = (struct _DDS_EntityFactoryQosPolicy *)_from;
    DDS_EntityFactoryQosPolicy *to = (DDS_EntityFactoryQosPolicy *)_to;
    to->autoenable_created_entities = (DDS_boolean)from->autoenable_created_entities;
}

void
__DDS_WriterDataLifecycleQosPolicy__copyOut(void *_from, void *_to)
{
    struct _DDS_WriterDataLifecycleQosPolicy *from = (struct _DDS_WriterDataLifecycleQosPolicy *)_from;
    DDS_WriterDataLifecycleQosPolicy *to = (DDS_WriterDataLifecycleQosPolicy *)_to;
    to->autodispose_unregistered_instances = (DDS_boolean)from->autodispose_unregistered_instances;
    __DDS_Duration_t__copyOut(&from->autopurge_suspended_samples_delay, &to->autopurge_suspended_samples_delay);
    __DDS_Duration_t__copyOut(&from->autounregister_instance_delay, &to->autounregister_instance_delay);
}

void
__DDS_InvalidSampleVisibilityQosPolicy__copyOut(void *_from, void *_to)
{
    struct _DDS_InvalidSampleVisibilityQosPolicy *from = (struct _DDS_InvalidSampleVisibilityQosPolicy *)_from;
    DDS_InvalidSampleVisibilityQosPolicy *to = (DDS_InvalidSampleVisibilityQosPolicy *)_to;
    to->kind = (DDS_InvalidSampleVisibilityQosPolicyKind)from->kind;
}

void
__DDS_ReaderDataLifecycleQosPolicy__copyOut(void *_from, void *_to)
{
    struct _DDS_ReaderDataLifecycleQosPolicy *from = (struct _DDS_ReaderDataLifecycleQosPolicy *)_from;
    DDS_ReaderDataLifecycleQosPolicy *to = (DDS_ReaderDataLifecycleQosPolicy *)_to;
    to->enable_invalid_samples = (DDS_boolean)from->enable_invalid_samples;
    __DDS_Duration_t__copyOut(&from->autopurge_nowriter_samples_delay, &to->autopurge_nowriter_samples_delay);
    __DDS_Duration_t__copyOut(&from->autopurge_disposed_samples_delay, &to->autopurge_disposed_samples_delay);
    __DDS_InvalidSampleVisibilityQosPolicy__copyOut(&from->invalid_sample_visibility, &to->invalid_sample_visibility);
}

void
__DDS_SubscriptionKeyQosPolicy__copyOut(void *_from, void *_to)
{
    struct _DDS_SubscriptionKeyQosPolicy *from = (struct _DDS_SubscriptionKeyQosPolicy *)_from;
    DDS_SubscriptionKeyQosPolicy *to = (DDS_SubscriptionKeyQosPolicy *)_to;
    to->use_key_list = (DDS_boolean)from->use_key_list;
    __DDS_StringSeq__copyOut(&from->key_list, &to->key_list);
}

void
__DDS_ReaderLifespanQosPolicy__copyOut(void *_from, void *_to)
{
    struct _DDS_ReaderLifespanQosPolicy *from = (struct _DDS_ReaderLifespanQosPolicy *)_from;
    DDS_ReaderLifespanQosPolicy *to = (DDS_ReaderLifespanQosPolicy *)_to;
    to->use_lifespan = (DDS_boolean)from->use_lifespan;
    __DDS_Duration_t__copyOut(&from->duration, &to->duration);
}

void
__DDS_ShareQosPolicy__copyOut(void *_from, void *_to)
{
    struct _DDS_ShareQosPolicy *from = (struct _DDS_ShareQosPolicy *)_from;
    DDS_ShareQosPolicy *to = (DDS_ShareQosPolicy *)_to;
    DDS_string_replace (from->name ? from->name : "", &to->name);
    to->enable = (DDS_boolean)from->enable;
}

void
__DDS_SchedulingClassQosPolicy__copyOut(void *_from, void *_to)
{
    struct _DDS_SchedulingClassQosPolicy *from = (struct _DDS_SchedulingClassQosPolicy *)_from;
    DDS_SchedulingClassQosPolicy *to = (DDS_SchedulingClassQosPolicy *)_to;
    to->kind = (DDS_SchedulingClassQosPolicyKind)from->kind;
}

void
__DDS_SchedulingPriorityQosPolicy__copyOut(void *_from, void *_to)
{
    struct _DDS_SchedulingPriorityQosPolicy *from = (struct _DDS_SchedulingPriorityQosPolicy *)_from;
    DDS_SchedulingPriorityQosPolicy *to = (DDS_SchedulingPriorityQosPolicy *)_to;
    to->kind = (DDS_SchedulingPriorityQosPolicyKind)from->kind;
}

void
__DDS_SchedulingQosPolicy__copyOut(void *_from, void *_to)
{
    struct _DDS_SchedulingQosPolicy *from = (struct _DDS_SchedulingQosPolicy *)_from;
    DDS_SchedulingQosPolicy *to = (DDS_SchedulingQosPolicy *)_to;
    to->scheduling_priority = (DDS_long)from->scheduling_priority;
    __DDS_SchedulingClassQosPolicy__copyOut(&from->scheduling_class, &to->scheduling_class);
    __DDS_SchedulingPriorityQosPolicy__copyOut(&from->scheduling_priority_kind, &to->scheduling_priority_kind);
}

void
__DDS_ViewKeyQosPolicy__copyOut(void *_from, void *_to)
{
    struct _DDS_ViewKeyQosPolicy *from = (struct _DDS_ViewKeyQosPolicy *)_from;
    DDS_ViewKeyQosPolicy *to = (DDS_ViewKeyQosPolicy *)_to;
    to->use_key_list = (DDS_boolean)from->use_key_list;
    __DDS_StringSeq__copyOut(&from->key_list, &to->key_list);
}

void
__DDS_DataReaderViewQos__copyOut(void *_from, void *_to)
{
    struct _DDS_DataReaderViewQos *from = (struct _DDS_DataReaderViewQos *)_from;
    DDS_DataReaderViewQos *to = (DDS_DataReaderViewQos *)_to;
    __DDS_ViewKeyQosPolicy__copyOut(&from->view_keys, &to->view_keys);
}

void
__DDS_DomainParticipantFactoryQos__copyOut(void *_from, void *_to)
{
    struct _DDS_DomainParticipantFactoryQos *from = (struct _DDS_DomainParticipantFactoryQos *)_from;
    DDS_DomainParticipantFactoryQos *to = (DDS_DomainParticipantFactoryQos *)_to;
    __DDS_EntityFactoryQosPolicy__copyOut(&from->entity_factory, &to->entity_factory);
}

void
__DDS_OwnershipStrengthQosPolicy__copyOut(void *_from, void *_to)
{
    struct _DDS_OwnershipStrengthQosPolicy *from = (struct _DDS_OwnershipStrengthQosPolicy *)_from;
    DDS_OwnershipStrengthQosPolicy *to = (DDS_OwnershipStrengthQosPolicy *)_to;
    to->value = (DDS_long)from->value;
}

void
__DDS_DurabilityServiceQosPolicy__copyOut(void *_from, void *_to)
{
    struct _DDS_DurabilityServiceQosPolicy *from = (struct _DDS_DurabilityServiceQosPolicy *)_from;
    DDS_DurabilityServiceQosPolicy *to = (DDS_DurabilityServiceQosPolicy *)_to;

    to->history_kind = (DDS_HistoryQosPolicyKind)from->history_kind;
    to->history_depth = (DDS_long)from->history_depth;
    to->max_samples = (DDS_long)from->max_samples;
    to->max_instances = (DDS_long)from->max_instances;
    to->max_samples_per_instance = (DDS_long)from->max_samples_per_instance;
    __DDS_Duration_t__copyOut(&from->service_cleanup_delay, &to->service_cleanup_delay);
}

void
__DDS_PresentationQosPolicy__copyOut(void *_from, void *_to)
{
    struct _DDS_PresentationQosPolicy *from = (struct _DDS_PresentationQosPolicy *)_from;
    DDS_PresentationQosPolicy *to = (DDS_PresentationQosPolicy *)_to;
    to->access_scope = (DDS_PresentationQosPolicyAccessScopeKind)from->access_scope;
    to->coherent_access = (DDS_boolean)from->coherent_access;
    to->ordered_access = (DDS_boolean)from->ordered_access;
}

void
__DDS_TransportPriorityQosPolicy__copyOut(void *_from, void *_to)
{
    struct _DDS_TransportPriorityQosPolicy *from = (struct _DDS_TransportPriorityQosPolicy *)_from;
    DDS_TransportPriorityQosPolicy *to = (DDS_TransportPriorityQosPolicy *)_to;
    to->value = (DDS_long)from->value;
}

void
__DDS_LatencyBudgetQosPolicy__copyOut(void *_from, void *_to)
{
    struct _DDS_LatencyBudgetQosPolicy *from = (struct _DDS_LatencyBudgetQosPolicy *)_from;
    DDS_LatencyBudgetQosPolicy *to = (DDS_LatencyBudgetQosPolicy *)_to;

    __DDS_Duration_t__copyOut(&from->duration, &to->duration);
}

void
__DDS_GroupDataQosPolicy__copyOut(void *_from, void *_to)
{
    struct _DDS_GroupDataQosPolicy *from = (struct _DDS_GroupDataQosPolicy *)_from;
    DDS_GroupDataQosPolicy *to = (DDS_GroupDataQosPolicy *)_to;
    __DDS_octSeq__copyOut(&from->value, &to->value);
}

void
__DDS_ResourceLimitsQosPolicy__copyOut(void *_from, void *_to)
{
    struct _DDS_ResourceLimitsQosPolicy *from = (struct _DDS_ResourceLimitsQosPolicy *)_from;
    DDS_ResourceLimitsQosPolicy *to = (DDS_ResourceLimitsQosPolicy *)_to;
    to->max_samples = (DDS_long)from->max_samples;
    to->max_instances = (DDS_long)from->max_instances;
    to->max_samples_per_instance = (DDS_long)from->max_samples_per_instance;
}

void
__DDS_DurabilityQosPolicy__copyOut(void *_from, void *_to)
{
    struct _DDS_DurabilityQosPolicy *from = (struct _DDS_DurabilityQosPolicy *)_from;
    DDS_DurabilityQosPolicy *to = (DDS_DurabilityQosPolicy *)_to;
    to->kind = (DDS_DurabilityQosPolicyKind)from->kind;
}

void
__DDS_HistoryQosPolicy__copyOut(void *_from, void *_to)
{
    struct _DDS_HistoryQosPolicy *from = (struct _DDS_HistoryQosPolicy *)_from;
    DDS_HistoryQosPolicy *to = (DDS_HistoryQosPolicy *)_to;
    to->kind = (DDS_HistoryQosPolicyKind)from->kind;
    to->depth = (DDS_long)from->depth;
}

void
__DDS_UserDataQosPolicy__copyOut(void *_from, void *_to)
{
    struct _DDS_UserDataQosPolicy *from = (struct _DDS_UserDataQosPolicy *)_from;
    DDS_UserDataQosPolicy *to = (DDS_UserDataQosPolicy *)_to;
    __DDS_octSeq__copyOut(&from->value, &to->value);
}

void
__DDS_LifespanQosPolicy__copyOut(void *_from, void *_to)
{
    struct _DDS_LifespanQosPolicy *from = (struct _DDS_LifespanQosPolicy *)_from;
    DDS_LifespanQosPolicy *to = (DDS_LifespanQosPolicy *)_to;
    __DDS_Duration_t__copyOut(&from->duration, &to->duration);
}

void
__DDS_TopicDataQosPolicy__copyOut(void *_from, void *_to)
{
    struct _DDS_TopicDataQosPolicy *from = (struct _DDS_TopicDataQosPolicy *)_from;
    DDS_TopicDataQosPolicy *to = (DDS_TopicDataQosPolicy *)_to;
    __DDS_octSeq__copyOut(&from->value, &to->value);
}

void
__DDS_ReliabilityQosPolicy__copyOut(void *_from, void *_to)
{
    struct _DDS_ReliabilityQosPolicy *from = (struct _DDS_ReliabilityQosPolicy *)_from;
    DDS_ReliabilityQosPolicy *to = (DDS_ReliabilityQosPolicy *)_to;
    to->kind = (DDS_ReliabilityQosPolicyKind)from->kind;
    to->synchronous = (DDS_boolean)from->synchronous;
    __DDS_Duration_t__copyOut(&from->max_blocking_time, &to->max_blocking_time);
}

void
__DDS_DeadlineQosPolicy__copyOut(void *_from, void *_to)
{
    struct _DDS_DeadlineQosPolicy *from = (struct _DDS_DeadlineQosPolicy *)_from;
    DDS_DeadlineQosPolicy *to = (DDS_DeadlineQosPolicy *)_to;
    __DDS_Duration_t__copyOut(&from->period, &to->period);
}

void
__DDS_LivelinessQosPolicy__copyOut(void *_from, void *_to)
{
    struct _DDS_LivelinessQosPolicy *from = (struct _DDS_LivelinessQosPolicy *)_from;
    DDS_LivelinessQosPolicy *to = (DDS_LivelinessQosPolicy *)_to;
    to->kind = (DDS_LivelinessQosPolicyKind)from->kind;
    __DDS_Duration_t__copyOut(&from->lease_duration, &to->lease_duration);
}

void
__DDS_PartitionQosPolicy__copyOut(void *_from, void *_to)
{
    struct _DDS_PartitionQosPolicy *from = (struct _DDS_PartitionQosPolicy *)_from;
    DDS_PartitionQosPolicy *to = (DDS_PartitionQosPolicy *)_to;
    __DDS_StringSeq__copyOut(&from->name, &to->name);
}

void
__DDS_TimeBasedFilterQosPolicy__copyOut(void *_from, void *_to)
{
    struct _DDS_TimeBasedFilterQosPolicy *from = (struct _DDS_TimeBasedFilterQosPolicy *)_from;
    DDS_TimeBasedFilterQosPolicy *to = (DDS_TimeBasedFilterQosPolicy *)_to;
    __DDS_Duration_t__copyOut(&from->minimum_separation, &to->minimum_separation);
}

void
__DDS_DestinationOrderQosPolicy__copyOut(void *_from, void *_to)
{
    struct _DDS_DestinationOrderQosPolicy *from = (struct _DDS_DestinationOrderQosPolicy *)_from;
    DDS_DestinationOrderQosPolicy *to = (DDS_DestinationOrderQosPolicy *)_to;
    to->kind = (DDS_DestinationOrderQosPolicyKind)from->kind;
}

void
__DDS_OwnershipQosPolicy__copyOut(void *_from, void *_to)
{
    struct _DDS_OwnershipQosPolicy *from = (struct _DDS_OwnershipQosPolicy *)_from;
    DDS_OwnershipQosPolicy *to = (DDS_OwnershipQosPolicy *)_to;
    to->kind = (DDS_OwnershipQosPolicyKind)from->kind;
}

void
__DDS_DomainParticipantQos__copyOut(void *_from, void *_to)
{
    struct _DDS_DomainParticipantQos *from = (struct _DDS_DomainParticipantQos *)_from;
    DDS_DomainParticipantQos *to = (DDS_DomainParticipantQos *)_to;
    __DDS_UserDataQosPolicy__copyOut(&from->user_data, &to->user_data);
    __DDS_EntityFactoryQosPolicy__copyOut(&from->entity_factory, &to->entity_factory);
    __DDS_SchedulingQosPolicy__copyOut(&from->watchdog_scheduling, &to->watchdog_scheduling);
    __DDS_SchedulingQosPolicy__copyOut(&from->listener_scheduling, &to->listener_scheduling);
}

void
__DDS_TopicQos__copyOut(void *_from, void *_to)
{
    struct _DDS_TopicQos *from = (struct _DDS_TopicQos *)_from;
    DDS_TopicQos *to = (DDS_TopicQos *)_to;
    __DDS_TopicDataQosPolicy__copyOut(&from->topic_data, &to->topic_data);
    __DDS_DurabilityQosPolicy__copyOut(&from->durability, &to->durability);
    __DDS_DurabilityServiceQosPolicy__copyOut(&from->durability_service, &to->durability_service);
    __DDS_DeadlineQosPolicy__copyOut(&from->deadline, &to->deadline);
    __DDS_LatencyBudgetQosPolicy__copyOut(&from->latency_budget, &to->latency_budget);
    __DDS_LivelinessQosPolicy__copyOut(&from->liveliness, &to->liveliness);
    __DDS_ReliabilityQosPolicy__copyOut(&from->reliability, &to->reliability);
    __DDS_DestinationOrderQosPolicy__copyOut(&from->destination_order, &to->destination_order);
    __DDS_HistoryQosPolicy__copyOut(&from->history, &to->history);
    __DDS_ResourceLimitsQosPolicy__copyOut(&from->resource_limits, &to->resource_limits);
    __DDS_TransportPriorityQosPolicy__copyOut(&from->transport_priority, &to->transport_priority);
    __DDS_LifespanQosPolicy__copyOut(&from->lifespan, &to->lifespan);
    __DDS_OwnershipQosPolicy__copyOut(&from->ownership, &to->ownership);
}

void
__DDS_DataWriterQos__copyOut(void *_from, void *_to)
{
    struct _DDS_DataWriterQos *from = (struct _DDS_DataWriterQos *)_from;
    DDS_DataWriterQos *to = (DDS_DataWriterQos *)_to;
    __DDS_DurabilityQosPolicy__copyOut(&from->durability, &to->durability);
    __DDS_DeadlineQosPolicy__copyOut(&from->deadline, &to->deadline);
    __DDS_LatencyBudgetQosPolicy__copyOut(&from->latency_budget, &to->latency_budget);
    __DDS_LivelinessQosPolicy__copyOut(&from->liveliness, &to->liveliness);
    __DDS_ReliabilityQosPolicy__copyOut(&from->reliability, &to->reliability);
    __DDS_DestinationOrderQosPolicy__copyOut(&from->destination_order, &to->destination_order);
    __DDS_HistoryQosPolicy__copyOut(&from->history, &to->history);
    __DDS_ResourceLimitsQosPolicy__copyOut(&from->resource_limits, &to->resource_limits);
    __DDS_TransportPriorityQosPolicy__copyOut(&from->transport_priority, &to->transport_priority);
    __DDS_LifespanQosPolicy__copyOut(&from->lifespan, &to->lifespan);
    __DDS_UserDataQosPolicy__copyOut(&from->user_data, &to->user_data);
    __DDS_OwnershipQosPolicy__copyOut(&from->ownership, &to->ownership);
    __DDS_OwnershipStrengthQosPolicy__copyOut(&from->ownership_strength, &to->ownership_strength);
    __DDS_WriterDataLifecycleQosPolicy__copyOut(&from->writer_data_lifecycle, &to->writer_data_lifecycle);
}

void
__DDS_PublisherQos__copyOut(void *_from, void *_to)
{
    struct _DDS_PublisherQos *from = (struct _DDS_PublisherQos *)_from;
    DDS_PublisherQos *to = (DDS_PublisherQos *)_to;
    __DDS_PresentationQosPolicy__copyOut(&from->presentation, &to->presentation);
    __DDS_PartitionQosPolicy__copyOut(&from->partition, &to->partition);
    __DDS_GroupDataQosPolicy__copyOut(&from->group_data, &to->group_data);
    __DDS_EntityFactoryQosPolicy__copyOut(&from->entity_factory, &to->entity_factory);
}

void
__DDS_DataReaderQos__copyOut(void *_from, void *_to)
{
    struct _DDS_DataReaderQos *from = (struct _DDS_DataReaderQos *)_from;
    DDS_DataReaderQos *to = (DDS_DataReaderQos *)_to;
    __DDS_DurabilityQosPolicy__copyOut(&from->durability, &to->durability);
    __DDS_DeadlineQosPolicy__copyOut(&from->deadline, &to->deadline);
    __DDS_LatencyBudgetQosPolicy__copyOut(&from->latency_budget, &to->latency_budget);
    __DDS_LivelinessQosPolicy__copyOut(&from->liveliness, &to->liveliness);
    __DDS_ReliabilityQosPolicy__copyOut(&from->reliability, &to->reliability);
    __DDS_DestinationOrderQosPolicy__copyOut(&from->destination_order, &to->destination_order);
    __DDS_HistoryQosPolicy__copyOut(&from->history, &to->history);
    __DDS_ResourceLimitsQosPolicy__copyOut(&from->resource_limits, &to->resource_limits);
    __DDS_UserDataQosPolicy__copyOut(&from->user_data, &to->user_data);
    __DDS_OwnershipQosPolicy__copyOut(&from->ownership, &to->ownership);
    __DDS_TimeBasedFilterQosPolicy__copyOut(&from->time_based_filter, &to->time_based_filter);
    __DDS_ReaderDataLifecycleQosPolicy__copyOut(&from->reader_data_lifecycle, &to->reader_data_lifecycle);
    __DDS_SubscriptionKeyQosPolicy__copyOut(&from->subscription_keys, &to->subscription_keys);
    __DDS_ReaderLifespanQosPolicy__copyOut(&from->reader_lifespan, &to->reader_lifespan);
    __DDS_ShareQosPolicy__copyOut(&from->share, &to->share);
}

void
__DDS_SubscriberQos__copyOut(void *_from, void *_to)
{
    struct _DDS_SubscriberQos *from = (struct _DDS_SubscriberQos *)_from;
    DDS_SubscriberQos *to = (DDS_SubscriberQos *)_to;
    __DDS_PresentationQosPolicy__copyOut(&from->presentation, &to->presentation);
    __DDS_PartitionQosPolicy__copyOut(&from->partition, &to->partition);
    __DDS_GroupDataQosPolicy__copyOut(&from->group_data, &to->group_data);
    __DDS_EntityFactoryQosPolicy__copyOut(&from->entity_factory, &to->entity_factory);
    __DDS_ShareQosPolicy__copyOut(&from->share, &to->share);
}


void
__DDS_NamedDomainParticipantQos__copyOut(void *_from, void *_to)
{
    struct _DDS_NamedDomainParticipantQos *from = (struct _DDS_NamedDomainParticipantQos *)_from;
    DDS_NamedDomainParticipantQos *to = (DDS_NamedDomainParticipantQos *)_to;
    DDS_string_replace (from->name, &to->name);
    __DDS_DomainParticipantQos__copyOut(&from->domainparticipant_qos, &to->participant_qos);
}

void
__DDS_NamedPublisherQos__copyOut(void *_from, void *_to)
{
    struct _DDS_NamedPublisherQos *from = (struct _DDS_NamedPublisherQos *)_from;
    DDS_NamedPublisherQos *to = (DDS_NamedPublisherQos *)_to;
    DDS_string_replace (from->name, &to->name);
    __DDS_PublisherQos__copyOut(&from->publisher_qos, &to->publisher_qos);
}

void
__DDS_NamedSubscriberQos__copyOut(void *_from, void *_to)
{
    struct _DDS_NamedSubscriberQos *from = (struct _DDS_NamedSubscriberQos *)_from;
    DDS_NamedSubscriberQos *to = (DDS_NamedSubscriberQos *)_to;
    DDS_string_replace (from->name, &to->name);
    __DDS_SubscriberQos__copyOut(&from->subscriber_qos, &to->subscriber_qos);

}

void
__DDS_NamedTopicQos__copyOut(void *_from, void *_to)
{
    struct _DDS_NamedTopicQos *from = (struct _DDS_NamedTopicQos *)_from;
    DDS_NamedTopicQos *to = (DDS_NamedTopicQos *)_to;
    DDS_string_replace (from->name, &to->name);
    __DDS_TopicQos__copyOut(&from->topic_qos, &to->topic_qos);
}

void
__DDS_NamedDataWriterQos__copyOut(void *_from, void *_to)
{
    struct _DDS_NamedDataWriterQos *from = (struct _DDS_NamedDataWriterQos *)_from;
    DDS_NamedDataWriterQos *to = (DDS_NamedDataWriterQos *)_to;
    DDS_string_replace (from->name, &to->name);
    __DDS_DataWriterQos__copyOut(&from->datawriter_qos, &to->datawriter_qos);
}

void
__DDS_NamedDataReaderQos__copyOut(void *_from, void *_to)
{
    struct _DDS_NamedDataReaderQos *from = (struct _DDS_NamedDataReaderQos *)_from;
    DDS_NamedDataReaderQos *to = (DDS_NamedDataReaderQos *)_to;
    DDS_string_replace (from->name, &to->name);
    __DDS_DataReaderQos__copyOut(&from->datareader_qos, &to->datareader_qos);
}
