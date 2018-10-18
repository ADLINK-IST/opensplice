#ifndef DDS_STATUS_H
#define DDS_STATUS_H

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

/** @file status.h
 *  @brief Vortex Lite Communication Status header
 */

/* Listeners implemented as structs containing callback functions
 * that take listener status types as arguments.
 */

/* Listener status types */
/**
 * \ref DCPS_Status_OfferedDeadlineMissed
 */
typedef struct dds_offered_deadline_missed_status
{
  uint32_t total_count;
  int32_t total_count_change;
  dds_instance_handle_t last_instance_handle;
}
dds_offered_deadline_missed_status_t;

/**
 * \ref DCPS_Status_OfferedIncompatibleQoS
 */
typedef struct dds_offered_incompatible_qos_status
{
  uint32_t total_count;
  int32_t total_count_change;
  uint32_t last_policy_id;
}
dds_offered_incompatible_qos_status_t;

/**
 * \ref DCPS_Status_PublicationMatched
 */
typedef struct dds_publication_matched_status
{
  uint32_t total_count;
  int32_t total_count_change;
  uint32_t current_count;
  int32_t current_count_change;
  dds_instance_handle_t last_subscription_handle;
}
dds_publication_matched_status_t;

/**
 * \ref DCPS_Status_LivelinessLost
 */
typedef struct dds_liveliness_lost_status
{
  uint32_t total_count;
  int32_t total_count_change;
}
dds_liveliness_lost_status_t;

/**
 * \ref DCPS_Status_SubscriptionMatched
 */
typedef struct dds_subscription_matched_status
{
  uint32_t total_count;
  int32_t total_count_change;
  uint32_t current_count;
  int32_t current_count_change;
  dds_instance_handle_t last_publication_handle;
}
dds_subscription_matched_status_t;

/**
 * dds_sample_rejected_status_kind
 */
typedef enum
{
  DDS_SRST_READ = DDS_NOT_REJECTED,
  DDS_SRST_REJECTED_BY_INSTANCES_LIMIT = DDS_REJECTED_BY_INSTANCES_LIMIT,
  DDS_SRST_REJECTED_BY_SAMPLES_LIMIT = DDS_REJECTED_BY_SAMPLES_LIMIT,
  DDS_SRST_REJECTED_BY_SAMPLES_PER_INSTANCE_LIMIT = DDS_REJECTED_BY_SAMPLES_PER_INSTANCE_LIMIT
}
dds_sample_rejected_status_kind;

/**
 * \ref DCPS_Status_SampleRejected
 */
typedef struct dds_sample_rejected_status
{
  uint32_t total_count;
  int32_t total_count_change;
  dds_sample_rejected_status_kind last_reason;
  dds_instance_handle_t last_instance_handle;
}
dds_sample_rejected_status_t;

/**
 * \ref DCPS_Status_LivelinessChanged
 */
typedef struct dds_liveliness_changed_status
{
  uint32_t alive_count;
  uint32_t not_alive_count;
  int32_t alive_count_change;
  int32_t not_alive_count_change;
  dds_instance_handle_t last_publication_handle;
}
dds_liveliness_changed_status_t;

/**
 * \ref DCPS_Status_RequestedDeadlineMissed
 */
typedef struct dds_requested_deadline_missed_status
{
  uint32_t total_count;
  int32_t total_count_change;
  dds_instance_handle_t last_instance_handle;
}
dds_requested_deadline_missed_status_t;

/**
 * \ref DCPS_Status_RequestedIncompatibleQoS
 */
typedef struct dds_requested_incompatible_qos_status
{
  uint32_t total_count;
  int32_t total_count_change;
  uint32_t last_policy_id;
}
dds_requested_incompatible_qos_status_t;

/**
 * \ref DCPS_Status_SampleLost
 */
typedef struct dds_sample_lost_status
{
  uint32_t total_count;
  int32_t total_count_change;
}
dds_sample_lost_status_t;

/**
 * \ref DCPS_Status_InconsistentTopic
 */
typedef struct dds_inconsistent_topic_status
{
  uint32_t total_count;
  int32_t total_count_change;
}
dds_inconsistent_topic_status_t;

/* Listener types */
/** \ref DCPS_Modules_Infrastructure_Listener  - TopicListener */


typedef struct dds_topiclistener
{
  void (*on_inconsistent_topic) (dds_entity_t topic, dds_inconsistent_topic_status_t * status);
}
dds_topiclistener_t;

/** \ref DCPS_Modules_Infrastructure_Listener  - DataWriterListener */
typedef struct dds_writerlistener
{
  void (*on_offered_deadline_missed) (dds_entity_t writer, dds_offered_deadline_missed_status_t * status);
  void (*on_offered_incompatible_qos) (dds_entity_t writer, dds_offered_incompatible_qos_status_t * status);
  void (*on_liveliness_lost) (dds_entity_t writer, dds_liveliness_lost_status_t * status);
  void (*on_publication_matched) (dds_entity_t writer, dds_publication_matched_status_t * status);
}
dds_writerlistener_t;

/** \ref DCPS_Modules_Infrastructure_Listener  - PublisherListener */
typedef struct dds_publisherlistener
{
  dds_writerlistener_t writerlistener;
}
dds_publisherlistener_t;

/** \ref DCPS_Modules_Infrastructure_Listener  - DataReaderListener */
typedef struct dds_readerlistener
{
  void (*on_requested_deadline_missed) (dds_entity_t reader, dds_requested_deadline_missed_status_t * status);
  void (*on_requested_incompatible_qos) (dds_entity_t reader, dds_requested_incompatible_qos_status_t * status);
  void (*on_sample_rejected) (dds_entity_t reader, dds_sample_rejected_status_t * status);
  void (*on_liveliness_changed) (dds_entity_t reader, dds_liveliness_changed_status_t * status);
  void (*on_data_available) (dds_entity_t reader);
  void (*on_subscription_matched) (dds_entity_t reader, dds_subscription_matched_status_t * status);
  void (*on_sample_lost) (dds_entity_t reader, dds_sample_lost_status_t * status);
}
dds_readerlistener_t;

/** \ref DCPS_Modules_Infrastructure_Listener  - SubscriberListener */
typedef struct dds_subscriberlistener
{
  dds_readerlistener_t readerlistener;
  void (*on_data_readers) (dds_entity_t subscriber);
}
dds_subscriberlistener_t;

/** \ref DCPS_Modules_Infrastructure_Listener  - DomainParticipantListener */
typedef struct dds_participantlistener
{
  dds_topiclistener_t topiclistener;
  dds_publisherlistener_t publisherlistener;
  dds_subscriberlistener_t subscriberlistener;
}
dds_participantlistener_t;

/* get_<status> APIs return the status of an entity and resets the status */

/**
 * Description : Get the status value corresponding to INCONSISTENT_TOPIC and reset the status
 *               The value can be obtained, only if the status is enabled for an entity
 *
 * Arguments :
 *   -# topic The entity to get the status
 *   -# status The pointer to \ref DCPS_Status_InconsistentTopic to get the status
 */
DDS_EXPORT int dds_get_inconsistent_topic_status (dds_entity_t topic, dds_inconsistent_topic_status_t * status);


/**
 * Description : Get the status value corresponding to PUBLICATION_MATCHED and reset the status
 *               The value can be obtained, only if the status is enabled for an entity
 *
 * Arguments :
 *   -# writer The entity to get the status
 *   -# status The pointer to \ref DCPS_Status_PublicationMatched to get the status
 */
DDS_EXPORT int dds_get_publication_matched_status (dds_entity_t writer, dds_publication_matched_status_t * status);

/**
 * Description : Get the status value corresponding to LIVELINESS_LOST and reset the status
 *               The value can be obtained, only if the status is enabled for an entity
 *
 * Arguments :
 *   -# writer The entity to get the status
 *   -# status The pointer to \ref DCPS_Status_LivelinessLost to get the status
 */
DDS_EXPORT int dds_get_liveliness_lost_status (dds_entity_t writer, dds_liveliness_lost_status_t * status);

/**
 * Description : Get the status value corresponding to OFFERED_DEADLINE_MISSED and reset the status
 *               The value can be obtained, only if the status is enabled for an entity
 *
 * Arguments :
 *   -# writer The entity to get the status
 *   -# status The pointer to \ref DCPS_Status_OfferedDeadlineMissed to get the status
 */
DDS_EXPORT int dds_get_offered_deadline_missed_status (dds_entity_t writer, dds_offered_deadline_missed_status_t * status);

/**
 * Description : Get the status value corresponding to OFFERED_INCOMPATIBLE_QOS and reset the status
 *               The value can be obtained, only if the status is enabled for an entity
 *
 * Arguments :
 *   -# writer The entity to get the status
 *   -# status The pointer to \ref DCPS_Status_OfferedIncompatibleQoS to get the status
 */
DDS_EXPORT int dds_get_offered_incompatible_qos_status (dds_entity_t writer, dds_offered_incompatible_qos_status_t * status);

/**
 * Description : Get the status value corresponding to SUBSCRIPTION_MATCHED and reset the status
 *               The value can be obtained, only if the status is enabled for an entity
 *
 * Arguments :
 *   -# reader The entity to get the status
 *   -# status The pointer to \ref DCPS_Status_SubscriptionMatched to get the status
 */
DDS_EXPORT int dds_get_subscription_matched_status (dds_entity_t reader, dds_subscription_matched_status_t * status);

/**
 * Description : Get the status value corresponding to LIVELINESS_CHANGED and reset the status
 *               The value can be obtained, only if the status is enabled for an entity
 *
 * Arguments :
 *   -# reader The entity to get the status
 *   -# status The pointer to \ref DCPS_Status_LivelinessChanged to get the status
 */
DDS_EXPORT int dds_get_liveliness_changed_status (dds_entity_t reader, dds_liveliness_changed_status_t * status);

/**
 * Description : Get the status value corresponding to SAMPLE_REJECTED and reset the status
 *               The value can be obtained, only if the status is enabled for an entity
 *
 * Arguments :
 *   -# reader The entity to get the status
 *   -# status The pointer to \ref DCPS_Status_SampleRejected to get the status
 */
DDS_EXPORT int dds_get_sample_rejected_status (dds_entity_t reader, dds_sample_rejected_status_t * status);

/**
 * Description : Get the status value corresponding to SAMPLE_LOST and reset the status
 *               The value can be obtained, only if the status is enabled for an entity
 *
 * Arguments :
 *   -# reader The entity to get the status
 *   -# status The pointer to \ref DCPS_Status_SampleLost to get the status
 */
DDS_EXPORT int dds_get_sample_lost_status (dds_entity_t reader, dds_sample_lost_status_t * status);

/**
 * Description : Get the status value corresponding to REQUESTED_DEADLINE_MISSED and reset the status
 *               The value can be obtained, only if the status is enabled for an entity
 *
 * Arguments :
 *   -# reader The entity to get the status
 *   -# status The pointer to \ref DCPS_Status_RequestedDeadlineMissed to get the status
 */
DDS_EXPORT int dds_get_requested_deadline_missed_status (dds_entity_t reader, dds_requested_deadline_missed_status_t * status);

/**
 * Description : Get the status value corresponding to REQUESTED_INCOMPATIBLE_QOS and reset the status
 *               The value can be obtained, only if the status is enabled for an entity
 *
 * Arguments :
 *   -# reader The entity to get the status
 *   -# status The pointer to \ref DCPS_Status_RequestedIncompatibleQoS to get the status
 */
DDS_EXPORT int dds_get_requested_incompatible_qos_status (dds_entity_t reader, dds_requested_incompatible_qos_status_t * status);

#undef DDS_EXPORT
#if defined (__cplusplus)
}
#endif
#endif
