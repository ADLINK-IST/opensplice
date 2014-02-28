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
#ifndef NN_XQOS_H
#define NN_XQOS_H

/*XXX*/
#include "q_protocol.h"
#include "q_rtps.h"
/*XXX*/
#include "q_log.h"

#if defined (__cplusplus)
extern "C" {
#endif

#define NN_DDS_LENGTH_UNLIMITED -1

typedef struct nn_octetseq {
  int length;
  unsigned char *value;
} nn_octetseq_t;

typedef nn_octetseq_t nn_userdata_qospolicy_t;
typedef nn_octetseq_t nn_topicdata_qospolicy_t;
typedef nn_octetseq_t nn_groupdata_qospolicy_t;

typedef enum nn_durability_kind {
  NN_VOLATILE_DURABILITY_QOS,
  NN_TRANSIENT_LOCAL_DURABILITY_QOS,
  NN_TRANSIENT_DURABILITY_QOS,
  NN_PERSISTENT_DURABILITY_QOS
} nn_durability_kind_t;

typedef struct nn_durability_qospolicy {
  nn_durability_kind_t kind;
} nn_durability_qospolicy_t;

typedef enum nn_history_kind {
  NN_KEEP_LAST_HISTORY_QOS,
  NN_KEEP_ALL_HISTORY_QOS
} nn_history_kind_t;

typedef struct nn_history_qospolicy {
  nn_history_kind_t kind;
  int depth;
} nn_history_qospolicy_t;

typedef struct nn_resource_limits_qospolicy {
  int max_samples;
  int max_instances;
  int max_samples_per_instance;
} nn_resource_limits_qospolicy_t;

typedef struct nn_durability_service_qospolicy {
  nn_duration_t service_cleanup_delay;
  nn_history_qospolicy_t history;
  nn_resource_limits_qospolicy_t resource_limits;
} nn_durability_service_qospolicy_t;

typedef enum nn_presentation_access_scope_kind {
  NN_INSTANCE_PRESENTATION_QOS,
  NN_TOPIC_PRESENTATION_QOS,
  NN_GROUP_PRESENTATION_QOS
} nn_presentation_access_scope_kind_t;

typedef struct nn_presentation_qospolicy {
  nn_presentation_access_scope_kind_t access_scope;
  char coherent_access;
  char ordered_access;
} nn_presentation_qospolicy_t;

typedef struct nn_deadline_qospolicy {
  nn_duration_t deadline;
} nn_deadline_qospolicy_t;

typedef struct nn_latency_budget_qospolicy {
  nn_duration_t duration;
} nn_latency_budget_qospolicy_t;

typedef enum nn_ownership_kind {
  NN_SHARED_OWNERSHIP_QOS,
  NN_EXCLUSIVE_OWNERSHIP_QOS
} nn_ownership_kind_t;

typedef struct nn_ownership_qospolicy {
  nn_ownership_kind_t kind;
} nn_ownership_qospolicy_t;

typedef struct nn_ownership_strength_qospolicy {
  int value;
} nn_ownership_strength_qospolicy_t;

typedef enum nn_liveliness_kind {
  NN_AUTOMATIC_LIVELINESS_QOS,
  NN_MANUAL_BY_PARTICIPANT_LIVELINESS_QOS,
  NN_MANUAL_BY_TOPIC_LIVELINESS_QOS
} nn_liveliness_kind_t;

typedef struct nn_liveliness_qospolicy {
  nn_liveliness_kind_t kind;
  nn_duration_t lease_duration;
} nn_liveliness_qospolicy_t;

typedef struct nn_time_based_filter_qospolicy {
  nn_duration_t minimum_separation;
} nn_time_based_filter_qospolicy_t;

typedef struct nn_stringseq {
  int n;
  char **strs;
} nn_stringseq_t;

typedef nn_stringseq_t nn_partition_qospolicy_t;

typedef enum nn_reliability_kind {
  NN_BEST_EFFORT_RELIABILITY_QOS,
  NN_RELIABLE_RELIABILITY_QOS
} nn_reliability_kind_t;

typedef struct nn_reliability_qospolicy {
  nn_reliability_kind_t kind;
  nn_duration_t max_blocking_time;
} nn_reliability_qospolicy_t;

typedef struct nn_external_reliability_qospolicy {
  int kind;
  nn_duration_t max_blocking_time;
} nn_external_reliability_qospolicy_t;

#define NN_PEDANTIC_BEST_EFFORT_RELIABILITY_QOS 1
#define NN_PEDANTIC_RELIABLE_RELIABILITY_QOS    3 /* <= see DDSI 2.1, table 9.4 */
#define NN_INTEROP_BEST_EFFORT_RELIABILITY_QOS  1
#define NN_INTEROP_RELIABLE_RELIABILITY_QOS     2

typedef struct nn_transport_priority_qospolicy {
  int value;
} nn_transport_priority_qospolicy_t;

typedef struct nn_lifespan_qospolicy {
  nn_duration_t duration;
} nn_lifespan_qospolicy_t;

typedef enum nn_destination_order_kind {
  NN_BY_RECEPTION_TIMESTAMP_DESTINATIONORDER_QOS,
  NN_BY_SOURCE_TIMESTAMP_DESTINATIONORDER_QOS
} nn_destination_order_kind_t;

typedef struct nn_destination_order_qospolicy {
  nn_destination_order_kind_t kind;
} nn_destination_order_qospolicy_t;

#if 0
typedef struct nn_entity_factory_qospolicy {
  char autoeable_created_entities;
} nn_entity_factory_qospolicy_t;
#endif

typedef struct nn_writer_data_lifecycle_qospolicy {
  char autodispose_unregistered_instances;
} nn_writer_data_lifecycle_qospolicy_t;

typedef struct nn_reader_data_lifecycle_qospolicy {
  nn_duration_t autopurge_nowriter_samples_delay;
  nn_duration_t autopurge_disposed_samples_delay;
} nn_reader_data_lifecycle_qospolicy_t;


typedef struct nn_relaxed_qos_matching_qospolicy {
  char value;
} nn_relaxed_qos_matching_qospolicy_t;


/***/

/* Qos Present bit indices */
#define QP_TOPIC_NAME              (1u <<  0)
#define QP_TYPE_NAME               (1u <<  1)
#define QP_PRESENTATION            (1u <<  2)
#define QP_PARTITION               (1u <<  3)
#define QP_GROUP_DATA              (1u <<  4)
#define QP_TOPIC_DATA              (1u <<  5)
#define QP_DURABILITY              (1u <<  6)
#define QP_DURABILITY_SERVICE      (1u <<  7)
#define QP_DEADLINE                (1u <<  8)
#define QP_LATENCY_BUDGET          (1u <<  9)
#define QP_LIVELINESS              (1u << 10)
#define QP_RELIABILITY             (1u << 11)
#define QP_DESTINATION_ORDER       (1u << 12)
#define QP_HISTORY                 (1u << 13)
#define QP_RESOURCE_LIMITS         (1u << 14)
#define QP_TRANSPORT_PRIORITY      (1u << 15)
#define QP_LIFESPAN                (1u << 16)
#define QP_USER_DATA               (1u << 17)
#define QP_OWNERSHIP               (1u << 18)
#define QP_OWNERSHIP_STRENGTH      (1u << 19)
#define QP_TIME_BASED_FILTER       (1u << 20)
#define QP_PRISMTECH_WRITER_DATA_LIFECYCLE   (1u << 21)
#define QP_PRISMTECH_READER_DATA_LIFECYCLE   (1u << 22)
#define QP_PRISMTECH_RELAXED_QOS_MATCHING    (1u << 23)

/* Partition QoS is not RxO according to the specification (DDS 1.2,
   section 7.1.3), but communication will not take place unless it
   matches. Same for topic and type.  Relaxed qos matching is a bit of
   a weird one, but it affects matching, so ... */
#define QP_RXO_MASK (QP_DURABILITY | QP_PRESENTATION | QP_DEADLINE | QP_LATENCY_BUDGET | QP_OWNERSHIP | QP_LIVELINESS | QP_RELIABILITY | QP_DESTINATION_ORDER | QP_PRISMTECH_RELAXED_QOS_MATCHING)
#define QP_CHANGEABLE_MASK (QP_USER_DATA | QP_TOPIC_DATA | QP_GROUP_DATA | QP_DEADLINE | QP_LATENCY_BUDGET | QP_OWNERSHIP_STRENGTH | QP_TIME_BASED_FILTER | QP_PARTITION | QP_TRANSPORT_PRIORITY | QP_LIFESPAN | QP_ENTITY_FACTORY | QP_PRISMTECH_WRITER_DATA_LIFECYCLE | QP_PRISMTECH_READER_DATA_LIFECYCLE)

/* readers & writers have an extended qos, hence why it is a separate
   type */
typedef struct nn_xqos {
  /* Entries present, for sparse QoS */
  unsigned present;
  unsigned aliased;

  /*v---- in ...Qos
     v--- in ...BuiltinTopicData
      v-- mapped in DDSI
       v- reader/writer specific */
  /*      Extras: */
  /* xx */char *topic_name;
  /* xx */char *type_name;
  /*      PublisherQos, SubscriberQos: */
  /*xxx */nn_presentation_qospolicy_t presentation;
  /*xxx */nn_partition_qospolicy_t partition;
  /*xxx */nn_groupdata_qospolicy_t group_data;
#if 0
  /*x   */nn_entity_factory_qospolicy_t entity_factory;
#endif
  /*      TopicQos: */
  /*xxx */nn_topicdata_qospolicy_t topic_data;
  /*      DataWriterQos, DataReaderQos: */
  /*xxx */nn_durability_qospolicy_t durability;
  /*xxx */nn_durability_service_qospolicy_t durability_service;
  /*xxx */nn_deadline_qospolicy_t deadline;
  /*xxx */nn_latency_budget_qospolicy_t latency_budget;
  /*xxx */nn_liveliness_qospolicy_t liveliness;
  /*xxx */nn_reliability_qospolicy_t reliability;
  /*xxx */nn_destination_order_qospolicy_t destination_order;
  /*x x */nn_history_qospolicy_t history;
  /*x x */nn_resource_limits_qospolicy_t resource_limits;
  /*x x */nn_transport_priority_qospolicy_t transport_priority;
  /*xxx */nn_lifespan_qospolicy_t lifespan;
  /*xxx */nn_userdata_qospolicy_t user_data;
  /*xxx */nn_ownership_qospolicy_t ownership;
  /*xxxW*/nn_ownership_strength_qospolicy_t ownership_strength;
  /*xxxR*/nn_time_based_filter_qospolicy_t time_based_filter;
  /*x  W*/nn_writer_data_lifecycle_qospolicy_t writer_data_lifecycle;
  /*x xR*/nn_reader_data_lifecycle_qospolicy_t reader_data_lifecycle;
  /*x x */nn_relaxed_qos_matching_qospolicy_t relaxed_qos_matching;
} nn_xqos_t;

struct nn_xmsg;

void nn_xqos_init_empty (nn_xqos_t *xqos);
int nn_xqos_init_default_reader (nn_xqos_t *xqos);
int nn_xqos_init_default_writer (nn_xqos_t *xqos);
int nn_xqos_copy (nn_xqos_t *dst, const nn_xqos_t *src);
int nn_xqos_unalias (nn_xqos_t *xqos);
void nn_xqos_fini (nn_xqos_t *xqos);
int nn_xqos_mergein_missing (nn_xqos_t *a, const nn_xqos_t *b);
unsigned nn_xqos_delta (const nn_xqos_t *a, const nn_xqos_t *b, unsigned mask);
int nn_xqos_addtomsg (struct nn_xmsg *m, const nn_xqos_t *xqos, unsigned wanted);
void nn_log_xqos (logcat_t cat, const nn_xqos_t *xqos);

#if defined (__cplusplus)
}
#endif

#endif /* NN_XQOS_H */

/* SHA1 not available (unoffical build.) */
