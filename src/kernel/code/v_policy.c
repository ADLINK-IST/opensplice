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

#include "v__policy.h"
#include "v_kernel.h"
#include "v_messageQos.h"
#include "v__dataReaderInstance.h"
#include "v__groupInstance.h"
#include "v_public.h"
#include "v_topic.h"
#include "c_base.h"
#include "sd_serializer.h"
#include "sd_serializerXMLTypeinfo.h"

#include "c_stringSupport.h"

#include "vortex_os.h"
#include "os_report.h"

static const struct { int shift; const char *name; } vpolicymapping[] = {
    { V_USERDATAPOLICY_ID,            V_USERDATAPOLICY_NAME },
    { V_DURABILITYPOLICY_ID,          V_DURABILITYPOLICY_NAME },
    { V_DURABILITYSERVICEPOLICY_ID,   V_DURABILITYSERVICEPOLICY_NAME },
    { V_PRESENTATIONPOLICY_ID,        V_PRESENTATIONPOLICY_NAME },
    { V_DEADLINEPOLICY_ID,            V_DEADLINEPOLICY_NAME },
    { V_LATENCYPOLICY_ID,             V_LATENCYPOLICY_NAME },
    { V_OWNERSHIPPOLICY_ID,           V_OWNERSHIPPOLICY_NAME },
    { V_STRENGTHPOLICY_ID,            V_STRENGTHPOLICY_NAME },
    { V_LIVELINESSPOLICY_ID,          V_LIVELINESSPOLICY_NAME },
    { V_PACINGPOLICY_ID,              V_PACINGPOLICY_NAME },
    { V_PARTITIONPOLICY_ID,           V_PARTITIONPOLICY_NAME },
    { V_RELIABILITYPOLICY_ID,         V_RELIABILITYPOLICY_NAME },
    { V_ORDERBYPOLICY_ID,             V_ORDERBYPOLICY_NAME },
    { V_HISTORYPOLICY_ID,             V_HISTORYPOLICY_NAME },
    { V_RESOURCEPOLICY_ID,            V_RESOURCEPOLICY_NAME },
    { V_ENTITYFACTORYPOLICY_ID,       V_ENTITYFACTORYPOLICY_NAME },
    { V_WRITERLIFECYCLEPOLICY_ID,     V_WRITERLIFECYCLEPOLICY_NAME },
    { V_READERLIFECYCLEPOLICY_ID,     V_READERLIFECYCLEPOLICY_NAME },
    { V_TOPICDATAPOLICY_ID,           V_TOPICDATAPOLICY_NAME },
    { V_GROUPDATAPOLICY_ID,           V_GROUPDATAPOLICY_NAME },
    { V_TRANSPORTPOLICY_ID,           V_TRANSPORTPOLICY_NAME },
    { V_LIFESPANPOLICY_ID,            V_LIFESPANPOLICY_NAME },
    { V_USERKEYPOLICY_ID,             V_USERKEYPOLICY_NAME },
    { V_SHAREPOLICY_ID,               V_SHAREPOLICY_NAME },
    { V_READERLIFESPANPOLICY_ID,      V_READERLIFESPANPOLICY_NAME },
    { V_SCHEDULINGPOLICY_ID,          V_SCHEDULINGPOLICY_NAME }
};

/**************************************************************
 * Private functions
 **************************************************************/

/**************************************************************
 * constructor/destructor
 **************************************************************/

/**************************************************************
 * Protected functions
 **************************************************************/
v_partitionPolicyI
v_partitionPolicyAdd(
    v_partitionPolicyI p,
    const c_char* expr,
    c_base base)
{
    c_iter list;
    c_char *str, *partition;
    os_size_t size;
    c_bool isIn;
    v_partitionPolicyI newPolicy;

    newPolicy.v = NULL;

    assert(expr);

    if(p.v){
        isIn = FALSE;

        list = v_partitionPolicySplit(p);
        partition = c_iterTakeFirst(list);

        while(partition){
            if(strcmp(partition, expr) == 0){
               isIn = TRUE;
           }
           os_free(partition);
           partition = c_iterTakeFirst(list);
        }
        c_iterFree(list);

        if(isIn){
            /* It's already in there, so return the current value */
            newPolicy.v = c_stringNew(base, p.v);
        } else {
            /* It's not in there, so add it to the existing one */
            size = strlen(p.v) + 1 + strlen(expr) + 1;
            str = os_malloc(size);

            if (str) {
                os_strncpy(str, p.v, size);
                str = os_strcat(str, ",");
                str = os_strcat(str, expr);
                newPolicy.v = c_stringNew(base, str);
                os_free(str);
            } else {
                /* failed to allocate, so report error and return NULL */
                OS_REPORT(OS_FATAL, "v_partitionPolicyAdd", V_RESULT_OUT_OF_MEMORY, "Failed to allocate partitionPolicy");
            }
        }
    } else {
        /* No policy exists yet, so make the expr the new policy */
        newPolicy.v = c_stringNew(base, expr);
    }
    return newPolicy;
}

v_partitionPolicyI
v_partitionPolicyRemove(
    v_partitionPolicyI p,
    const c_char *expr,
    c_base base)
{
    v_partitionPolicyI newPolicy;
    c_char *str;
    c_char *start; /* start of expr in p */
    os_size_t len;

    newPolicy.v = NULL;
    if (p.v != NULL) {
        if (strcmp(p.v, expr) != 0) {
            len = strlen(p.v);
            str = os_malloc(len + 1);
            start = strstr(p.v, expr);
            assert(start != NULL);
            assert((c_address)start >= (c_address)p.v);
            os_strncpy(str, p.v, (c_address)start - (c_address)p.v); /* includes ',' */
            str[(c_address)start - (c_address)p.v] = 0; /* make '\0' terminated */
            if (strcmp(start, expr) != 0) { /* not at the end */
                os_strcat(str, (c_char *)((c_address)start + strlen(expr) + 1 /* , */));
            }
            newPolicy.v = c_stringNew(base, str);
            os_free(str);
        }
    }

    return newPolicy;
}

c_iter
v_partitionPolicySplit(
    v_partitionPolicyI p)
{
    const c_char *head, *tail;
    c_char *nibble;
    c_iter iter = NULL;
    os_size_t length;
    const c_char *delimiters = ",";

    if (p.v == NULL) return NULL;

    head = p.v;
    do {
        tail = c_skipUntil(head,delimiters);
        length = (os_size_t) (tail - head);
        if (length != 0) {
            length++;
            nibble = (c_string)os_malloc(length);
            os_strncpy(nibble, head, length);
            nibble[length-1]=0;
            iter = c_iterAppend(iter, nibble);
        } else {
            /* head points to one of the delimiters, so we
               add an empty string */
            length = 1;
            nibble = (c_string)os_malloc(length);
            nibble[length - 1 ] = 0;
            iter = c_iterAppend(iter, nibble);
        }
        head = tail;
        if (c_isOneOf(*head, delimiters)) {
            /* if the string ends with a delimiter, we also add an empty string */
            head++;
            if (*head == '\0') {
                length = 1;
                nibble = (c_string)os_malloc(length);
                nibble[length - 1 ] = 0;
                iter = c_iterAppend(iter, nibble);
            }
        }
    } while (*head != '\0');

    return iter;
}

/* FIXME:
 * "claim" parameter serves as a workaround for dds1784. We need to make sure
 * that the owner is not set for DataReader instances when there are no more
 * live DataWriters. Otherwise a new DataWriter with a lower strength would
 * never be able to take over again!
 *
 * THIS IS A WORKAROUND AND NEEDS A REAL FIX.
 */
v_ownershipResult
v_determineOwnershipByStrength (
    struct v_owner *owner,
    const struct v_owner *candidate,
    c_bool claim)
{
    c_equality equality;
    v_ownershipResult result;

    assert (owner != NULL);
    assert (candidate != NULL);

    if (owner->exclusive == TRUE) {
        if (v_gidIsValid (candidate->gid)) {
            if (owner->exclusive == candidate->exclusive) {
                if (v_gidIsValid (owner->gid)) {
                    equality = v_gidCompare (owner->gid, candidate->gid);

                    if (candidate->strength > owner->strength) {
                        if (equality == C_EQ) {
                            result = V_OWNERSHIP_ALREADY_OWNER;
                            owner->strength = candidate->strength;
                        } else {
                            if (claim == TRUE) {
                                owner->gid = candidate->gid;
                                owner->strength = candidate->strength;
                            }
                            result = V_OWNERSHIP_OWNER;
                        }
                    } else if (candidate->strength < owner->strength) {
                        if (equality == C_EQ) {
                            /* The current message comes from the a writer,
                             * which is the owner AND which lowered it's
                             * strength. The strength associated with the
                             * ownership must be updated.
                             */
                            owner->strength = candidate->strength;
                            result = V_OWNERSHIP_ALREADY_OWNER;
                        } else {
                            result = V_OWNERSHIP_NOT_OWNER;
                        }
                    } else {
                        if (equality == C_EQ) {
                            result = V_OWNERSHIP_ALREADY_OWNER;
                        } else if (equality == C_GT) {
                            if (claim == TRUE) {
                                /* The current message comes from a writer, which
                                 * is not owner AND has a strength that is equal to
                                 * the strength of the current owner. So we must
                                 * determine which writer should be the owner.
                                 * Every reader must determine the ownership
                                 * identically, so we determine it by comparing the
                                 * identification of the writer. The writer with
                                 * the highest gid will be the owner.
                                 */
                                owner->gid = candidate->gid;
                            }
                            result = V_OWNERSHIP_OWNER;
                        } else {
                            result = V_OWNERSHIP_NOT_OWNER;
                        }
                    }
                } else if (claim == TRUE) {
                    owner->gid = candidate->gid;
                    owner->strength = candidate->strength;
                    result = V_OWNERSHIP_OWNER;
                } else {
                    /* Instance has no owner and no registrations either.
                     * This may happen during deletion of a DataReader or
                     * when inserting historical data
                     */
                    result = V_OWNERSHIP_OWNER;
                }
            } else {
                result = V_OWNERSHIP_INCOMPATIBLE_QOS;
            }
        } else {
            v_gidSetNil (owner->gid);
            result = V_OWNERSHIP_OWNER_RESET;
        }
    } else {
        result = V_OWNERSHIP_SHARED_QOS;
    }

    return result;
}

/**************************************************************
 * Public functions
 **************************************************************/

void
v_policyReportInvalid(
    c_ulong mask)
{
    int i;
    for (i = 0; i < (int)(sizeof(vpolicymapping) / sizeof(*vpolicymapping)); i++) {
        if (mask & (1u << vpolicymapping[i].shift)) {
            OS_REPORT(OS_ERROR, "v_policyReportInvalid", V_RESULT_ILL_PARAM,
                "Invalid '%s' QoS policy", vpolicymapping[i].name);
        }
    }
}

void
v_policyReportImmutable(
    c_ulong mask,
    c_ulong immutable)
{
    int i;
    for (i = 0; i < (int)(sizeof(vpolicymapping) / sizeof(*vpolicymapping)); i++) {
        if ((mask & (1u << vpolicymapping[i].shift)) &&
            (immutable & (1u << vpolicymapping[i].shift))) {
            OS_REPORT(OS_ERROR, "v_policyReportImmutable", V_RESULT_ILL_PARAM,
                "QoS policy '%s' is immutable", vpolicymapping[i].name);
        }
    }
}


v_result
v_policyConvToExt_type_name (
    c_base base,
    c_string *dst,
    const C_STRUCT(c_type) *topic_type)
{
    char *str;
    v_result result = V_RESULT_OK;
    if ((str = c_metaScopedName (c_metaObject (topic_type))) == NULL) {
        result = V_RESULT_OUT_OF_MEMORY;
        OS_REPORT(OS_CRITICAL,
                  "kernel::v_builtin::v_policyConvToExt_type_name", result,
                  "Operation c_metaScopedName(topic_type) failed."
                  OS_REPORT_NL "topic_type = %p",
                  (void *) topic_type);
    } else {
        if ((*dst = c_stringNew_s (base, str)) == NULL) {
            result = V_RESULT_OUT_OF_MEMORY;
            OS_REPORT(OS_CRITICAL,
                      "kernel::v_builtin::v_policyConvToExt_type_name", result,
                      "Operation c_stringNew(base=%p, str=\"%s\") failed."
                      OS_REPORT_NL "topic_type = %p",
                      (void *) base, str, (void *) topic_type);
        }
        os_free (str);
    }
    return result;
}

void v_policyConvToExt_topic_name (c_string *dst, const char *topic)
{
    *dst = c_keep ((c_object) topic);
}

void v_policyConvToExt_durability (struct v_durabilityPolicy *dst, const v_durabilityPolicyI *src)
{
    *dst = src->v;
}

void v_policyConvToExt_durability_service (struct v_durabilityServicePolicy *dst, const v_durabilityServicePolicyI *src)
{
    dst->service_cleanup_delay = v_durationFromOsDuration(src->v.service_cleanup_delay);
    dst->history_kind = src->v.history_kind;
    dst->history_depth = src->v.history_depth;
    dst->max_samples = src->v.max_samples;
    dst->max_instances = src->v.max_instances;
    dst->max_samples_per_instance = src->v.max_samples_per_instance;
}

void v_policyConvToExt_deadline (struct v_deadlinePolicy *dst, const v_deadlinePolicyI *src)
{
    dst->period = v_durationFromOsDuration(src->v.period);
}

void v_policyConvToExt_latency_budget (struct v_latencyPolicy *dst, const v_latencyPolicyI *src)
{
    dst->duration = v_durationFromOsDuration(src->v.duration);
}

void v_policyConvToExt_liveliness (struct v_livelinessPolicy *dst, const v_livelinessPolicyI *src)
{
    dst->kind = src->v.kind;
    dst->lease_duration = v_durationFromOsDuration(src->v.lease_duration);
}

void v_policyConvToExt_reliability (struct v_reliabilityPolicy *dst, const v_reliabilityPolicyI *src)
{
    dst->kind = src->v.kind;
    dst->max_blocking_time = v_durationFromOsDuration(src->v.max_blocking_time);
    dst->synchronous = src->v.synchronous;
}

void v_policyConvToExt_transport_priority (struct v_transportPolicy *dst, const v_transportPolicyI *src)
{
    *dst = src->v;
}

void v_policyConvToExt_lifespan (struct v_lifespanPolicy *dst, const v_lifespanPolicyI *src)
{
    dst->duration = v_durationFromOsDuration(src->v.duration);
}

void v_policyConvToExt_reader_lifespan (struct v_readerLifespanPolicy *dst, const v_readerLifespanPolicyI *src)
{
    dst->duration = v_durationFromOsDuration(src->v.duration);
}

void v_policyConvToExt_time_based_filter (struct v_pacingPolicy *dst, const v_pacingPolicyI *src)
{
    dst->minSeperation = v_durationFromOsDuration(src->v.minSeperation);
}

void v_policyConvToExt_destination_order (struct v_orderbyPolicy *dst, const v_orderbyPolicyI *src)
{
    *dst = src->v;
}

void v_policyConvToExt_history (struct v_historyPolicy *dst, const v_historyPolicyI *src)
{
    *dst = src->v;
}

void v_policyConvToExt_resource_limits (struct v_resourcePolicy *dst, const v_resourcePolicyI *src)
{
    *dst = src->v;
}

void v_policyConvToExt_ownership (struct v_ownershipPolicy *dst, const v_ownershipPolicyI *src)
{
    *dst = src->v;
}

void v_policyConvToExt_ownership_strength (struct v_strengthPolicy *dst, const v_strengthPolicyI *src)
{
    *dst = src->v;
}

void v_policyConvToExt_presentation (struct v_presentationPolicy *dst, const v_presentationPolicyI *src)
{
    *dst = src->v;
}

void v_policyConvToExt_writer_data_lifecycle (struct v_writerLifecyclePolicy *dst, const v_writerLifecyclePolicyI *src)
{
    dst->autodispose_unregistered_instances = src->v.autodispose_unregistered_instances;
    dst->autopurge_suspended_samples_delay = v_durationFromOsDuration(src->v.autopurge_suspended_samples_delay);
    dst->autounregister_instance_delay = v_durationFromOsDuration(src->v.autounregister_instance_delay);
}

void v_policyConvToExt_reader_data_lifecycle (struct v_readerLifecyclePolicy *dst, const v_readerLifecyclePolicyI *src)
{
    dst->autopurge_nowriter_samples_delay = v_durationFromOsDuration(src->v.autopurge_nowriter_samples_delay);
    dst->autopurge_disposed_samples_delay = v_durationFromOsDuration(src->v.autopurge_disposed_samples_delay);
    dst->autopurge_dispose_all = src->v.autopurge_dispose_all;
    dst->enable_invalid_samples = src->v.enable_invalid_samples;
    dst->invalid_sample_visibility = src->v.invalid_sample_visibility;
}

void v_policyConvToExt_subscription_keys (struct v_userKeyPolicy *dst, const v_userKeyPolicyI *src)
{
    dst->enable = src->v.enable;
    dst->expression = c_keep (src->v.expression);
}

void v_policyConvToExt_share (c_base base, struct v_sharePolicy *dst, const v_sharePolicyI *src)
{
    dst->enable = src->v.enable;
    dst->name = src->v.name ? c_keep (src->v.name) : c_stringNew (base, "");
}

static v_result v_policyConvToExt_blobdata (c_base base, c_array *dst, const c_octet *src, c_ulong srcsize)
{
    if (srcsize == 0) {
        *dst = NULL;
        return V_RESULT_OK;
    } else {
        c_type type = c_octet_t (base);
        *dst = c_arrayNew_s (type, srcsize);
        if (*dst) {
            memcpy (*dst, src, srcsize);
            return V_RESULT_OK;
        } else {
            c_free (*dst);
            *dst = NULL;
            OS_REPORT(OS_CRITICAL,
                      "kernel::v_builtin::v_policyConvToExt_blobdata", V_RESULT_OUT_OF_MEMORY,
                      "Operation c_arrayNew(type=%p, size=%d)"
                      OS_REPORT_NL "failed for built-in topic message",
                      (void *) type, srcsize);
            return V_RESULT_OUT_OF_MEMORY;
        }
    }
}

v_result v_policyConvToExt_topic_data (c_base base, struct v_builtinTopicDataPolicy *dst, const v_topicDataPolicyI *src)
{
    return v_policyConvToExt_blobdata (base, &dst->value, (const c_octet *) src->v.value, (c_ulong) src->v.size);
}

v_result v_policyConvToExt_group_data (c_base base, struct v_builtinGroupDataPolicy *dst, const v_groupDataPolicyI *src)
{
    return v_policyConvToExt_blobdata (base, &dst->value, (const c_octet *) src->v.value, (c_ulong) src->v.size);
}

v_result v_policyConvToExt_user_data (c_base base, struct v_builtinUserDataPolicy *dst, const v_userDataPolicyI *src)
{
    return v_policyConvToExt_blobdata (base, &dst->value, (const c_octet *) src->v.value, (c_ulong) src->v.size);
}

void v_policyConvToExt_entity_factory (struct v_entityFactoryPolicy *dst, const v_entityFactoryPolicyI *src)
{
    *dst = src->v;
}

v_result v_policyConvToExt_partition (c_base base, struct v_builtinPartitionPolicy *dst, const v_partitionPolicyI *src)
{
    c_iter partitions;
    c_ulong i, length;
    c_type type;
    char *str;

    partitions = v_partitionPolicySplit (*src);
    length = c_iterLength (partitions);
    if (length == 0) {
        str = os_strdup ("");
        partitions = c_iterAppend (partitions, str);
        length = 1;
    }

    type = c_string_t (base);
    if ((dst->name = c_arrayNew_s (type, length)) == NULL) {
        OS_REPORT(OS_ERROR,
                  "kernel::v_builtin::v_policyConvToExt_partition", V_RESULT_OUT_OF_MEMORY,
                  "c_arrayNew(type=%p, length=%d) failed",
                  (void *) type, length);
        goto err_out_of_mem;
    }

    for (i = 0; i < length; i++)
    {
        str = c_iterTakeFirst (partitions);
        assert (str != NULL);
        dst->name[i] = c_stringNew_s (base, str);
        os_free(str);
        if (dst->name[i] == NULL) {
            goto err_out_of_mem_pname;
        }
    }
    c_iterFree (partitions);
    return V_RESULT_OK;

 err_out_of_mem_pname:
    c_free (dst->name);
    dst->name = NULL;
 err_out_of_mem:
    while ((str = c_iterTakeFirst (partitions)) != NULL) {
        os_free (str);
    }
    c_iterFree (partitions);
    return V_RESULT_OUT_OF_MEMORY;
}

v_result v_policyConvToExt_topic_meta_data (c_base base, c_string *meta_data, c_string *key_list, const C_STRUCT(c_type) *topic_type, const char *topic_key_expr)
{
    v_result result = V_RESULT_OUT_OF_MEMORY;
    sd_serializer serializer;
    sd_serializedData md;
    char *str;
    if ((serializer = sd_serializerXMLTypeinfoNew (base, FALSE)) == NULL) {
        OS_REPORT(OS_ERROR,
                  "kernel::v_builtin::v_builtinCreateTopicInfo", 0,
                  "Operation sd_serializerXMLTypeinfoNew(base=%p, FALSE)"
                  OS_REPORT_NL "failed for built-in topic message.",
                  (void *) base);
        goto err_serializer_new;
    }
    if ((md = sd_serializerSerialize (serializer, c_object (topic_type))) == NULL) {
        goto err_serialize;
    }
    if ((str = sd_serializerToString (serializer, md)) == NULL) {
        OS_REPORT(OS_ERROR,
                  "kernel::v_builtin::v_builtinCreateTopicInfo", 0,
                  "Operation sd_serializerToString(serializer=%p, md=%p)"
                  OS_REPORT_NL "failed for built-in topic message.",
                  (void *) serializer, (void *) md);
        goto err_serialize_to_str;
    }
    if ((*meta_data = c_stringNew_s (base, str)) == NULL) {
        OS_REPORT(OS_CRITICAL,
                  "kernel::v_builtin::v_builtinCreateTopicInfo", V_RESULT_OUT_OF_MEMORY,
                  "Failed to allocate metadata: c_stringNew(size=%" PA_PRIuSIZE ") failed",
                  strlen(str));
        goto err_meta_data;
    }
    *key_list = c_keep ((c_object) topic_key_expr);
    result = V_RESULT_OK;
 err_meta_data:
    os_free (str);
 err_serialize_to_str:
    sd_serializedDataFree (md);
 err_serialize:
    sd_serializerFree (serializer);
 err_serializer_new:
    return result;
}

void v_policyConvToInt_durability (v_durabilityPolicyI *dst, const struct v_durabilityPolicy *src)
{
    dst->v = *src;
}

void v_policyConvToInt_durability_service (v_durabilityServicePolicyI *dst, const struct v_durabilityServicePolicy *src)
{
    dst->v.service_cleanup_delay = v_durationToOsDuration(src->service_cleanup_delay);
    dst->v.history_kind = src->history_kind;
    dst->v.history_depth = src->history_depth;
    dst->v.max_samples = src->max_samples;
    dst->v.max_instances = src->max_instances;
    dst->v.max_samples_per_instance = src->max_samples_per_instance;
}

void v_policyConvToInt_deadline (v_deadlinePolicyI *dst, const struct v_deadlinePolicy *src)
{
    dst->v.period = v_durationToOsDuration(src->period);
}

void v_policyConvToInt_latency_budget (v_latencyPolicyI *dst, const struct v_latencyPolicy *src)
{
    dst->v.duration = v_durationToOsDuration(src->duration);
}

void v_policyConvToInt_liveliness (v_livelinessPolicyI *dst, const struct v_livelinessPolicy *src)
{
    dst->v.kind = src->kind;
    dst->v.lease_duration = v_durationToOsDuration(src->lease_duration);
}

void v_policyConvToInt_reliability (v_reliabilityPolicyI *dst, const struct v_reliabilityPolicy *src)
{
    dst->v.kind = src->kind;
    dst->v.max_blocking_time = v_durationToOsDuration(src->max_blocking_time);
    dst->v.synchronous = src->synchronous;
}

void v_policyConvToInt_transport_priority (v_transportPolicyI *dst, const struct v_transportPolicy *src)
{
    dst->v = *src;
}

void v_policyConvToInt_lifespan (v_lifespanPolicyI *dst, const struct v_lifespanPolicy *src)
{
    dst->v.duration = v_durationToOsDuration(src->duration);
}

void v_policyConvToInt_readerLifespan (v_readerLifespanPolicyI *dst, const struct v_readerLifespanPolicy *src)
{
    dst->v.duration = v_durationToOsDuration(src->duration);
}

void v_policyConvToInt_time_based_filter (v_pacingPolicyI *dst, const struct v_pacingPolicy *src)
{
    dst->v.minSeperation = v_durationToOsDuration(src->minSeperation);
}

void v_policyConvToInt_destination_order (v_orderbyPolicyI *dst, const struct v_orderbyPolicy *src)
{
    dst->v = *src;
}

void v_policyConvToInt_history (v_historyPolicyI *dst, const struct v_historyPolicy *src)
{
    dst->v = *src;
}

void v_policyConvToInt_resource_limits (v_resourcePolicyI *dst, const struct v_resourcePolicy *src)
{
    dst->v = *src;
}

void v_policyConvToInt_ownership (v_ownershipPolicyI *dst, const struct v_ownershipPolicy *src)
{
    dst->v = *src;
}

static v_result v_policyConvToInt_blobdata (c_base base, c_array *dst, c_long *dstsize, const c_array src)
{
    OS_UNUSED_ARG (base);
    *dstsize = (c_long) c_arraySize (src);
    *dst = c_keep (src);
    return V_RESULT_OK;
}

v_result v_policyConvToInt_topic_data (c_base base, v_topicDataPolicyI *dst, const struct v_builtinTopicDataPolicy *src)
{
    return v_policyConvToInt_blobdata (base, &dst->v.value, &dst->v.size, src->value);
}
