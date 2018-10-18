#ifndef DDS_H
#define DDS_H

/** @file dds.h
 *  @brief Vortex Lite core DDS header
 */

#include <stdbool.h>

/* Sub components */

#include "dds/impl.h"
#include "dds/time.h"
#include "dds/qos.h"
#include "dds/error.h"
#include "dds/status.h"
#include "dds/alloc.h"
#include "os_if.h"

#include "dds_predefTypes.h"

#if defined (__cplusplus)
extern "C" {
#endif

#ifdef OSPL_BUILD_DCPSC99
#define DDS_EXPORT OS_API_EXPORT
#else
#define DDS_EXPORT OS_API_IMPORT
#endif


/**
 * Description : Initialization function, called from main. This operation
 * initializes all the required DDS resources,
 * handles configuration of domainid based on the input passed, parses and
 * configures middleware from a xml file and initializes required resources.
 *
 * Arguments :
 *   -# argc, argv - specifies the domainId
 *   -# Returns 0 on success or a non-zero error status
 */

DDS_EXPORT int dds_init (int argc, char ** argv);

/* Finalization function, called from main */

/**
 * Description : Finalization function, called from main. This operation
 * releases all the resources used by DDS.
 *
 * Arguments :
 *   -# None
 */
DDS_EXPORT void dds_fini (void);

/**
 * Description : Returns the default DDS domain id. This can be configured
 * in xml or set as an evironment variable (LITE_DOMAIN).
 *
 * Arguments :
 *   -# None
 *   -# Returns the default domain id
 */
DDS_EXPORT dds_domainid_t dds_domain_default (void);

/** @name Communication Status definitions
  @{**/
#ifndef DDS_INCONSISTENT_TOPIC_STATUS
#define DDS_INCONSISTENT_TOPIC_STATUS          1u
#define DDS_OFFERED_DEADLINE_MISSED_STATUS     2u
#define DDS_REQUESTED_DEADLINE_MISSED_STATUS   4u
#define DDS_OFFERED_INCOMPATIBLE_QOS_STATUS    32u
#define DDS_REQUESTED_INCOMPATIBLE_QOS_STATUS  64u
#define DDS_SAMPLE_LOST_STATUS                 128u
#define DDS_SAMPLE_REJECTED_STATUS             256u
#define DDS_DATA_ON_READERS_STATUS             512u
#define DDS_DATA_AVAILABLE_STATUS              1024u
#define DDS_LIVELINESS_LOST_STATUS             2048u
#define DDS_LIVELINESS_CHANGED_STATUS          4096u
#define DDS_PUBLICATION_MATCHED_STATUS         8192u
#define DDS_SUBSCRIPTION_MATCHED_STATUS        16384u
#endif
/** @}*/

/**
 * dds_sample_state_t
 * \brief defines the state for a data value
 * -# DDS_SST_READ - DataReader has already accessed the sample by read
 * -# DDS_SST_NOT_READ - DataReader has not accessed that sample before
 */
typedef enum dds_sample_state
{
  DDS_SST_READ = DDS_READ_SAMPLE_STATE,
  DDS_SST_NOT_READ = DDS_NOT_READ_SAMPLE_STATE
}
dds_sample_state_t;

/**
 * dds_view_state_t
 * \brief defines the view state of an instance relative to the samples
 * -# DDS_VST_NEW - DataReader is accessing the sample for the first time when the
 *                  instance is alive
 * -# DDS_VST_OLD - DataReader has accesssed the sample before
 */
typedef enum dds_view_state
{
  DDS_VST_NEW = DDS_NEW_VIEW_STATE,
  DDS_VST_OLD = DDS_NOT_NEW_VIEW_STATE
}
dds_view_state_t;

/**
 * dds_instance_state_t
 * \brief defines the state of the instance
 * -# DDS_IST_ALIVE - Samples received for the instance from the live data writers
 * -# DDS_IST_NOT_ALIVE_DISPOSED - Instance was explicitly disposed by the data writer
 * -# DDS_IST_NOT_ALIVE_NO_WRITERS - Instance has been declared as not alive by data reader
 *                                   as there are no live data writers writing that instance
 */
typedef enum dds_instance_state
{
  DDS_IST_ALIVE = DDS_ALIVE_INSTANCE_STATE,
  DDS_IST_NOT_ALIVE_DISPOSED = DDS_NOT_ALIVE_DISPOSED_INSTANCE_STATE,
  DDS_IST_NOT_ALIVE_NO_WRITERS = DDS_NOT_ALIVE_NO_WRITERS_INSTANCE_STATE
}
dds_instance_state_t;

/**
 * Structure dds_sample_info_t - contains information about the associated data value
 * -# sample_state - \ref dds_sample_state_t
 * -# view_state - \ref dds_view_state_t
 * -# instance_state - \ref dds_instance_state_t
 * -# valid_data - indicates whether there is a data associated with a sample
 *    - true, indicates the data is valid
 *    - false, indicates the data is invalid, no data to read
 * -# source_timestamp - timestamp of a data instance when it is written
 * -# instance_handle - handle to the data instance
 * -# publication_handle - handle to the publisher
 * -# disposed_generation_count - count of instance state change from
 *    NOT_ALIVE_DISPOSED to ALIVE
 * -# no_writers_generation_count - count of instance state change from
 *    NOT_ALIVE_NO_WRITERS to ALIVE
 * -# sample_rank - indicates the number of samples of the same instance
 *    that follow the current one in the collection
 * -# generation_rank - difference in generations between the sample and most recent sample
 *    of the same instance that appears in the returned collection
 * -# absolute_generation_rank - difference in generations between the sample and most recent sample
 *    of the same instance when read/take was called
 * -# reception_timestamp - timestamp of a data instance when it is added to a read queue
 */
typedef struct dds_sample_info
{
  dds_sample_state_t sample_state;
  dds_view_state_t view_state;
  dds_instance_state_t instance_state;
  bool valid_data;
  dds_time_t source_timestamp;
  dds_instance_handle_t instance_handle;
  dds_instance_handle_t publication_handle;
  uint32_t disposed_generation_count;
  uint32_t no_writers_generation_count;
  uint32_t sample_rank;
  uint32_t generation_rank;
  uint32_t absolute_generation_rank;
  dds_time_t reception_timestamp; /* NOTE: VLite extension */
}
dds_sample_info_t;

/* All entities are represented by a process-private handle, with one
 * call to delete an entity and all entities it logically contains.
 * That is, it is equivalent to combination of
 * delete_contained_entities and delete_xxx in the DCPS API.
 */

/**
 * Description : Recursively deletes all the contained entities and deletes
 * the entity. Before deleting an entity with registered listeners, it's
 * status should be set to zero to disable callbacks.
 *
 * Arguments :
 *   -# e Entity to delete
 */
DDS_EXPORT void dds_entity_delete (dds_entity_t e);

/* All entities have a set of "status conditions" (following the DCPS
 * spec), read peeks, take reads & resets (analogously to read & take
 * operations on reader). The "mask" allows operating only on a subset
 * of the statuses. Enabled status analogously to DCPS spec.
 */


/**
 * Description : Read the status(es) set for the entity based on the enabled
 * status and mask set. This operation does not clear the read status(es).
 *
 * Arguments :
 *   -# e Entity on which the status has to be read
 *   -# status Returns the status set on the entity, based on the enabled status
 *   -# mask Filter the status condition to be read (can be NULL)
 *   -# Returns 0 on success, or a non-zero error value if the mask does not
 *      correspond to the entity
 */
DDS_EXPORT int dds_status_read (dds_entity_t e, uint32_t * status, uint32_t mask);

/**
 * Description : Read the status(es) set for the entity based on the enabled
 * status and mask set. This operation clears the status set after reading.
 *
 * Arguments :
 *   -# e Entity on which the status has to be read
 *   -# status Returns the status set on the entity, based on the enabled status
 *   -# mask Filter the status condition to be read (can be NULL)
 *   -# Returns 0 on success, or a non-zero error value if the mask does not
 *      correspond to the entity
 */
DDS_EXPORT int dds_status_take (dds_entity_t e, uint32_t * status, uint32_t mask);

/**
 * Description : Returns the status changes since they were last read.
 *
 * Arguments :
 *   -# e Entity on which the statuses are read
 *   -# Returns the curent set of triggered statuses.
 */
DDS_EXPORT uint32_t dds_status_changes (dds_entity_t e);

/**
 * Description : This operation returns the status enabled on the entity
 *
 * Arguments :
 *   -# e Entity to get the status
 *   -# Returns the status that are enabled for the entity
 */
DDS_EXPORT uint32_t dds_status_get_enabled (dds_entity_t e);


/**
 * Description : This operation enables the status(es) based on the mask set
 *
 * Arguments :
 *   -# e Entity to enable the status
 *   -# mask Status value that indicates the status to be enabled
 *   -# Returns 0 on success, or a non-zero error value indicating failure if the mask
 *      does not correspond to the entity.
 */
DDS_EXPORT int dds_status_set_enabled (dds_entity_t e, uint32_t mask);

/* Almost all entities have get/set qos operations defined on them,
 * again following the DCPS spec. But unlike the DCPS spec, the
 * "present" field in qos_t allows one to initialise just the one QoS
 * one wants to set & pass it to set_qos.
 */

/**
 * Description : This operation allows access to the existing set of QoS policies
 * for the entity.
 *
 * Arguments :
 *   -# e Entity on which to get qos
 *   -# qos pointer to the qos structure that returns the set policies.
 */
DDS_EXPORT void dds_qos_get (dds_entity_t e, dds_qos_t * qos);


/**
 * Description : This operation sets the QoS policies of the entity at runtime
 * This call replaces the exisiting set of policies, if already available.
 *
 * Arguments :
 *   -# e Entity to apply QoS
 *   -# qos pointer to the qos structure with a set of policies to be applied
 *   -# Returns 0 on success, or a non-zero error value to indicate immutable QoS
 *      is set or the values set are incorrect, which cannot be applied.
 *
 * NOTE: Latency Budget and Ownership Strength are changeable QoS that can be set for LITE
 */
DDS_EXPORT int dds_qos_set (dds_entity_t e, const dds_qos_t * qos);

/* Get or set listener associated with an entity, type of listener
 * provided much match type of entity.
 */

/**
 * Description : This operation allows access to the existing listeners attached to
 * the entity.
 *
 * Arguments :
 *   -# e Entity to get the listener set
 *   -# listener pointer to the listener set on the entity
 */
DDS_EXPORT void dds_listener_get (dds_entity_t e, dds_listener_t listener);


/**
 * Description : This operation installs the listener on the entity.
 * If a listener is set, Call to this will replace with the new one.
 *
 * Arguments :
 *   -# e Entity to set listener
 *   -# listener pointer to the listener (can be NULL)
 *
 */
DDS_EXPORT void dds_listener_set (dds_entity_t e, const dds_listener_t listener);

/* Creation functions for various entities. Creating a subscriber or
 * publisher is optional: if one creates a reader as a descendant of a
 * participant, it is as if a subscriber is created specially for
 * that reader.
 *
 * QoS default values are those of the DDS specification, but the
 * inheritance rules are different:
 *
 *   * publishers and subscribers inherit from the participant QoS
 *   * readers and writers always inherit from the topic QoS
 *   * the QoS's present in the "qos" parameter override the inherited values
 */

/**
 * Description : Creates a new instance of a DDS participant in a domain. If domain
 * is set (not DDS_DOMAIN_DEFAULT) then it must match if the domain has also
 * been configured or an error status will be returned. Currently only a single domain
 * can be configured by setting the environment variable LITE_DOMAIN, if this is not set
 * the the default domain is 0. Valid values for domain id are between 0 and 230.
 *
 * Arguments :
 *   -# pp The created participant entity
 *   -# domain The domain in which to create the participant (can be DDS_DOMAIN_DEFAULT)
 *   -# qos The QoS to set on the new participant (can be NULL)
 *   -# listener Any listener functions associated with the new participant (can be NULL)
 *   -# mask Communication status notification mask
 *   -# Returns a status, 0 on success or non-zero value to indicate an error
 */
DDS_EXPORT int dds_participant_create
(
  dds_entity_t * pp,
  const dds_domainid_t domain,
  const dds_qos_t * qos,
  const dds_participantlistener_t * listener
);

/**
 * Description : Returns the participant for an entity.
 *
 * Arguments :
 *   -# entity The entity
 *   -# Returns The participant
 */
DDS_EXPORT dds_entity_t dds_participant_get (dds_entity_t entity);

/**
 * Description : Returns the domain id for a participant.
 *
 * Arguments :
 *   -# pp The participant entity
 *   -# Returns The participant domain id
 */
DDS_EXPORT dds_domainid_t dds_participant_get_domain_id (dds_entity_t pp);

/**
 * Description : Returns a participant created on a domain. Note that if
 * multiple participants have been created on the same domain then the first
 * found is returned.
 *
 * Arguments :
 *   -# domain_id The domain id
 *   -# Returns Participant for domain
 */
DDS_EXPORT dds_entity_t dds_participant_lookup (dds_domainid_t domain_id);

/**
 * Description : Creates a topic_descriptor from an XML type definition
 * and corresponding key values. The returned topic_descriptor can be used
 * to create a topic that can be used to create generic "typeless"
 * readers or writers.
 * This provides an application to read or write a topic without the need
 * to generate code from an idl definition.
 * The returned topic_descriptor must be freed using either the function
 * dds_topic_descriptor_delete or by using dds_free.
 * When a generic reader is used there is the restriction that the
 * read/take operations should use the loan mechanism to ensure that
 * no memory leakage occurs.
 *
 * Arguments :
 *   -# name The name of the type
 *   -# keys The keys associated with the topic type
 *   -# spec The XML description of the topic type
 *   -# Returns an dds_topic_descriptor_t
 */
DDS_EXPORT dds_topic_descriptor_t * dds_topic_descriptor_create(
  const char *name,
  const char *keys,
  const char *spec);

/**
 * Description : Delete an allocated topic_descriptor.
 *
 * Arguments :
 *   -# descriptor The topic_descriptor to free
 */
DDS_EXPORT void dds_topic_descriptor_delete(dds_topic_descriptor_t *descriptor);

/**
 * Description : Creates a new DDS topic. The type name for the topic
 * is taken from the generated descriptor. Topic matching is done on a
 * combination of topic name and type name.
 *
 * Arguments :
 *   -# pp The participant on which the topic is being created
 *   -# topic The created topic
 *   -# descriptor The IDL generated topic descriptor
 *   -# name The name of the created topic
 *   -# qos The QoS to set on the new topic (can be NULL)
 *   -# listener Any listener functions associated with the new topic (can be NULL)
 *   -# Returns a status, 0 on success or non-zero value to indicate an error
 */
DDS_EXPORT int dds_topic_create
(
  dds_entity_t pp,
  dds_entity_t * topic,
  const dds_topic_descriptor_t * descriptor,
  const char * name,
  const dds_qos_t * qos,
  const dds_topiclistener_t * listener
);

/**
 * Description : Finds a named topic. Returns NULL if does not exist.
 * The returned topic should be released with dds_entity_delete.
 *
 * Arguments :
 *   -# pp The participant on which to find the topic
 *   -# name The name of the topic to find
 *   -# Returns a topic, NULL if could not be found or error
 */
DDS_EXPORT dds_entity_t dds_topic_find
(
  dds_entity_t pp,
  const char * name
);

/**
 * Description : Returns a topic name.
 *
 * Arguments :
 *   -# topic The topic
 *   -# Returns The topic name or NULL to indicate an error
 */
DDS_EXPORT char * dds_topic_get_name (dds_entity_t topic);

/**
 * Description : Returns a topic type name.
 *
 * Arguments :
 *   -# topic The topic
 *   -# Returns The topic type name or NULL to indicate an error
 */
DDS_EXPORT char * dds_topic_get_type_name (dds_entity_t topic);

/**
 * Description : Return a topic meta descriptor (XML). Returned value must be freed with dds_free
 *
 * Arguments :
 *   -# topic The topic
 *   -# Returns The XML topic descriptor or NULL to indicate an error
 */
DDS_EXPORT char * dds_topic_get_metadescriptor (dds_entity_t topic);

/**
 * Description : Return the keys of a topic. Returned value must be freed with dds_free
 *
 * Arguments :
 *   -# topic The topic
 *   -# Returns A comma separated string of the key fields or NULL to indicate an error
 */
DDS_EXPORT char * dds_topic_get_keylist (dds_entity_t topic);

typedef bool (*dds_topic_filter_fn) (const void * sample);

/**
 * Description : Sets a filter on a topic.
 *
 * Arguments :
 *   -# topic The topic on which the content filter is set
 *   -# filter The filter function used to filter topic samples
 */
DDS_EXPORT void dds_topic_set_filter (dds_entity_t topic, dds_topic_filter_fn filter);

/**
 * Description : Gets a topic's filter.
 *
 * Arguments :
 *   -# topic The topic from which to get the filter
 *   -# Returns The topic filter
 */
DDS_EXPORT dds_topic_filter_fn dds_topic_get_filter (dds_entity_t topic);

/**
 * Description : Creates a new instance of a DDS subscriber
 *
 * Arguments :
 *   -# pp The participant on which the subscriber is being created
 *   -# subscriber The created subscriber entity
 *   -# qos The QoS to set on the new subscriber (can be NULL)
 *   -# listener Any listener functions associated with the new subscriber (can be NULL)
 *   -# Returns a status, 0 on success or non-zero value to indicate an error
 */
DDS_EXPORT int dds_subscriber_create
(
  dds_entity_t pp,
  dds_entity_t * subscriber,
  const dds_qos_t * qos,
  const dds_subscriberlistener_t * listener
);

/**
 * Description : Creates a new instance of a DDS publisher
 *
 * Arguments :
 *   -# pp The participant on which the publisher is being created
 *   -# publisher The created publisher entity
 *   -# qos The QoS to set on the new publisher (can be NULL)
 *   -# listener Any listener functions associated with the new publisher (can be NULL)
 *   -# Returns a status, 0 on success or non-zero value to indicate an error
 */
DDS_EXPORT int dds_publisher_create
(
  dds_entity_t pp,
  dds_entity_t *  publisher,
  const dds_qos_t * qos,
  const dds_publisherlistener_t * listener
);

/**
 * Description : Creates a new instance of a DDS reader
 *
 * Arguments :
 *   -# pp_or_sub The participant or subscriber on which the reader is being created
 *   -# reader The created reader entity
 *   -# topic The topic to read
 *   -# qos The QoS to set on the new reader (can be NULL)
 *   -# listener Any listener functions associated with the new reader (can be NULL)
 *   -# Returns a status, 0 on success or non-zero value to indicate an error
 */
DDS_EXPORT int dds_reader_create
(
  dds_entity_t pp_or_sub,
  dds_entity_t * reader,
  dds_entity_t topic,
  const dds_qos_t * qos,
  const dds_readerlistener_t * listener
);

/**
 * Description : The operation blocks the calling thread until either all “historical” data is
 * received, or else the duration specified by the max_wait parameter elapses, whichever happens
 * first. A return value of 0 indicates that all the “historical” data was received; a return
 * value of TIMEOUT indicates that max_wait elapsed before all the data was received.
 *
 * Arguments :
 *   -# reader The reader on which to wait for historical data
 *   -# max_wait How long to wait for historical data before time out
 *   -# Returns a status 0 on success, TIMEOUT on timeout and a negative value to indicate error
 */

DDS_EXPORT int dds_reader_wait_for_historical_data
(
  dds_entity_t reader,
  dds_duration_t max_wait
);

typedef bool (*dds_querycondition_filter_fn) (const void * sample);

/**
 * Description : Create a QueryCondtiion associated with a reader.
 *      Based on the mask value set, and the return value of the filter the readcondition gets triggered when
 *      data is available on the reader.
 *
 * Arguments :
 *   -# reader Reader entity on which the condition is created
 *   -# mask The sample_state, instance_state and view_state of the sample
 *   -# filter The filter function for the query
 *   -# Returns Status, 0 on success or non-zero value to indicate an error
 */
DDS_EXPORT dds_condition_t dds_querycondition_create
(
  dds_entity_t reader,
  uint32_t mask,
  dds_querycondition_filter_fn filter
);


/**
 * Description : Create a QueryCondtiion associated with a reader.
 *      Based on the mask value set, and the return value of the filter the readcondition gets triggered when
 *      data is available on the reader.
 *
 * Arguments :
 *   -# reader Reader entity on which the condition is created
 *   -# mask The sample_state, instance_state and view_state of the sample
 *   -# expressiom The sql where expression for the query
 *   -# parameters The parameter values used in the expression
 *   -# maxp The number of parameters supplied
 *   -# Returns Status, 0 on success or non-zero value to indicate an error
 */
DDS_EXPORT dds_condition_t dds_querycondition_create_sql
(
  dds_entity_t reader,
  uint32_t mask,
  const char *expression,
  const char **parameters,
  uint32_t maxp
);


/**
 * Description : Creates a new instance of a DDS writer
 *
 * Arguments :
 *   -# pp_or_pub The participant or publisher on which the writer is being created
 *   -# writer The created writer entity
 *   -# topic The topic to write
 *   -# qos The QoS to set on the new writer (can be NULL)
 *   -# listener Any listener functions associated with the new writer (can be NULL)
 *   -# Returns a status, 0 on success or non-zero value to indicate an error
 */
DDS_EXPORT int dds_writer_create
(
  dds_entity_t pp_or_pub,
  dds_entity_t * writer,
  dds_entity_t topic,
  const dds_qos_t * qos,
  const dds_writerlistener_t * listener
);

/* Writing data (and variants of it) is straightforward. The first set
 * is equivalent to the second set with -1 passed for "tstamp",
 * meaning, substitute the result of a call to time(). The dispose
 * and unregister operations take an object of the topic's type, but
 * only touch the key fields; the remained may be undefined.
 */

/**
 * Description : Registers an instance with a key value to the data writer
 *
 * Arguments :
 *   -# wr The writer to which instance has be associated
 *   -# data Instance with the key value
 *   -# Returns an instance handle that could be used for successive write & dispose operations or
 *      NULL, if handle is not allocated
 */
DDS_EXPORT dds_instance_handle_t dds_instance_register (dds_entity_t wr, const void *data);

/**
 * Description : Unregisters an instance with a key value from the data writer. Instance can be identified
 *               either from data sample or from instance handle (at least one must be provided).
 *
 * Arguments :
 *   -# wr The writer to which instance is associated
 *   -# data Instance with the key value (can be NULL if handle set)
 *   -# handle Instance handle (can be DDS_HANDLE_NIL if data set)
 *   -# Returns 0 on success, or non-zero value to indicate an error
 *
 * Note : If an unregistered key ID is passed as instance data, an error is logged and not flagged as return value
 */
DDS_EXPORT int dds_instance_unregister (dds_entity_t wr, const void * data, dds_instance_handle_t handle);

  /**
 * Description : Unregisters an instance with a key value from the data writer. Instance can be identified
 *               either from data sample or from instance handle (at least one must be provided).
 *
 * Arguments :
 *   -# wr The writer to which instance is associated
 *   -# data Instance with the key value (can be NULL if handle set)
 *   -# handle Instance handle (can be DDS_HANDLE_NIL if data set)
 *   -# timestamp used at registration.
 *   -# Returns 0 on success, or non-zero value to indicate an error
 *
 * Note : If an unregistered key ID is passed as instance data, an error is logged and not flagged as return value
 */
  DDS_EXPORT int dds_instance_unregister_ts (dds_entity_t wr, const void * data, dds_instance_handle_t handle, dds_time_t timestamp);

/**
 * Description : Write the keyed data passed, and delete the data instance
 *
 * Arguments :
 *   -# wr The writer to which instance is associated
 *   -# data Instance with the key value
 *   -# Returns 0 on success, or non-zero value to indicate an error
 */
DDS_EXPORT int dds_instance_writedispose (dds_entity_t wr, const void *data);

/**
 * Description : Write the keyed data passed with the source timestamp, and delete the data instance
 *
 * Arguments :
 *   -# wr The writer to which instance is associated
 *   -# data Instance with the key value
 *   -# tstamp Source timestamp
 *   -# Returns 0 on success, or non-zero value to indicate an error
 */
DDS_EXPORT int dds_instance_writedispose_ts (dds_entity_t wr, const void *data, dds_time_t tstamp);

/**
 * Description : Delete an instance, identified by the data passed.
 *
 * Arguments :
 *   -# wr The writer to which instance is associated
 *   -# data Instance with the key value, used to get identify the instance
 *   -# Returns 0 on success, or non-zero value to indicate an error
 *
 * Note : If an invalid key ID is passed as instance data, an error is logged and not flagged as return value
 */
DDS_EXPORT int dds_instance_dispose (dds_entity_t wr, const void *data);

/**
 * Description : Delete an instance, identified by the and source timestamp
 *               passed to reader as part of SampleInfo.
 *
 * Arguments :
 *   -# wr The writer to which instance is associated
 *   -# data Instance with the key value, used to get identify the instance
 *   -# tstamp Source Timestamp
 *   -# Returns 0 on success, or non-zero value to indicate an error
 */
DDS_EXPORT int dds_instance_dispose_ts (dds_entity_t wr, const void *data, dds_time_t tstamp);

/**
 * Description : Write the value of a data instance. With this API, the value of the source timestamp
 *               is automatically made available to the data reader by the service
 *
 * Arguments :
 *   -# wr The writer entity
 *   -# data value to be written
 *   -# Returns 0 on success, or non-zero value to indicate an error
 */
DDS_EXPORT int dds_write (dds_entity_t wr, const void *data);

/**
 * Description : Write the value of a data instance along with the source timestamp passed.
 *
 * Arguments :
 *   -# wr The writer entity
 *   -# data value to be written
 *   -# tstamp source timestamp
 *   -# Returns 0 on success, or non-zero value to indicate an error
 */
DDS_EXPORT int dds_write_ts (dds_entity_t wr, const void *data, dds_time_t tstamp);

/**
 * Description : Flushes all the samples from the write cache.
 *               By default, data sent by write API, is queued in the write cache
 *
 * Arguments :
 *   -# wr The writer entity
 */
DDS_EXPORT void dds_write_flush (dds_entity_t wr);

/* Waitsets allow waiting for an event on some of any set of entities
 * (all can in principle be waited for via their status conditions;
 * the "enabled" statuses are the only ones considered). Then there
 * are the "guard" and "read" conditions, both following the DCPS
 * spec.
 *
 * The "guard" is a simple application-controlled entity with a state
 * consisting of a single status condition, TRIGGERED.
 *
 * The "read" condition allows specifying which samples are of
 * interest in a data reader's history. The application visible state
 * of a read (or query) condition consists of a single status condition,
 * TRIGGERED. This status changes based on the contents of the history
 * cache of the associated data reader.
 *
 * The DCPS "query" condition is not currently supported.
 */

/**
 * Description : Return the status condition set by an entity
 *
 * Arguments :
 *   -# pp - The entity which has a status condition
 *   -# Returns status condition enabled by an entity
 */
DDS_EXPORT dds_condition_t dds_statuscondition_get (dds_entity_t pp);


/**
 * Description : Create a specific condition.
 *               This is mainly used to provide the means to manually wakeup a waitset.
 *
 * Arguments :
 *   -# Returns a pointer to the created condition
 */
DDS_EXPORT dds_condition_t dds_guardcondition_create (void);


/**
 * Description : Create a ReadCondition associated to a reader.
 *               Based on the mask value set, the readcondition gets triggered when
 *               data is available on the reader.
 *
 * Arguments :
 *   -# rd Reader entity on which the condition is created
 *   -# mask set the sample_state, instance_state and view_state of the sample
 */
DDS_EXPORT dds_condition_t dds_readcondition_create (dds_entity_t rd, uint32_t mask);

/**
 * Description : Deletes the condition. The condition is detached from any waitsets
 *               to which it is attached.
 *
 * Arguments :
 *   -# cond pointer of a guard condition or readcondition
 */
DDS_EXPORT void dds_condition_delete (dds_condition_t cond);

/* Guard conditions may be triggered or not. The status of a guard condition
 * can always be retrieved via the dds_condition_triggered function. To trigger
 * or reset a guard condition it must first be associated with a waitset or
 * an error status will be returned.
 */

/**
 * Description : Sets the trigger_value associated with a guard condition
 *               The guard condition should be associated with a waitset, before
 *               setting the trigger value.
 *
 * Arguments :
 *   -# guard pointer to the condition to be triggered
 */
DDS_EXPORT void dds_guard_trigger (dds_condition_t guard);

/**
 * Description : Resets the trigger_value associated with a guard condition
 *
 * Arguments :
 *   -# guard pointer to the condition to be reset
 */
DDS_EXPORT void dds_guard_reset (dds_condition_t guard);

/**
 * Description : Check the trigger_value associated with a condition
 *
 * Arguments :
 *   -# pointer to the condition to evaluate
 *   -# Returns true if the condition is already triggered, else returns false.
 */
DDS_EXPORT bool dds_condition_triggered (dds_condition_t guard);

/* Entities can be attached to a waitset or removed from a waitset (in
 * an NxM relationship, but each entity can be in one waitset only
 * once), the "x" value is what is returned by "wait" when the entity
 * represented by handle e triggers.
 */

typedef void * dds_attach_t;

/**
 * Description : Create a waitset and allocate the resources required
 *
 * Arguments :
 *   -# Returns a pointer to a waitset created
 */
DDS_EXPORT dds_waitset_t dds_waitset_create (void);

/**
 * Description : Get the conditions associated with a waitset. The sequence of
 * returned conditions is only valid as long as the waitset in not modified.
 * The returned sequence buffer is a copy and should be freed.
 *
 * Arguments :
 *   -# ws The waitset
 *   -# seq The sequence of returned conditions
 */
DDS_EXPORT void dds_waitset_get_conditions (dds_waitset_t ws, dds_condition_seq * seq);

/**
 * Description : Deletes the waitset, detaching any attached conditions.
 *
 * Arguments :
 *   -# ws pointer to a waitset
 *   -# Returns 0 on success, else non-zero indicating an error
 */
DDS_EXPORT int dds_waitset_delete (dds_waitset_t ws);


/**
 * Description : Associate a status, guard or read condition with a waitset.
 *               Multiple conditions could be attached to a single waitset.
 *
 * Arguments :
 *   -# ws pointer to a waitset
 *   -# e pointer to a condition to wait for the trigger value
 *   -# x attach condition, could be used to know the reason for the waitset to unblock (can be NULL)
 *   -# Returns 0 on success, else non-zero indicating an error
 */
DDS_EXPORT int dds_waitset_attach (dds_waitset_t ws, dds_condition_t e, dds_attach_t x);


/**
 * Description : Disassociate the condition attached with a waitset.
 *               Number of call(s) to detach from the conditions should match attach call(s).
 *
 * Arguments :
 *   -# ws pointer to a waitset
 *   -# e pointer to a condition to wait for the trigger value
 *   -# Returns 0 on success, else non-zero indicating an error
 */
DDS_EXPORT int dds_waitset_detach (dds_waitset_t ws, dds_condition_t e);

/* The "dds_waitset_wait" operation blocks until the some of the
 * attached entities has an enabled and set status condition, or
 * "reltimeout" has elapsed. The "dds_waitset_wait_until" operation
 * is the same as the "dds_wait" except that it takes an absolute timeout.
 *
 * Upon successful return, the array "xs" is filled with 0 < M <= nxs
 * values of the "x"s corresponding to the triggering entities, as
 * specified in attach_to_waitset, and M is returned. In case of a
 * time out, the return value is 0.
 *
 * Deleting the waitset while the application is blocked results in an
 * error code (i.e. < 0) returned by "wait".
 *
 * Multiple threads may block on a single waitset at the same time;
 * the calls are entirely independent.
 *
 * An empty waitset never triggers (i.e., dds_waitset_wait on an empty
 * waitset is essentially equivalent to dds_sleepfor).
 */

/**
 * Description : This API is used to block the current executing thread until some of the
 *               attached condition(s) is triggered or 'reltimeout' has elapsed.
 *               On successful return, the array xs is filled with the value corresponding
 *               to triggered entities as specified in the attach, and nxs with count.
 *
 *
 * Arguments :
 *   -# ws pointer to a waitset
 *   -# xs pointer to an array of attached_conditions based on the conditions associated with a waitset (can be NULL)
 *   -# nxs number of attached conditions (can be zero)
 *   -# reltimeout timeout value associated with a waitset (can be INFINITY or some value)
 *   -# Returns 0 on timeout, else number of signaled waitset conditions
 */
DDS_EXPORT int dds_waitset_wait (dds_waitset_t ws, dds_attach_t *xs, size_t nxs, dds_duration_t reltimeout);


/**
 * Description : This API is used to block the current executing thread until some of the
 *               attached condition(s) is triggered or 'abstimeout' has elapsed.
 *               On successful return, the array xs is filled with the value corresponding
 *               to triggered entities as specified in the attach, and nxs with count.
 *
 * Arguments :
 *   -# ws pointer to a waitset
 *   -# xs pointer to an array of attached_conditions based on the conditions associated with a waitset (can be NULL)
 *   -# nxs number of attached conditions (can be NULL)
 *   -# abstimeout absolute timeout value associated with a waitset (can be INFINITY or some value)
 *   -# Returns 0 if unblocked due to timeout, else number of the waitset conditions that resulted to unblock
 */
DDS_EXPORT int dds_waitset_wait_until (dds_waitset_t ws, dds_attach_t *xs, size_t nxs, dds_time_t abstimeout);

/* There are a number of read and take variations.
 *
 * Return value is the number of elements returned. "max_samples"
 * should have the same type, as one can't return more than MAX_INT
 * this way, anyway. X, Y, CX, CY return to the various filtering
 * options, see the DCPS spec.
 *
 * O ::= read | take
 *
 * X             => CX
 * (empty)          (empty)
 * _next_instance   instance_handle_t prev
 *
 * Y             => CY
 * (empty)          uint32_t mask
 * _cond            cond_t cond -- refers to a read condition (or query if implemented)
 */

/**
 * Description : Access the collection of data values (of same type) and sample info from the
 *               data reader based on the mask set.
 *               Return value provides information about number of samples read, which will
 *               be <= maxs. Based on the count, the buffer will contain data to be read only
 *               when valid_data bit in sample info structure is set.
 *               The buffer required for data values, could be allocated explicitly or can
 *               use the memory from data reader to prevent copy. In the latter case, buffer and
 *               sample_info should be returned back, once it is no longer using the Data.
 *               Data values once read will remain in the buffer with the sample_state set to READ
 *               and view_state set to NOT_NEW.
 *
 * Arguments :
 *   -# rd Reader entity
 *   -# buf an array of pointers to samples into which data is read (pointers can be NULL)
 *   -# maxs maximum number of samples to read
 *   -# si pointer to an array of \ref dds_sample_info_t returned for each data value
 *   -# mask filter the data value based on the set sample, view and instance state
 *   -# Returns the number of samples read, 0 indicates no data to read.
 */
DDS_EXPORT int dds_read
(
  dds_entity_t rd,
  void ** buf,
  uint32_t maxs,
  dds_sample_info_t * si,
  uint32_t mask
);

/**
 * Description : Implements the same functionality as dds_read, except that only data
 *               scoped to the provided instance handle is read.
 *
 * Arguments :
 *   -# rd Reader entity
 *   -# buf an array of pointers to samples into which data is read (pointers can be NULL)
 *   -# maxs maximum number of samples to read
 *   -# si pointer to an array of \ref dds_sample_info_t returned for each data value
 *   -# handle the instance handle identifying the instance from which to read
 *   -# mask filter the data value based on the set sample, view and instance state
 *   -# Returns the number of samples read, 0 indicates no data to read.
 */
DDS_EXPORT int dds_read_instance
(
  dds_entity_t rd,
  void ** buf,
  uint32_t maxs,
  dds_sample_info_t * si,
  dds_instance_handle_t handle,
  uint32_t mask
);

/**
 * Description : Access the collection of data values (of same type) and sample info from the data reader
 *               based on the criteria specified in the read condition.
 *               Read condition must be attached to the data reader before associating with data read.
 *               Return value provides information about number of samples read, which will
 *               be <= maxs. Based on the count, the buffer will contain data to be read only
 *               when valid_data bit in sample info structure is set.
 *               The buffer required for data values, could be allocated explicitly or can
 *               use the memory from data reader to prevent copy. In the latter case, buffer and
 *               sample_info should be returned back, once it is no longer using the Data.
 *               Data values once read will remain in the buffer with the sample_state set to READ
 *               and view_state set to NOT_NEW.
 *
 * Arguments :
 *   -# rd Reader entity
 *   -# buf an array of pointers to samples into which data is read (pointers can be NULL)
 *   -# maxs maximum number of samples to read
 *   -# si pointer to an array of \ref dds_sample_info_t returned for each data value
 *   -# cond read condition to filter the data samples based on the content
 *   -# Returns the number of samples read, 0 indicates no data to read.
 */
DDS_EXPORT int dds_read_cond
(
  dds_entity_t rd,
  void ** buf,
  uint32_t maxs,
  dds_sample_info_t * si,
  dds_condition_t cond
);

/**
 * Description : Access the collection of data values (of same type) and sample info from the data reader
 *               based on the mask set. Data value once read is removed from the Data Reader cannot to
 *               'read' or 'taken' again.
 *               Return value provides information about number of samples read, which will
 *               be <= maxs. Based on the count, the buffer will contain data to be read only
 *               when valid_data bit in sample info structure is set.
 *               The buffer required for data values, could be allocated explicitly or can
 *               use the memory from data reader to prevent copy. In the latter case, buffer and
 *               sample_info should be returned back, once it is no longer using the Data.
 *
 * Arguments :
 *   -# rd Reader entity
 *   -# buf an array of pointers to samples into which data is read (pointers can be NULL)
 *   -# maxs maximum number of samples to read
 *   -# si pointer to an array of \ref dds_sample_info_t returned for each data value
 *   -# mask filter the data value based on the set sample, view and instance state
 *   -# Returns the number of samples read, 0 indicates no data to read.
 */
DDS_EXPORT int dds_take
(
  dds_entity_t rd,
  void ** buf,
  uint32_t maxs,
  dds_sample_info_t * si,
  uint32_t mask
);

/**
 * Description : Implements the same functionality as dds_take, except that only data
 *               scoped to the provided instance handle is taken.
 *
 * Arguments :
 *   -# rd Reader entity
 *   -# buf an array of pointers to samples into which data is read (pointers can be NULL)
 *   -# maxs maximum number of samples to read
 *   -# si pointer to an array of \ref dds_sample_info_t returned for each data value
 *   -# mask filter the data value based on the set sample, view and instance state
 *   -# handle the instance handle identifying the instance from which to take
 *   -# Returns the number of samples read, 0 indicates no data to read.
 */
DDS_EXPORT int dds_take_instance
(
  dds_entity_t rd,
  void ** buf,
  uint32_t maxs,
  dds_sample_info_t * si,
  dds_instance_handle_t handle,
  uint32_t mask
);

/**
 * Description : Access the collection of data values (of same type) and sample info from the data reader
 *               based on the read condition set. Data value once read is removed from the Data Reader
 *               cannot to 'read' or 'taken' again.
 *               Read condition must be attached to the data reader before associating with data_take.
 *               Return value provides information about number of samples read, which will
 *               be <= maxs. Based on the count, the buffer will contain data to be read only
 *               when valid_data bit in sample info structure is set.
 *               The buffer required for data values, could be allocated explicitly or can
 *               use the memory from data reader to prevent copy. In the latter case, buffer and
 *               sample_info should be returned back, once it is no longer using the Data.
 *
 * Arguments :
 *   -# rd Reader entity
 *   -# buf an array of pointers to samples into which data is read (pointers can be NULL)
 *   -# maxs maximum number of samples to read
 *   -# si pointer to an array of \ref dds_sample_info_t returned for each data value
 *   -# cond read condition to filter the data samples based on the content
 *   -# Returns the number of samples read, 0 indicates no data to read.
 */
DDS_EXPORT int dds_take_cond
(
  dds_entity_t rd,
  void ** buf,
  uint32_t maxs,
  dds_sample_info_t * si,
  dds_condition_t cond
);

/* The read/take next functions return a single sample. The returned sample
 * has a sample state of NOT_READ, a view state of ANY_VIEW_STATE and an
 * instance state of ANY_INSTANCE_STATE.
 */

/**
 * Description : This operation copies the next, non-previously accessed data value and corresponding
 *               sample info and removes from the data reader.
 *
 * Arguments :
 * -# rd Reader entity
 * -# buf an array of pointers to samples into which data is read (pointers can be NULL)
 * -# si pointer to \ref dds_sample_info_t returned for a data value
 * -# Returns 1 on successful operation, else 0 if there is no data to be read.
 */
DDS_EXPORT int dds_take_next (dds_entity_t rd, void ** buf, dds_sample_info_t * si);

/**
 * Description : This operation copies the next, non-previously accessed data value and corresponding
 *               sample info.
 *
 * Arguments :
 * -# rd Reader entity
 * -# buf an array of pointers to samples into which data is read (pointers can be NULL)
 * -# si pointer to \ref dds_sample_info_t returned for a data value
 * -# Returns 1 on successful operation, else 0 if there is no data to be read.
 */
DDS_EXPORT int dds_read_next (dds_entity_t rd, void ** buf, dds_sample_info_t * si);

/**
 * Description : This operation is used to return loaned samples from a data reader
 *               returned from a read/take operation. This function is used where the samples
 *               returned by a read/take operation have been allocated by DDS (an array
 *               of NULL pointers was provided as the buffer for the read/take operation
 *               of size maxs).
 *
 * Arguments :
 * -# rd Reader entity
 * -# buf An array of pointers used by read/take operation
 * -# maxs The maximum number of samples provided to the read/take operation
 */
DDS_EXPORT void dds_return_loan (dds_entity_t rd, void ** buf, uint32_t maxs);

/*
 * Instance handle <=> key value mapping.
 * Functions exactly as read w.r.t. treatment of data
 * parameter. On output, only key values set.
 *
 *   T x = { ... };
 *   T y;
 *   dds_instance_handle_t ih;
 *   ih = dds_instance_lookup (e, &x);
 *   dds_instance_get_key (e, ih, &y);
 */

/**
 * Description : This operation takes a sample and returns an instance handle to be used for
 * subsequent operations.
 *
 * Arguments :
 * -# e Reader or Writer entity
 * -# data sample with a key fields set
 * -# Returns instance handle or DDS_HANDLE_NIL if instance could not be found from key
 */
DDS_EXPORT dds_instance_handle_t dds_instance_lookup (dds_entity_t e, const void * data);

/**
 * Description : This operation takes an instance handle and return a key-value corresponding to it.
 *
 * Arguments :
 * -# e Reader or Writer entity
 * -# inst Instance handle
 * -# data pointer to an instance, to which the key ID corresponding to the instance handle will be
 *    returned, the sample in the instance should be ignored.
 * -# Returns 0 on successful operation, or a non-zero value to indicate an error if the instance
 *    passed doesn't have a key-value
 */
DDS_EXPORT int dds_instance_get_key (dds_entity_t e, dds_instance_handle_t inst, void * data);

/**
 * Description : This operation sets a property on the entity.
 *
 * Arguments :
 * -# e The entity on which the property should be available. Only entities of kind DDS_ENTITY_KIND_DOMAINPARTICIPANT are checked.
 * -# property The name (case insensitive) of the property to set. If found, the value of the property will be 
 *             set to the string in the value parameter
 * -# value The value to set the property to (if found).
 * -# Returns 0 on successful operation, or a negative value indicates an error. Use  dds_err_no to retrieve the error code.
 * 
 * Detailed Description
 * This operation sets the property specified by name to the value given by value on the given entity specified by e.
 * 
 * Currently, the following properties are defined: 
 * isolateNode 
 * The isolateNode property allows applications to isolate the federation from the rest of the Domain, i.e. 
 * at network level disconnect the node from the rest of the system. Additionally, they also need to be able 
 * to issue a request to reconnect their federation to the domain again after which the durability merge-policy 
 * that is configured needs to be applied. 
 * To isolate a federation, the application needs to set the isolateNode property value to ‘true’ and to 
 * (de)isolate the federation the same property needs to set to ‘false’. The default value of the isolateNode property is ‘false’.
 * All data that is published after isolateNode is set to true will not be sent to the network and any data 
 * received from the network will be ignored.  
 * Be aware that data being processed by the network service at time of isolating a node may still be sent to the 
 * network due to asynchronous nature of network service internals.
 * The value is interpreted as a boolean (i.e., it must be either ‘true’ or ‘false’ (case insensitive)).
 * false (default): The federation is connected to the domain.
 * true: The federation is disconnected from the domain meaning that data is not published on the network and data from the network is ignored.
 * 
 * Error codes
 * The following error code can be returned (retrieved by using dds_err_no);
 * DDS_RETCODE_BAD_PARAMETER  -  the given dds_entity_t is NULL
 * DDS_RETCODE_BAD_PARAMETER  -  the given name is NULL
 * DDS_RETCODE_BAD_PARAMETER  -  the given value is either NULL or not  ‘true’   or  ‘false’  (case insensitive)
 * DDS_RETCODE_UNSUPPORTED    -  the given dds_entity_t is not a DDS_ENTITY_KIND_DOMAINPARTICIPANT  kind
 * DDS_RETCODE_UNSUPPORTED    -  the given name contains an unsupported property
 */
DDS_EXPORT int dds_set_property(dds_entity_t e, const char *property, const char *value);

/**
 * Description : This operation gets a property from the entity.
 *
 * Arguments :
 * -# e entity The entity on which the property should be available. Only entities of kind DDS_ENTITY_KIND_DOMAINPARTICIPANT are checked.
 * -# property The name (case insensitive) of the property to fetch. If found, the value of the property will be 
 *             represented as string in the value parameter
 * -# value The current value of the property (if found).
 * -# size Number of reserved characters for value. If size is less than the number of characters of the stored 
 *         value, value will be truncated.
 * -# Returns on success the number of characters in the output (excluding the null terminator).
 *    If the output was truncated due to size limits, then the returned value is the number of
 *    characters (excluding the null terminator) which would have been in the output had there been
 *    enough space available. Thus, a returns value of size or more means that the output was
 *    truncated.
 *    A negative value indicates an error. Use  dds_err_no to retrieve the error code.
 * 
 * Detailed Description
 * "This operation looks up the property specified by name in the DomainParticipant, copying 
 * the corresponding value to the parameter 'value' and returning the number of characters copied to parameter 'value'"
 * 
 * Supported properties:
 * See dds_set_property for supported properties
 * 
 * Error codes
 * The following error code can be returned (retrieved by using dds_err_no);
 * DDS_RETCODE_BAD_PARAMETER  -  the given dds_entity_t is NULL
 * DDS_RETCODE_BAD_PARAMETER  -  the given name is NULL
 * DDS_RETCODE_BAD_PARAMETER  -  the given value is NULL
 * DDS_RETCODE_BAD_PARAMETER  -  the given size is larger than MAX_INT
 * DDS_RETCODE_UNSUPPORTED    -  the given dds_entity_t is not a DDS_ENTITY_KIND_DOMAINPARTICIPANT kind, or 
 * DDS_RETCODE_UNSUPPORTED    -  the given name contains an invalid property

  */
DDS_EXPORT int dds_get_property(dds_entity_t e, const char *property, char *value, size_t size);

/**
 * Description : This operation stores the thread state for the thread created.
 *
 * Arguments :
 * -# name Thread name
 * -# Returns 0 on successful thread creation, else a non-zero value to indicate an error,
 *    which could be a lack of resources or a thread with the same name already exists.
 */
DDS_EXPORT int dds_thread_init (const char * name);

/**
 * Description : This operation frees the thread state stored
 *
 * Note: This function should be called from the same thread context before exiting
 */
DDS_EXPORT void dds_thread_fini (void);

/**
 * Description : The QoSProvider is created given the file uri and the profile within the file
 *
 *  Arguments :
 *   -# qp  The QosProvider
 *   -# uri _the file uri (absolute path) to th QoS XML file
 *   -# profile the profile within the file
 *   -# Returns _Success_(return == 0)
 */
DDS_EXPORT int dds_qosprovider_create (dds_entity_t * qp, const char *uri,const char *profile);

/**
 * Description : Given the QoSProvider, populate the created QoS
 *
 *  Arguments :
 *   -# qp  the QoSProvider
 *   -# qos the QoS to be populated. Qos must be created at this point.
 *   -# id
 *   -# Returns _Success_(return == 0)
 */
DDS_EXPORT int dds_qosprovider_get_participant_qos (dds_entity_t qp, dds_qos_t *qos, const char *id);

/**
 * Description :  Given the QoSProvider, populate the created QoS
 *
 *  Arguments :
 *   -# qp  the QoSProvider
 *   -# qos the QoS to be populated. Qos must be created at this point.
 *   -# id
 *   -# Returns _Success_(return == 0)
 */
DDS_EXPORT int dds_qosprovider_get_topic_qos
(
  dds_entity_t qp,
  dds_qos_t *qos,
  const char *id
);

/**
 * Description : Given the QoSProvider, populate the created QoS
 *
 *  Arguments :
 *   -# qp  the QoSProvider
 *   -# qos the QoS to be populated. Qos must be created at this point.
 *   -# id
 *   -# Returns _Success_(return == 0)
 */
DDS_EXPORT int dds_qosprovider_get_subscriber_qos (dds_entity_t qp, dds_qos_t *qos, const char *id);

/**
 * Description : Given the QoSProvider, populate the created QoS
 *
 *  Arguments :
 *   -# qp  the QoSProvider
 *   -# qos the QoS to be populated. Qos must be created at this point.
 *   -# id
 *   -# Returns _Success_(return == 0)
 */
DDS_EXPORT int dds_qosprovider_get_reader_qos(dds_entity_t qp, dds_qos_t *qos, const char *id);

/**
 * Description : Given the QoSProvider, populate the created QoS
 *
 *  Arguments :
 *   -# qp  the QoSProvider
 *   -# qos the QoS to be populated. Qos must be created at this point.
 *   -# id
 *   -# Returns _Success_(return == 0)
 */
DDS_EXPORT int dds_qosprovider_get_writer_qos (dds_entity_t qp, dds_qos_t *qos, const char *id);

/**
 * Description : Given the QoSProvider, populate the created QoS
 *
 *  Arguments :
 *   -# qp  the QoSProvider
 *   -# qos the QoS to be populated. Qos must be created at this point.
 *   -# id
 *   -# Returns _Success_(return == 0)
 */
DDS_EXPORT int dds_qosprovider_get_publisher_qos(dds_entity_t qp, dds_qos_t *qos, const char *id);

#undef DDS_EXPORT
#if defined (__cplusplus)
}
#endif
#endif /* DDS_H */
