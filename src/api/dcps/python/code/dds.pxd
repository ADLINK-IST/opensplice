#
#                         Vortex OpenSplice
#
#   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
#   Technology Limited, its affiliated companies and licensors. All rights
#   reserved.
#
#   Licensed under the Apache License, Version 2.0 (the "License");
#   you may not use this file except in compliance with the License.
#   You may obtain a copy of the License at
#
#       http://www.apache.org/licenses/LICENSE-2.0
#
#   Unless required by applicable law or agreed to in writing, software
#   distributed under the License is distributed on an "AS IS" BASIS,
#   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#   See the License for the specific language governing permissions and
#   limitations under the License.
#
# Python API - for listener callbacks
cdef extern from "Python.h":
    void PyEval_InitThreads()

# The C99 API
cdef extern from "dds.h" nogil:

    cdef int DDS_RETCODE_OK                   #0 /**< Success */
    cdef int DDS_RETCODE_ERROR                #1 /**< Non specific error */
    cdef int DDS_RETCODE_UNSUPPORTED          #2 /**< Feature unsupported */
    cdef int DDS_RETCODE_BAD_PARAMETER        #3 /**< Bad parameter value */
    cdef int DDS_RETCODE_PRECONDITION_NOT_MET #4 /**< Precondition for operation not met */
    cdef int DDS_RETCODE_OUT_OF_RESOURCES     #5 /**< When an operation fails because of a lack of resources */
    cdef int DDS_RETCODE_NOT_ENABLED          #6 /**< When a configurable feature is not enabled */
    cdef int DDS_RETCODE_IMMUTABLE_POLICY     #7 /**< When an attempt is made to modify an immutable policy */
    cdef int DDS_RETCODE_INCONSISTENT_POLICY  #8 /**< When a policy is used with inconsistent values */
    cdef int DDS_RETCODE_ALREADY_DELETED      #9 /**< When an attempt is made to delete something more than once */
    cdef int DDS_RETCODE_TIMEOUT              #10 /**< When a timeout has occurred */
    cdef int DDS_RETCODE_NO_DATA              #11 /*< When expected data is not provided */
    cdef int DDS_RETCODE_ILLEGAL_OPERATION    #12 /*< When a function is called when it should not be */

    cdef unsigned DDS_INCONSISTENT_TOPIC_STATUS
    cdef unsigned DDS_OFFERED_DEADLINE_MISSED_STATUS
    cdef unsigned DDS_REQUESTED_DEADLINE_MISSED_STATUS
    cdef unsigned DDS_OFFERED_INCOMPATIBLE_QOS_STATUS
    cdef unsigned DDS_REQUESTED_INCOMPATIBLE_QOS_STATUS
    cdef unsigned DDS_SAMPLE_LOST_STATUS
    cdef unsigned DDS_SAMPLE_REJECTED_STATUS
    cdef unsigned DDS_DATA_ON_READERS_STATUS
    cdef unsigned DDS_DATA_AVAILABLE_STATUS
    cdef unsigned DDS_LIVELINESS_LOST_STATUS
    cdef unsigned DDS_LIVELINESS_CHANGED_STATUS
    cdef unsigned DDS_PUBLICATION_MATCHED_STATUS
    cdef unsigned DDS_SUBSCRIPTION_MATCHED_STATUS

    # return code functions and macros
    cdef int dds_err_no(int e)
    cdef const char * dds_err_str (int err)
    cdef const char * dds_err_mod_str (int err);

    # basic types
    ctypedef signed char  int8_t
    ctypedef signed short int16_t
    ctypedef signed int   int32_t
    ctypedef signed long  int64_t
    ctypedef unsigned char  uint8_t
    ctypedef unsigned short uint16_t
    ctypedef unsigned int   uint32_t
    ctypedef unsigned long long uint64_t

    # basic DDS type definitions
    ctypedef int64_t dds_duration_t
    ctypedef int64_t dds_time_t
    ctypedef uint64_t dds_instance_handle_t

    cdef int DDS_HANDLE_NIL #0

    cdef dds_duration_t DDS_INFINITY
    cdef int DDS_LENGTH_UNLIMITED
    cdef int DDS_DOMAIN_DEFAULT

    cdef enum dds_presentation_access_scope_kind:
        instance,
        topic,
        group

    ctypedef enum dds_durability_kind_t:
        volatile,
        transient_local,
        transient,
        persistent

    cdef enum dds_ownership_kind:
        shared,
        exclusive

    cdef enum dds_liveliness_kind:
        automatic,
        manual_by_participant,
        manual_by_topic

    cdef enum dds_reliability_kind:
        best_effort,
        reliable

    cdef enum dds_destination_order_kind:
        by_reception_timestamp,
        by_source_timestamp

    cdef enum dds_history_kind:
        keep_last,
        keep_all

    ctypedef enum dds_sample_state_t:
        read = 1,
        not_read = 2

    ctypedef enum dds_view_state_t:
        new = 4,
        not_new = 8

    ctypedef enum dds_instance_state_t:
        alive = 16,
        not_alive_disposed = 32,
        not_alive_no_writers = 64

    ctypedef void *dds_entity_t
    ctypedef void *dds_condition_t
    ctypedef void *dds_waitset_t
    ctypedef int   dds_domainid_t
    ctypedef void *dds_attach_t
    ctypedef struct dds_qos_t:
        pass
    ctypedef struct dds_topic_descriptor_t:
        pass
    ctypedef struct dds_condition_seq:
        uint32_t _length
        dds_condition_t *_buffer
        bint _release

    # status type definitions
    ctypedef enum dds_sample_rejected_status_kind:
        not_rejected,
        rejected_by_instance_limit,
        rejected_by_samples_limit,
        rejected_by_samples_per_instance_limit

    ctypedef struct dds_offered_deadline_missed_status_t:
        uint32_t total_count
        int32_t total_count_change
        dds_instance_handle_t last_instance_handle

    ctypedef struct dds_offered_incompatible_qos_status_t:
        uint32_t total_count
        int32_t total_count_change
        uint32_t last_policy_id

    ctypedef struct dds_publication_matched_status_t:
        uint32_t total_count
        int32_t total_count_change
        uint32_t current_count
        int32_t current_count_change
        dds_instance_handle_t last_subscription_handle

    ctypedef struct dds_liveliness_lost_status_t:
        uint32_t total_count
        int32_t total_count_change

    ctypedef struct dds_subscription_matched_status_t:
        uint32_t total_count
        int32_t total_count_change
        uint32_t current_count
        int32_t current_count_change
        dds_instance_handle_t last_publication_handle

    ctypedef struct dds_sample_rejected_status_t:
        uint32_t total_count
        int32_t total_count_change
        dds_sample_rejected_status_kind last_reason
        dds_instance_handle_t last_instance_handle

    ctypedef struct dds_liveliness_changed_status_t:
        uint32_t alive_count
        uint32_t not_alive_count
        int32_t alive_count_change
        int32_t not_alive_count_change
        dds_instance_handle_t last_publication_handle

    ctypedef struct dds_requested_deadline_missed_status_t:
        uint32_t total_count
        int32_t total_count_change
        dds_instance_handle_t last_instance_handle

    ctypedef struct dds_requested_incompatible_qos_status_t:
        uint32_t total_count
        int32_t total_count_change
        uint32_t last_policy_id

    ctypedef struct dds_sample_lost_status_t:
        uint32_t total_count
        int32_t total_count_change

    ctypedef struct dds_inconsistent_topic_status_t:
        uint32_t total_count
        int32_t total_count_change

    # Status condition getters
    int dds_get_inconsistent_topic_status (dds_entity_t topic, dds_inconsistent_topic_status_t * status)
    int dds_get_publication_matched_status (dds_entity_t writer, dds_publication_matched_status_t * status)
    int dds_get_liveliness_lost_status (dds_entity_t writer, dds_liveliness_lost_status_t * status)
    int dds_get_offered_deadline_missed_status (dds_entity_t writer, dds_offered_deadline_missed_status_t * status)
    int dds_get_offered_incompatible_qos_status (dds_entity_t writer, dds_offered_incompatible_qos_status_t * status)
    int dds_get_subscription_matched_status (dds_entity_t reader, dds_subscription_matched_status_t * status)
    int dds_get_liveliness_changed_status (dds_entity_t reader, dds_liveliness_changed_status_t * status)
    int dds_get_sample_rejected_status (dds_entity_t reader, dds_sample_rejected_status_t * status)
    int dds_get_sample_lost_status (dds_entity_t reader, dds_sample_lost_status_t * status)
    int dds_get_requested_deadline_missed_status (dds_entity_t reader, dds_requested_deadline_missed_status_t * status)
    int dds_get_requested_incompatible_qos_status (dds_entity_t reader, dds_requested_incompatible_qos_status_t * status)

    # Listeners callback function definitions
    ctypedef void (*on_inconsistent_topic_fnc) (dds_entity_t topic, dds_inconsistent_topic_status_t * status)
    ctypedef void (*on_offered_deadline_missed_fnc) (dds_entity_t writer, dds_offered_deadline_missed_status_t * status)
    ctypedef void (*on_offered_incompatible_qos_fnc) (dds_entity_t writer, dds_offered_incompatible_qos_status_t * status)
    ctypedef void (*on_liveliness_lost_fnc) (dds_entity_t writer, dds_liveliness_lost_status_t * status)
    ctypedef void (*on_publication_matched_fnc) (dds_entity_t writer, dds_publication_matched_status_t * status)
    ctypedef void (*on_requested_deadline_missed_fnc) (dds_entity_t reader, dds_requested_deadline_missed_status_t * status)
    ctypedef void (*on_requested_incompatible_qos_fnc) (dds_entity_t reader, dds_requested_incompatible_qos_status_t * status)
    ctypedef void (*on_sample_rejected_fnc) (dds_entity_t reader, dds_sample_rejected_status_t * status)
    ctypedef void (*on_liveliness_changed_fnc) (dds_entity_t reader, dds_liveliness_changed_status_t * status)
    ctypedef void (*on_data_available_fnc) (dds_entity_t reader)
    ctypedef void (*on_subscription_matched_fnc) (dds_entity_t reader, dds_subscription_matched_status_t * status)
    ctypedef void (*on_sample_lost_fnc) (dds_entity_t reader, dds_sample_lost_status_t * status)
    ctypedef void (*on_data_readers_fnc) (dds_entity_t subscriber)

    # Listener definitions
    ctypedef struct dds_topiclistener_t:
        on_inconsistent_topic_fnc on_inconsistent_topic

    ctypedef struct dds_writerlistener_t:
        on_offered_deadline_missed_fnc on_offered_deadline_missed
        on_offered_incompatible_qos_fnc on_offered_incompatible_qos
        on_liveliness_lost_fnc on_liveliness_lost
        on_publication_matched_fnc on_publication_matched

    ctypedef struct dds_readerlistener_t:
        on_requested_deadline_missed_fnc on_requested_deadline_missed
        on_requested_incompatible_qos_fnc on_requested_incompatible_qos
        on_sample_rejected_fnc on_sample_rejected
        on_liveliness_changed_fnc on_liveliness_changed
        on_data_available_fnc on_data_available
        on_subscription_matched_fnc on_subscription_matched
        on_sample_lost_fnc on_sample_lost

    ctypedef struct dds_publisherlistener_t:
        dds_writerlistener_t writerlistener

    ctypedef struct dds_subscriberlistener_t:
        dds_readerlistener_t readerlistener
        on_data_readers_fnc on_data_readers

    ctypedef struct dds_participantlistener_t:
        dds_topiclistener_t topiclistener
        dds_publisherlistener_t publisherlistener
        dds_subscriberlistener_t subscriberlistener

    # Sample info
    ctypedef struct dds_sample_info_t:
        dds_sample_state_t sample_state
        dds_view_state_t view_state
        dds_instance_state_t instance_state
        bint valid_data
        dds_time_t source_timestamp
        dds_instance_handle_t instance_handle
        dds_instance_handle_t publication_handle
        uint32_t disposed_generation_count
        uint32_t no_writers_generation_count
        uint32_t sample_rank
        uint32_t generation_rank
        uint32_t absolute_generation_rank
        dds_time_t reception_timestamp

    # QoS functions
    dds_qos_t * dds_qos_create ()
    void dds_qos_get (dds_entity_t e, dds_qos_t * qos)
    void dds_qos_delete (dds_qos_t *qos)
    void dds_qos_reset (dds_qos_t *qos)
    void dds_qos_copy (dds_qos_t *dst, const dds_qos_t *src)
    void dds_qos_merge (dds_qos_t *dst, const dds_qos_t *src)

    void dds_get_default_participant_qos (dds_qos_t *qos)
    void dds_get_default_topic_qos (dds_qos_t *qos)
    void dds_get_default_publisher_qos (dds_qos_t *qos)
    void dds_get_default_subscriber_qos (dds_qos_t *qos)
    void dds_get_default_writer_qos (dds_qos_t *qos)
    void dds_get_default_reader_qos (dds_qos_t *qos)

    void dds_qset_userdata (dds_qos_t *qos, const void * value, size_t sz)
    void dds_qset_topicdata (dds_qos_t *qos, const void * value, size_t sz)
    void dds_qset_groupdata (dds_qos_t *qos, const void * value, size_t sz)
    void dds_qset_durability (dds_qos_t *qos, dds_durability_kind_t kind)
    void dds_qset_history (dds_qos_t *qos, dds_history_kind kind, int32_t depth)
    void dds_qset_resource_limits (dds_qos_t *qos, int32_t max_samples, int32_t max_instances, int32_t max_samples_per_instance)
    void dds_qset_presentation (dds_qos_t *qos, dds_presentation_access_scope_kind access_scope, bint coherent_access, bint ordered_access)
    void dds_qset_lifespan (dds_qos_t *qos, dds_duration_t lifespan)
    void dds_qset_deadline (dds_qos_t *qos, dds_duration_t deadline)
    void dds_qset_latency_budget (dds_qos_t *qos, dds_duration_t duration)
    void dds_qset_ownership (dds_qos_t *qos, dds_ownership_kind kind)
    void dds_qset_ownership_strength (dds_qos_t *qos, int32_t value)
    void dds_qset_liveliness (dds_qos_t *qos, dds_liveliness_kind kind, dds_duration_t lease_duration)
    void dds_qset_time_based_filter (dds_qos_t *qos, dds_duration_t minimum_separation)
    void dds_qset_partition (dds_qos_t * qos, uint32_t n, const char ** ps)
    void dds_qset_reliability (dds_qos_t *qos, dds_reliability_kind kind, dds_duration_t max_blocking_time)
    void dds_qset_transport_priority (dds_qos_t *qos, int32_t value)
    void dds_qset_destination_order (dds_qos_t *qos, dds_destination_order_kind kind)
    void dds_qset_writer_data_lifecycle (dds_qos_t *qos, bint autodispose_unregistered_instances)
    void dds_qset_reader_data_lifecycle (dds_qos_t *qos, dds_duration_t autopurge_nowriter_samples, dds_duration_t autopurge_disposed_samples_delay)
    void dds_qset_durability_service ( dds_qos_t * qos, dds_duration_t service_cleanup_delay, dds_history_kind
            history_kind, int32_t history_depth, int32_t max_samples, int32_t max_instances, int32_t max_samples_per_instance)
    void dds_qget_userdata (const dds_qos_t *qos, void ** value, size_t *sz)
    void dds_qget_topicdata (const dds_qos_t *qos, void ** value, size_t *sz)
    void dds_qget_groupdata (const dds_qos_t *qos, void ** value, size_t *sz)
    void dds_qget_durability (const dds_qos_t *qos, dds_durability_kind_t *kind)
    void dds_qget_history (const dds_qos_t *qos, dds_history_kind *kind, int32_t * depth)
    void dds_qget_resource_limits (const dds_qos_t *qos, int32_t *max_samples, int32_t *max_instances, int32_t *max_samples_per_instance)
    void dds_qget_presentation (const dds_qos_t *qos, dds_presentation_access_scope_kind *access_scope, bint *coherent_access, bint *ordered_access)
    void dds_qget_lifespan (const dds_qos_t *qos, dds_duration_t *lifespan)
    void dds_qget_deadline (const dds_qos_t *qos, dds_duration_t *deadline)
    void dds_qget_latency_budget (const dds_qos_t *qos, dds_duration_t *duration)
    void dds_qget_ownership (const dds_qos_t *qos, dds_ownership_kind *kind)
    void dds_qget_ownership_strength (const dds_qos_t *qos, int32_t *value)
    void dds_qget_liveliness (const dds_qos_t *qos, dds_liveliness_kind *kind, dds_duration_t *lease_duration)
    void dds_qget_time_based_filter (const dds_qos_t *qos, dds_duration_t *minimum_separation)
    void dds_qget_partition (const dds_qos_t *qos, uint32_t *n, char *** ps)
    void dds_qget_reliability (const dds_qos_t *qos, dds_reliability_kind *kind, dds_duration_t *max_blocking_time)
    void dds_qget_transport_priority (const dds_qos_t *qos, int32_t *value)
    void dds_qget_destination_order (const dds_qos_t *qos, dds_destination_order_kind *value)
    void dds_qget_writer_data_lifecycle (const dds_qos_t *qos, bint * autodispose_unregistered_instances)
    void dds_qget_reader_data_lifecycle (const dds_qos_t *qos, dds_duration_t *autopurge_nowriter_samples, dds_duration_t *autopurge_disposed_samples_delay)
    void dds_qget_durability_service ( const dds_qos_t * qos, dds_duration_t * service_cleanup_delay, dds_history_kind * history_kind,
            int32_t * history_depth, int32_t * max_samples, int32_t * max_instances, int32_t * max_samples_per_instance)

    # Entity operations
    int dds_status_set_enabled (dds_entity_t e, uint32_t mask)
    uint32_t dds_status_changes (dds_entity_t e)

    dds_topic_descriptor_t * dds_topic_descriptor_create(const char *name, const char *keys, const char *spec)
    void dds_topic_descriptor_delete(dds_topic_descriptor_t *descriptor)

    int dds_participant_create(dds_entity_t *pp, dds_domainid_t did, dds_qos_t *qos, dds_participantlistener_t *listener)
    int dds_entity_delete(dds_entity_t e)

    int dds_topic_create(dds_entity_t pp, dds_entity_t *topic, const dds_topic_descriptor_t *descriptor,
            const char *name, const dds_qos_t *qos, const dds_topiclistener_t *listener)
    int dds_subscriber_create(dds_entity_t pp, dds_entity_t *subscriber, const dds_qos_t *qos, const dds_subscriberlistener_t *listener)
    int dds_publisher_create(dds_entity_t pp, dds_entity_t *publisher, const dds_qos_t *qos, const dds_publisherlistener_t *listener)
    int dds_writer_create(dds_entity_t pp_or_pub, dds_entity_t *writer, dds_entity_t topic, const dds_qos_t *qos, const dds_writerlistener_t *listener)
    int dds_write(dds_entity_t wr, const void *data)
    int dds_write_ts(dds_entity_t wr, const void *data, dds_time_t timestamp)
    int dds_instance_dispose(dds_entity_t wr, const void *data)
    int dds_instance_dispose_ts (dds_entity_t wr, const void *data, dds_time_t timestamp)
    int dds_reader_create(dds_entity_t pp_or_sub, dds_entity_t *reader, dds_entity_t topic, const dds_qos_t *qos, const dds_readerlistener_t *listener)
    int dds_read(dds_entity_t rd, void **buf, uint32_t maxs, dds_sample_info_t *si, uint32_t mask)
    int dds_take(dds_entity_t rd, void **buf, uint32_t maxs, dds_sample_info_t *si, uint32_t mask)
    int dds_read_cond(dds_entity_t rd,void ** buf,uint32_t maxs,dds_sample_info_t * si,dds_condition_t cond)
    int dds_take_cond(dds_entity_t rd,void ** buf,uint32_t maxs,dds_sample_info_t * si,dds_condition_t cond)
    int dds_reader_wait_for_historical_data(dds_entity_t reader, dds_duration_t max_wait) nogil

    void dds_return_loan (dds_entity_t rd, void **buf, uint32_t maxs)

    # instance operations
    dds_instance_handle_t dds_instance_register (dds_entity_t wr, const void *data)
    int dds_instance_unregister (dds_entity_t wr, const void * data, dds_instance_handle_t handle)
    int dds_instance_unregister_ts (dds_entity_t wr, const void * data, dds_instance_handle_t handle, dds_time_t timestamp)
    int dds_read_instance(dds_entity_t rd, void ** buf, uint32_t maxs, dds_sample_info_t * si, dds_instance_handle_t handle, uint32_t mask)
    int dds_take_instance(dds_entity_t rd, void ** buf, uint32_t maxs, dds_sample_info_t * si, dds_instance_handle_t handle, uint32_t mask)
    dds_instance_handle_t dds_instance_lookup (dds_entity_t e, const void * data)
    int dds_instance_get_key (dds_entity_t e, dds_instance_handle_t inst, void * data)

    # Condition operations
    bint dds_condition_triggered (dds_condition_t cond)
    dds_condition_t dds_statuscondition_get (dds_entity_t pp)

    dds_condition_t dds_guardcondition_create ()
    void dds_guard_trigger (dds_condition_t guard)
    void dds_guard_reset (dds_condition_t guard)
    dds_condition_t dds_readcondition_create (dds_entity_t rd, uint32_t mask)
    dds_condition_t dds_querycondition_create_sql (dds_entity_t reader, uint32_t mask, const char *expression, const char **parameters, uint32_t maxp)
    void dds_condition_delete (dds_condition_t cond)

    # Topic operations
    dds_entity_t dds_topic_find(dds_entity_t pp, const char * name)
    char * dds_topic_get_name (dds_entity_t topic)
    char * dds_topic_get_type_name (dds_entity_t topic)
    char * dds_topic_get_metadescriptor (dds_entity_t topic)
    char * dds_topic_get_keylist (dds_entity_t topic)

    # Waitset operations
    dds_waitset_t dds_waitset_create ()
    void dds_waitset_get_conditions (dds_waitset_t ws, dds_condition_seq * seq)
    int dds_waitset_delete (dds_waitset_t ws)
    int dds_waitset_attach (dds_waitset_t ws, dds_condition_t e, dds_attach_t x)
    int dds_waitset_detach (dds_waitset_t ws, dds_condition_t e)
    int dds_waitset_wait (dds_waitset_t ws, dds_attach_t *xs, size_t nxs, dds_duration_t reltimeout) nogil
    int dds_waitset_wait_until (dds_waitset_t ws, dds_attach_t *xs, size_t nxs, dds_time_t abstimeout) nogil

    # xml qos provider
    int dds_qosprovider_create(dds_entity_t * qp, const char *uri, const char *profile)
    int dds_qosprovider_get_participant_qos(dds_entity_t qp, dds_qos_t *qos, const char *id)
    int dds_qosprovider_get_topic_qos(dds_entity_t qp, dds_qos_t *qos, const char *id)
    int dds_qosprovider_get_subscriber_qos(dds_entity_t qp, dds_qos_t *qos, const char *id)
    int dds_qosprovider_get_reader_qos(dds_entity_t qp, dds_qos_t *qos, const char *id)
    int dds_qosprovider_get_writer_qos(dds_entity_t qp, dds_qos_t *qos, const char *id)
    int dds_qosprovider_get_publisher_qos(dds_entity_t qp, dds_qos_t *qos, const char *id)

    void dds_free (void * ptr)

# BuiltinTopics topic descriptors
cdef extern from "dds_builtinTopics.h":

    dds_topic_descriptor_t DDS_ParticipantBuiltinTopicData_desc
    dds_topic_descriptor_t DDS_TopicBuiltinTopicData_desc
    dds_topic_descriptor_t DDS_PublicationBuiltinTopicData_desc
    dds_topic_descriptor_t DDS_SubscriptionBuiltinTopicData_desc
    dds_topic_descriptor_t DDS_CMParticipantBuiltinTopicData_desc
    dds_topic_descriptor_t DDS_CMPublisherBuiltinTopicData_desc
    dds_topic_descriptor_t DDS_CMSubscriberBuiltinTopicData_desc
    dds_topic_descriptor_t DDS_CMDataWriterBuiltinTopicData_desc
    dds_topic_descriptor_t DDS_CMDataReaderBuiltinTopicData_desc
    dds_topic_descriptor_t DDS_TypeBuiltinTopicData_desc
