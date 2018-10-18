#ifndef DDS_QOS_H
#define DDS_QOS_H

/** @file qos.h
 *  @brief Vortex Lite QoS header
 */

#include <stdbool.h>
#include <dds/restrict.h>
#include "os_if.h"

#if defined (__cplusplus)
extern "C" {
#endif

#undef DDS_EXPORT
#ifdef OSPL_BUILD_DCPSC99
#define DDS_EXPORT OS_API_EXPORT
#else
#define DDS_EXPORT OS_API_IMPORT
#endif

/* QoS identifiers */
/** @name QoS identifiers
  @{**/
#define DDS_INVALID_QOS_POLICY_ID 0
#define DDS_USERDATA_QOS_POLICY_ID 1
#define DDS_DURABILITY_QOS_POLICY_ID 2
#define DDS_PRESENTATION_QOS_POLICY_ID 3
#define DDS_DEADLINE_QOS_POLICY_ID 4
#define DDS_LATENCYBUDGET_QOS_POLICY_ID 5
#define DDS_OWNERSHIP_QOS_POLICY_ID 6
#define DDS_OWNERSHIPSTRENGTH_QOS_POLICY_ID 7
#define DDS_LIVELINESS_QOS_POLICY_ID 8
#define DDS_TIMEBASEDFILTER_QOS_POLICY_ID 9
#define DDS_PARTITION_QOS_POLICY_ID 10
#define DDS_RELIABILITY_QOS_POLICY_ID 11
#define DDS_DESTINATIONORDER_QOS_POLICY_ID 12
#define DDS_HISTORY_QOS_POLICY_ID 13
#define DDS_RESOURCELIMITS_QOS_POLICY_ID 14
#define DDS_ENTITYFACTORY_QOS_POLICY_ID 15
#define DDS_WRITERDATALIFECYCLE_QOS_POLICY_ID 16
#define DDS_READERDATALIFECYCLE_QOS_POLICY_ID 17
#define DDS_TOPICDATA_QOS_POLICY_ID 18
#define DDS_GROUPDATA_QOS_POLICY_ID 19
#define DDS_TRANSPORTPRIORITY_QOS_POLICY_ID 20
#define DDS_LIFESPAN_QOS_POLICY_ID 21
#define DDS_DURABILITYSERVICE_QOS_POLICY_ID 22
/** @}*/


/* QoS structure is opaque */
/** QoS structure */
typedef struct nn_xqos dds_qos_t;

/* Durability QoS: Applies to Topic, DataReader, DataWriter */
/**
 * \ref DCPS_QoS_Durability
 */
typedef enum dds_durability_kind
{
  DDS_DURABILITY_VOLATILE,
  DDS_DURABILITY_TRANSIENT_LOCAL,
  DDS_DURABILITY_TRANSIENT,
  DDS_DURABILITY_PERSISTENT
}
dds_durability_kind_t;

/* History QoS: Applies to Topic, DataReader, DataWriter */
/**
 * \ref DCPS_QoS_History
 */
typedef enum dds_history_kind
{
  DDS_HISTORY_KEEP_LAST,
  DDS_HISTORY_KEEP_ALL
}
dds_history_kind_t;

/* Ownership QoS: Applies to Topic, DataReader, DataWriter */
/**
 * \ref DCPS_QoS_Ownership
 */
typedef enum dds_ownership_kind
{
  DDS_OWNERSHIP_SHARED,
  DDS_OWNERSHIP_EXCLUSIVE
}
dds_ownership_kind_t;

/* Liveliness QoS: Applies to Topic, DataReader, DataWriter */
/**
 * \ref DCPS_QoS_Liveliness
 */
typedef enum dds_liveliness_kind
{
  DDS_LIVELINESS_AUTOMATIC,
  DDS_LIVELINESS_MANUAL_BY_PARTICIPANT,
  DDS_LIVELINESS_MANUAL_BY_TOPIC
}
dds_liveliness_kind_t;

/* Reliability QoS: Applies to Topic, DataReader, DataWriter */
/**
 * \ref DCPS_QoS_Reliability
 */
typedef enum dds_reliability_kind
{
  DDS_RELIABILITY_BEST_EFFORT,
  DDS_RELIABILITY_RELIABLE
}
dds_reliability_kind_t;

/* DestinationOrder QoS: Applies to Topic, DataReader, DataWriter */
/**
 * \ref DCPS_QoS_DestinationOrder
 */
typedef enum dds_destination_order_kind
{
  DDS_DESTINATIONORDER_BY_RECEPTION_TIMESTAMP,
  DDS_DESTINATIONORDER_BY_SOURCE_TIMESTAMP
}
dds_destination_order_kind_t;

/* History QoS: Applies to Topic, DataReader, DataWriter */
/**
 * \ref DCPS_QoS_History
 */
typedef struct dds_history_qospolicy
{
  dds_history_kind_t kind;
  int32_t depth;
}
dds_history_qospolicy_t;

/* ResourceLimits QoS: Applies to Topic, DataReader, DataWriter */
/**
 * \ref DCPS_QoS_ResourceLimits
 */
typedef struct dds_resource_limits_qospolicy
{
  int32_t max_samples;
  int32_t max_instances;
  int32_t max_samples_per_instance;
}
dds_resource_limits_qospolicy_t;

/* Presentation QoS: Applies to Publisher, Subscriber */
/**
 * \ref DCPS_QoS_Presentation
 */
typedef enum dds_presentation_access_scope_kind
{
  DDS_PRESENTATION_INSTANCE,
  DDS_PRESENTATION_TOPIC,
  DDS_PRESENTATION_GROUP
}
dds_presentation_access_scope_kind_t;

/**
 * Description : Allocate memory and initializes to default values for qos
 *
 * Arguments :
 *   -# Returns a pointer to the allocated memory for dds_qos_t  structure, 0 if unsuccessful
 */

DDS_EXPORT dds_qos_t * dds_qos_create (void);

/**
 * Description : Delete the memory allocated to qos structure
 *
 * Arguments :
 *   -# qos pointer to the structure
 */
DDS_EXPORT void dds_qos_delete (dds_qos_t * restrict qos);

/**
 * Description : This operation results in resetting the qos structure contents to 0
 *
 * Arguments :
 *   -# qos pointer to the structure
 */
DDS_EXPORT void dds_qos_reset (dds_qos_t * restrict qos);

/**
 * Description : Copy the qos policies from source to destination
 *
 * Arguments :
 *   -# dst The pointer to the destination qos structure, where the content is to copied
 *   -# src The pointer to the source qos structure to be copied
 */
DDS_EXPORT void dds_qos_copy (dds_qos_t * restrict dst, const dds_qos_t * restrict src);

/**
 * Description : Copy the qos policies from source to destination, unless already set
 *
 * Arguments :
 *   -# dst The pointer to the destination qos structure, where the content is merged
 *   -# src The pointer to the source qos structure to be copied
 */
DDS_EXPORT void dds_qos_merge (dds_qos_t * restrict dst, const dds_qos_t * restrict src);

/**
 * Description : Retrieves the default value of the domain participant qos
 *
 * Arguments :
 *   -# qos pointer that contains default values of the policies for participant
 */
DDS_EXPORT void dds_get_default_participant_qos (dds_qos_t * restrict qos);

/**
 * Description : Retrieves the default value of the topic qos
 *
 * Arguments :
 *   -# qos pointer that contains default values of the policies for topic
 */
DDS_EXPORT void dds_get_default_topic_qos (dds_qos_t * restrict qos);

/**
 * Description : Retrieves the default value of the publisher qos
 *
 * Arguments :
 *   -# qos pointer that contains default values of the policies for publisher
 */
DDS_EXPORT void dds_get_default_publisher_qos (dds_qos_t * restrict qos);

/**
 * Description : Retrieves the default value of the subscriber qos
 *
 * Arguments :
 *   -# qos pointer that contains default values of the policies for subscriber
 */
DDS_EXPORT void dds_get_default_subscriber_qos (dds_qos_t * restrict qos);

/**
 * Description : Retrieves the default value of the data writer qos
 *
 * Arguments :
 *   -# qos pointer that contains default values of the policies for data writer
 */
DDS_EXPORT void dds_get_default_writer_qos (dds_qos_t * restrict qos);

/**
 * Description : Retrieves the default value of the data reader qos
 *
 * Arguments :
 *   -# qos pointer that contains default values of the policies for data reader
 */
DDS_EXPORT void dds_get_default_reader_qos (dds_qos_t * restrict qos);

/* return values represent the Error codes if set call is unsuccessful */

/**
 * Description : Set the userdata policy in the qos structure. This value will be validated and applied
 * at the time of entity creation.
 *
 * Arguments :
 *   -# qos The pointer to the qos structure, where the userdata will be applied
 *   -# value content of the user data
 *   -# sz size of the value passed in
 */
DDS_EXPORT void dds_qset_userdata (dds_qos_t * restrict qos, const void * restrict value, size_t sz);

/**
 * Description : Set the topicdata policy in the qos structure. This value will be validated and applied
 * at the time of entity creation.
 *
 * Arguments :
 *   -# qos The pointer to the qos structure, where the topicdata will be applied
 *   -# value content of the topic data
 *   -# sz size of the value passed in
 */
DDS_EXPORT void dds_qset_topicdata (dds_qos_t * restrict qos, const void * restrict value, size_t sz);

/**
 * Description : Set the groupdata policy in the qos structure. This value will be validated and applied
 * at the time of entity creation.
 *
 * Arguments :
 *   -# qos The pointer to the qos structure, where the groupdata will be applied
 *   -# value content of the group data
 *   -# sz size of the value passed in
 */
DDS_EXPORT void dds_qset_groupdata (dds_qos_t * restrict qos, const void * restrict value, size_t sz);

/**
 * Description : Set the durability policy in the qos structure. This value will be validated and applied
 * at the time of entity creation.
 *
 * Arguments :
 *   -# qos The pointer to the qos structure, where the durability will be applied
 *   -# kind \ref DCPS_QoS_Durability
 */
DDS_EXPORT void dds_qset_durability (dds_qos_t *qos, dds_durability_kind_t kind);

/**
 * Description : Set the history policy in the qos structure. This value will be validated and applied
 * at the time of entity creation.
 *
 * Arguments :
 *   -# qos The pointer to the qos structure, where the history will be applied
 *   -# kind, depth \ref DCPS_QoS_History
 */
DDS_EXPORT void dds_qset_history (dds_qos_t *qos, dds_history_kind_t kind, int32_t depth);

/**
 * Description : Set the resource limits policy to the qos structure. This value will be validated and applied
 * at the time of entity creation.
 *
 * Arguments :
 *   -# qos The pointer to the qos structure, where the resource limits will be applied
 *   -# max_samples, max_instances, max_samples_per_instance \ref DCPS_QoS_ResourceLimits
 */
DDS_EXPORT void dds_qset_resource_limits (dds_qos_t *qos, int32_t max_samples, int32_t max_instances, int32_t max_samples_per_instance);

/**
 * Description : Set the presentation policy in the qos structure. This value will be validated and applied
 * at the time of entity creation.
 *
 * Arguments :
 *   -# qos The pointer to the qos structure, where the presentation policy will be applied
 *   -# access_scope, coherent_access & ordered access  \ref DCPS_QoS_Presentation
 */
DDS_EXPORT void dds_qset_presentation (dds_qos_t *qos, dds_presentation_access_scope_kind_t access_scope, bool coherent_access, bool ordered_access);

/**
 * Description : Set the lifespan policy in the qos structure. This value will be validated and applied
 * at the time of entity creation.
 *
 * Arguments :
 *   -# qos The pointer to the qos structure, where the lifespan policy will be applied
 *   -# lifespan Expiration time relative to source timestamp beyond which the sample shall be
 *      removed from the caches.
 */
DDS_EXPORT void dds_qset_lifespan (dds_qos_t *qos, dds_duration_t lifespan);

/**
 * Description : Set the deadline policy in the qos structure. This value will be validated and applied
 * at the time of entity creation.
 *
 * Arguments :
 *   -# qos The pointer to the qos structure, where the deadline policy will be applied
 *   -# deadline \ref DCPS_QoS_Deadline.
 */
DDS_EXPORT void dds_qset_deadline (dds_qos_t *qos, dds_duration_t deadline);

/**
 * Description : Set the latency budget policy in the qos structure. This value will be validated
 * and applied at the time of entity creation.
 *
 * Arguments :
 *   -# qos The pointer to the qos structure, where the latency budget policy will be applied
 *   -# duration \ref DCPS_QoS_LatencyBudget
 */
DDS_EXPORT void dds_qset_latency_budget (dds_qos_t *qos, dds_duration_t duration);

/**
 * Description : Set the ownership policy in the qos structure. This value will be validated
 * and applied at the time of entity creation.
 *
 * Arguments :
 *   -# qos The pointer to the qos structure, where the ownership policy will be applied
 *   -# kind \ref DCPS_QoS_Ownership
 */
DDS_EXPORT void dds_qset_ownership (dds_qos_t *qos, dds_ownership_kind_t kind);

/**
 * Description : Set the ownership strength in the qos structure. This value will be validated
 * and applied at the time of entity creation.
 *
 * Arguments :
 *   -# qos The pointer to the qos structure, where the ownership strength will be applied
 *   -# value determines the ownership of a data instance, and the arbitration is performed by the data reader.
 */
DDS_EXPORT void dds_qset_ownership_strength (dds_qos_t *qos, int32_t value);

/**
 * Description : Set the liveliness policy in the qos structure. This value will be validated
 * and applied at the time of entity creation.
 *
 * Arguments :
 *   -# qos The pointer to the qos structure, where the liveliness policy will be applied
 *   -# kind, lease_duration \ref DCPS_QoS_Liveliness
 */
DDS_EXPORT void dds_qset_liveliness (dds_qos_t *qos, dds_liveliness_kind_t kind, dds_duration_t lease_duration);

/**
 * Description : Set the time based filter policy in the qos structure. This value will be validated
 * and applied at the time of entity creation.
 *
 * Arguments :
 *   -# qos The pointer to the qos structure, where the timebased filter policy will be applied
 *   -# minimum_separation The duration that determines the rate at which the data reader want to see the sample
 *      per instance.
 */
DDS_EXPORT void dds_qset_time_based_filter (dds_qos_t *qos, dds_duration_t minimum_separation);

/**
 * Description : Set the logical partition name in the qos structure. This value will be validated
 * and applied at the time of entity creation.
 *
 * Arguments :
 *   -# qos The pointer to the qos structure, where the timebased filter policy will be applied
 *   -# n The partition number
 *   -# ps The logical name to the partition to create \ref DCPS_QoS_Partition
 */
DDS_EXPORT void dds_qset_partition (dds_qos_t * restrict qos, uint32_t n, const char ** ps);

/**
 * Description : Set the reliability policy in the qos structure. This value will be validated
 * and applied at the time of entity creation.
 *
 * Arguments :
 *   -# qos The pointer to the qos structure, where the reliability policy will be applied
 *   -# kind, max_blocking_time \ref DCPS_QoS_Reliability
 */
DDS_EXPORT void dds_qset_reliability (dds_qos_t *qos, dds_reliability_kind_t kind, dds_duration_t max_blocking_time);

/**
 * Description : Set the transport priority policy in the qos structure. This value will be validated
 * and applied at the time of entity creation.
 *
 * Arguments :
 *   -# qos The pointer to the qos structure, where the transport policy will be applied
 *   -# value priority value for transporting the messages (higher the number, higher the priority)
 *   NOTE: Depends on the underlying transport.
 */
DDS_EXPORT void dds_qset_transport_priority (dds_qos_t *qos, int32_t value);

/**
 * Description : Set the destination order policy in the qos structure. This value will be validated
 * and applied at the time of entity creation.
 *
 * Arguments :
 *   -# qos The pointer to the qos structure, where the destination order will be applied
 *   -# kind \ref DCPS_QoS_DestinationOrder
 */
DDS_EXPORT void dds_qset_destination_order (dds_qos_t *qos, dds_destination_order_kind_t kind);

/**
 * Description : Set the writer data lifecycle policy in the qos structure. This value will be validated
 * and applied at the time of entity creation.
 *
 * Arguments :
 *   -# qos The pointer to the qos structure, where the writer data cycle policy will be applied
 *   -# autodispose_unregistered_instances
 *      - true : dispose the instance automatically each time when it is unregistered
 *      - false : automatic disposition will not happen upon unregistration
 */
DDS_EXPORT void dds_qset_writer_data_lifecycle (dds_qos_t *qos, bool autodispose_unregistered_instances);

/**
 * Description : Set the reader data lifecycle policy in the qos structure. This value will be validated
 * and applied at the time of entity creation.
 *
 * Arguments :
 *   -# qos The pointer to the qos structure, where the reader data cycle policy will be applied
 *   -# autopurge_nowriter_samples, autopurge_disposed_samples_delay \ref DCPS_QoS_ReaderDataLifecycle
 */
DDS_EXPORT void dds_qset_reader_data_lifecycle (dds_qos_t *qos, dds_duration_t autopurge_nowriter_samples, dds_duration_t autopurge_disposed_samples_delay);

/**
 * Description : Set the durability service qos. This value will be validated
 * and applied on entity creation.
 *
 * Arguments :
 *   -# qos The pointer to the qos structure, where the durability service policy will be applied
 *   -# service_cleanup_delay Controls when the durability service can remove all information regarding a data-instance
 *   -# history_kind, history_depth Controls the HISTORY QoS of the durability server DataReader
 *   -# max_samples, max_instances, max_samples_per_instance Controls the RESOURCE_ LIMITS QoS of the durability server DataReader
 */
DDS_EXPORT void dds_qset_durability_service
(
  dds_qos_t * qos,
  dds_duration_t service_cleanup_delay,
  dds_history_kind_t history_kind,
  int32_t history_depth,
  int32_t max_samples,
  int32_t max_instances,
  int32_t max_samples_per_instance
);

/* Getters: return false if not set, true if set; userdata, topicdata,
 * groupdata, partition are all aliased into the qos data, NULL
 * pointers indicate the value is of no interest, so
 * dds_qget_deadline (&qos,NULL) is a simple way of testing whether the
 * deadline qos is set at all.
 */

/**
 * Description : Get the userdata policy.
 *               This operation returns false to indicate that the policy is not set.
 *               To check whether the policy is set at all, all other arguments could be set to NULL
 *               except qos.
 *
 * Arguments :
 *   -# qos The pointer to the qos structure which has the policies set
 *   -# value The pointer to the userdata (can be NULL)
 *   -# sz The size of the userdata (can be NULL)
 */
DDS_EXPORT void dds_qget_userdata (const dds_qos_t *qos, void ** value, size_t *sz);

/**
 * Description : Get the topicdata policy
 *               This operation returns false to indicate that the policy is not set.
 *               To check whether the policy is set at all, all other arguments could be set to NULL
 *               except qos.
 *
 * Arguments :
 *   -# qos The pointer to the qos structure which has the policies set
 *   -# value The pointer to the topicdata (can be NULL)
 *   -# sz The size of the topicdata (can be NULL)
 */
DDS_EXPORT void dds_qget_topicdata (const dds_qos_t *qos, void ** value, size_t *sz);

/**
 * Description : Get the groupdata policy
 *               This operation returns false to indicate that the policy is not set.
 *               To check whether the policy is set at all, all other arguments could be set to NULL
 *               except qos.
 *
 * Arguments :
 *   -# qos The pointer to the qos structure which has the policies set
 *   -# value The pointer to the groupdata (can be NULL)
 *   -# sz The size of the groupdata (can be NULL)
 */
DDS_EXPORT void dds_qget_groupdata (const dds_qos_t *qos, void ** value, size_t *sz);

/**
 * Description : Get the durability qos policy
 *               This operation returns false to indicate that the policy is not set.
 *               To check whether the policy is set at all, all other arguments could be set to NULL
 *               except qos.
 * Arguments :
 *   -# qos The pointer to the qos structure which has the policies set
 *   -# kind The pointer to \ref DCPS_QoS_Durability (can be NULL)
 */
DDS_EXPORT void dds_qget_durability (const dds_qos_t *qos, dds_durability_kind_t *kind);

/**
 * Description : Get the history qos policy
 *               This operation returns false to indicate that the policy is not set.
 *               To check whether the policy is set at all, all other arguments could be set to NULL
 *               except qos.
 *
 * Arguments :
 *   -# qos The pointer to the qos structure which has the policies set
 *   -# kind, depth The pointer to retrieve \ref DCPS_QoS_History (can be NULL)
 */
DDS_EXPORT void dds_qget_history (const dds_qos_t *qos, dds_history_kind_t *kind, int32_t * depth);

/**
 * Description : Get the resource limits qos policy
 *               This operation returns false to indicate that the policy is not set.
 *               To check whether the policy is set at all, all other arguments could be set to NULL
 *               except qos.
 *
 * Arguments :
 *   -# qos The pointer to the qos structure which has the policies set
 *   -# max_samples, max_instances, max_samples_per_instance The pointer to retrieve the value of
 *      \ref DCPS_QoS_ResourceLimits (can be NULL)
 */
DDS_EXPORT void dds_qget_resource_limits (const dds_qos_t *qos, int32_t *max_samples, int32_t *max_instances, int32_t *max_samples_per_instance);

/**
 * Description : Get the presentation qos policy
 *               This operation returns false to indicate that the policy is not set.
 *               To check whether the policy is set at all, all other arguments could be set to NULL
 *               except qos.
 *
 * Arguments :
 *   -# qos The pointer to the qos structure which has the policies set
 *   -# access_scope, coherent_access & ordered_access The pointer to \ref DCPS_QoS_Presentation (can be NULL)
 */
DDS_EXPORT void dds_qget_presentation (const dds_qos_t *qos, dds_presentation_access_scope_kind_t *access_scope, bool *coherent_access, bool *ordered_access);

/**
 * Description : Get the lifespan qos policy
 *               This operation returns false to indicate that the policy is not set.
 *               To check whether the policy is set at all, all other arguments could be set to NULL
 *               except qos.
 *
 * Arguments :
 *   -# qos The pointer to the qos structure which has the policies set
 *   -# lifespan The pointer to retrieve the lifespan duration value set (can be NULL)
 */
DDS_EXPORT void dds_qget_lifespan (const dds_qos_t *qos, dds_duration_t *lifespan);

/**
 * Description : Get the deadline qos policy
 *               This operation returns false to indicate that the policy is not set.
 *               To check whether the policy is set at all, all other arguments could be set to NULL
 *               except qos.
 *
 * Arguments :
 *   -# qos The pointer to the qos structure which has the policies set
 *   -# deadline The pointer to retrieve \ref DCPS_QoS_Deadline (can be NULL)
 */
DDS_EXPORT void dds_qget_deadline (const dds_qos_t *qos, dds_duration_t *deadline);

/**
 * Description : Get the latency budget qos policy
 *               This operation returns false to indicate that the policy is not set.
 *               To check whether the policy is set at all, all other arguments could be set to NULL
 *               except qos.
 * Arguments :
 *   -# qos The pointer to the qos structure which has the policies set
 *   -# duration The pointer to retrieve \ref DCPS_QoS_LatencyBudget (can be NULL)
 */
DDS_EXPORT void dds_qget_latency_budget (const dds_qos_t *qos, dds_duration_t *duration);

/**
 * Description : Get the ownership qos policy
 *               This operation returns false to indicate that the policy is not set.
 *               To check whether the policy is set at all, all other arguments could be set to NULL
 *               except qos.
 *
 * Arguments :
 *   -# qos The pointer to the qos structure which has the policies set
 *   -# kind The pointer to retrieve the value of \ref dds_ownership_kind_t (can be NULL)
 */
DDS_EXPORT void dds_qget_ownership (const dds_qos_t *qos, dds_ownership_kind_t *kind);

/**
 * Description : Get the ownership strength qos policy
 *               This operation returns false to indicate that the policy is not set.
 *               To check whether the policy is set at all, all other arguments could be set to NULL
 *               except qos.
 *
 * Arguments :
 *   -# qos The pointer to the qos structure which has the policies set
 *   -# value The pointer to retrieve the ownership strength (can be NULL)
 */
DDS_EXPORT void dds_qget_ownership_strength (const dds_qos_t *qos, int32_t *value);

/**
 * Description : Get the liveliness qos policy
 *               This operation returns false to indicate that the policy is not set.
 *               To check whether the policy is set at all, all other arguments could be set to NULL
 *               except qos.
 *
 * Arguments :
 *   -# qos The pointer to the qos structure which has the policies set
 *   -# kind, lease_duration The pointer to retrieve \ref DCPS_QoS_Liveliness (can be NULL)
 */
DDS_EXPORT void dds_qget_liveliness (const dds_qos_t *qos, dds_liveliness_kind_t *kind, dds_duration_t *lease_duration);

/**
 * Description : Get the timebased filter qos policy
 *               This operation returns false to indicate that the policy is not set.
 *               To check whether the policy is set at all, all other arguments could be set to NULL
 *               except qos.
 * Arguments :
 *   -# qos The pointer to the qos structure which has the policies set
 *   -# minimum_separation The pointer to retrieve \ref DCPS_QoS_TimeBasedFilter (can be NULL)
 */
DDS_EXPORT void dds_qget_time_based_filter (const dds_qos_t *qos, dds_duration_t *minimum_separation);

/**
 * Description : Get the logical partition qos policy
 *               This operation returns false to indicate that the policy is not set.
 *               To check whether the policy is set at all, all other arguments could be set to NULL
 *               except qos.
 * Arguments :
 *   -# qos The pointer to the qos structure which has the policies set
 *   -# n The pointer to retrieve the partition number set (can be NULL)
 *   -# ps The pointer to \ref DCPS_QoS_Partition (can be NULL)
 */
DDS_EXPORT void dds_qget_partition (const dds_qos_t *qos, uint32_t *n, char *** ps);

/**
 * Description : Get the reliability qos policy
 *               This operation returns false to indicate that the policy is not set.
 *               To check whether the policy is set at all, all other arguments could be set to NULL
 *               except qos.
 *
 * Arguments :
 *   -# qos The pointer to the qos structure which has the policies set
 *   -# kind, max_blocking_time The pointer to retrieve \ref DCPS_QoS_Reliability (can be NULL)
 */
DDS_EXPORT void dds_qget_reliability (const dds_qos_t *qos, dds_reliability_kind_t *kind, dds_duration_t *max_blocking_time);

/**
 * Description : Get the transport priority qos policy
 *               This operation returns false to indicate that the policy is not set.
 *               To check whether the policy is set at all, all other arguments could be set to NULL
 *               except qos.
 *
 * Arguments :
 *   -# qos The pointer to the qos structure which has the policies set
 *   -# value The pointer to retrieve the transport priority set (can be NULL)
 */
DDS_EXPORT void dds_qget_transport_priority (const dds_qos_t *qos, int32_t *value);

/**
 * Description : Get the destination order qos policy
 *               This operation returns false to indicate that the policy is not set.
 *               To check whether the policy is set at all, all other arguments could be set to NULL
 *               except qos.
 *
 * Arguments :
 *   -# qos The pointer to the qos structure which has the policies set
 *   -# value The pointer to retrieve \ref DCPS_QoS_DestinationOrder (can be NULL)
 */
DDS_EXPORT void dds_qget_destination_order (const dds_qos_t *qos, dds_destination_order_kind_t *value);

/**
 * Description : Get the writer data lifecycle qos policy
 *               This operation returns false to indicate that the policy is not set.
 *               To check whether the policy is set at all, all other arguments could be set to NULL
 *               except qos.
 *
 * Arguments :
 *   -# qos The pointer to the qos structure which has the policies set
 *   -# autodispose_unregistered_instances The pointer to retrieve the value set (can be NULL)
 */
DDS_EXPORT void dds_qget_writer_data_lifecycle (const dds_qos_t *qos, bool * autodispose_unregistered_instances);

/**
 * Description : Get the reader data lifecycle qos policy
 *               This operation returns false to indicate that the policy is not set.
 *               To check whether the policy is set at all, all other arguments could be set to NULL
 *               except qos.
 *
 * Arguments :
 *   -# qos The pointer to the qos structure which has the policies set
 *   -# autopurge_nowriter_samples & autopurge_disposed_samples_delay
 *      The pointer to retrieve DCPS_QoS_ReaderDataLifecycle (can be NULL)
 */
DDS_EXPORT void dds_qget_reader_data_lifecycle (const dds_qos_t *qos, dds_duration_t *autopurge_nowriter_samples, dds_duration_t *autopurge_disposed_samples_delay);

/**
 * Description : Get the durability service qos policy values.
 *
 * Arguments :
 *   -# qos The pointer to the qos structure, where the durability service policy will be applied
 *   -# service_cleanup_delay Controls when the durability service can remove all information regarding a data-instance
 *   -# history_kind, history_depth Controls the HISTORY QoS of the durability server DataReader
 *   -# max_samples, max_instances, max_samples_per_instance Controls the RESOURCE_ LIMITS QoS of the durability server DataReader
 */
DDS_EXPORT void dds_qget_durability_service
(
  const dds_qos_t * qos,
  dds_duration_t * service_cleanup_delay,
  dds_history_kind_t * history_kind,
  int32_t * history_depth,
  int32_t * max_samples,
  int32_t * max_instances,
  int32_t * max_samples_per_instance
);

#undef DDS_EXPORT

#if defined (__cplusplus)
}
#endif
#endif
