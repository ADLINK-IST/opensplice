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
#include "gapi_kernel.h"
#include "gapi_common.h"
#include "gapi_structured.h"

#include "v_entity.h"
#include "v_status.h"
#include "v_topic.h"
#include "v_dataReaderInstance.h"
#include "v_subscriber.h"
#include "v_public.h"
#include "v_event.h"
#include "c_base.h"
#include "c_stringSupport.h"
#include "kernelModule.h"

#include "os_report.h"
#include "os_stdlib.h"

#include "u_user.h"

static void
retrieveBase(v_entity e, c_voidp arg)
{
    c_base *pbase = (c_base *) arg;

    *pbase = c_getBase(e);
}

c_base
kernelGetBase(u_entity e)
{
    c_base base = NULL;

    u_entityAction(u_entity(e), retrieveBase, &base);

    return base;
}


/*
 * This function determines whether a reader has unprocessed data available.
 * Be aware that this is a walk function, that wants to stop the walk when
 * a Reader has been found that has its DATA_AVAILABLE flag set to TRUE.
 * That is why it returns FALSE in case of DATA_AVAILABLE, since that stops
 * the walk function from checking other readers as well.
 */
static c_bool
readerHasDataAvailable (
    v_entity e,
    c_voidp arg
    )
{
    c_bool result = TRUE;

    if ( (v_statusGetMask(e->status) & V_EVENT_DATA_AVAILABLE) != 0 ) {
        result = FALSE;
    }
    return result;
}

static void
getStatusMask(
    v_entity e,
    c_voidp arg)
{
    c_long *mask = (c_long *)arg;

    if ( v_objectKind(e) == K_SUBSCRIBER ) {
        if ( !c_setWalk(v_subscriber(e)->readers, (c_action)readerHasDataAvailable, NULL) ) {
            *mask = V_EVENT_DATA_AVAILABLE;
        } else {
            *mask = 0;
        }
    } else {
        *mask = v_statusGetMask(e->status);
    }
}

c_long
kernelStatusGet (
    u_entity e)
{
    c_long mask;
    u_result result;
    result = u_entityAction(e, getStatusMask, &mask);
    if (result != U_RESULT_OK) {
        mask = 0;
    }
    return mask;
}


static void
getKernelId(
    v_entity e,
    c_voidp arg)
{
    v_kernel *kernel = (v_kernel *)arg;

    *kernel = v_objectKernel(e);
}

v_kernel
kernelGetKernelId (
    u_entity e)
{
    v_kernel kernel = NULL;

    u_entityAction(e, getKernelId, &kernel);

    return kernel;
}

static void
getEntityGid (
    v_entity e,
    c_voidp  arg)
{
    v_gid *gid = (v_gid *) arg;

    *gid = v_publicGid(v_public(e));
}


static v_gid
kernelEntityGid (
    u_entity entity)
{
    v_gid gid;

    u_entityAction(entity, getEntityGid, &gid);

    return gid;
}

gapi_returnCode_t
kernelResultToApiResult(
    u_result r)
{
    gapi_returnCode_t result;

    switch ( r ) {
        case U_RESULT_OK:
            result = GAPI_RETCODE_OK;
            break;
        case U_RESULT_INTERRUPTED:
            result = GAPI_RETCODE_ERROR;
            break;
        case U_RESULT_NOT_INITIALISED:
            result = GAPI_RETCODE_ERROR;
            break;
        case U_RESULT_OUT_OF_MEMORY:
            result = GAPI_RETCODE_OUT_OF_RESOURCES;
            break;
        case U_RESULT_INTERNAL_ERROR:
            result = GAPI_RETCODE_ERROR;
            break;
        case U_RESULT_ILL_PARAM:
            result = GAPI_RETCODE_BAD_PARAMETER;
            break;
        case U_RESULT_CLASS_MISMATCH:
            result = GAPI_RETCODE_ERROR;
            break;
        case U_RESULT_DETACHING:
            result = GAPI_RETCODE_ERROR;
            break;
        case U_RESULT_TIMEOUT:
            result = GAPI_RETCODE_TIMEOUT;
            break;
        case U_RESULT_OUT_OF_RESOURCES:
            result = GAPI_RETCODE_OUT_OF_RESOURCES;
            break;
        case U_RESULT_IMMUTABLE_POLICY:
            result = GAPI_RETCODE_IMMUTABLE_POLICY;
            break;
        case U_RESULT_INCONSISTENT_QOS:
            result = GAPI_RETCODE_INCONSISTENT_POLICY;
            break;
        case U_RESULT_UNDEFINED:
            result = GAPI_RETCODE_ERROR;
            break;
        case U_RESULT_PRECONDITION_NOT_MET:
            result = GAPI_RETCODE_PRECONDITION_NOT_MET;
            break;
        case U_RESULT_ALREADY_DELETED:
            result = GAPI_RETCODE_ALREADY_DELETED;
            break;
        case U_RESULT_UNSUPPORTED:
            result = GAPI_RETCODE_UNSUPPORTED;
            break;
        default:
            result = GAPI_RETCODE_ERROR;
            break;
    }

    return result;
}

void
kernelCopyInDuration (
    const gapi_duration_t *from,
    v_duration            *to)
{
    if ( (from->sec     == GAPI_DURATION_INFINITE_SEC) &&
         (from->nanosec == GAPI_DURATION_INFINITE_NSEC) ) {
        *to = C_TIME_INFINITE;
    } else {
        to->seconds     = from->sec;
        to->nanoseconds = from->nanosec;
    }
}


void
kernelCopyOutDuration (
    const v_duration *from,
    gapi_duration_t  *to)
{
    if ( (from->seconds     == C_TIME_INFINITE.seconds) &&
         (from->nanoseconds == C_TIME_INFINITE.nanoseconds) ) {
        to->sec     = GAPI_DURATION_INFINITE_SEC;
        to->nanosec = GAPI_DURATION_INFINITE_NSEC;
    } else {
        to->sec     = from->seconds;
        to->nanosec = from->nanoseconds;
    }
}

gapi_returnCode_t
kernelCopyInTime (
    const gapi_time_t *from,
    c_time            *to)
{
    gapi_returnCode_t result = GAPI_RETCODE_OK;

    if ( from && to ) {
        if ( (from->sec     ==  GAPI_TIMESTAMP_INVALID_SEC) &&
             (from->nanosec == GAPI_TIMESTAMP_INVALID_NSEC) ) {
            to->seconds     = C_TIME_INVALID.seconds;
            to->nanoseconds = C_TIME_INVALID.nanoseconds;
        } else if ( gapi_validTime(from) ) {
            to->seconds     = from->sec;
            to->nanoseconds = from->nanosec;
        } else {
            result = GAPI_RETCODE_BAD_PARAMETER;
        }
    } else {
        result = GAPI_RETCODE_BAD_PARAMETER;
    }

    return result;
}


gapi_returnCode_t
kernelCopyOutTime (
    const c_time *from,
    gapi_time_t  *to)
{
    gapi_returnCode_t result = GAPI_RETCODE_OK;

    if ( from && to ) {
        to->sec     = from->seconds;
        to->nanosec = from->nanoseconds;
    } else {
        result = GAPI_RETCODE_BAD_PARAMETER;
    }
    return result;
}

static c_ulong
getSumImage (
    c_char *image,
    c_ulong size)
{
    c_ulong sum = 0;
    c_ulong i;

    for ( i = 0; i < size; i++ ) {
        sum += image[i];
    }

    return sum;
}


static c_ulong
getHashKeyFromValue (
    c_value v)
{
    c_ulong result = 0;

    switch ( v.kind ) {
        case V_CHAR:      result = (c_ulong)v.is.Char; break;
        case V_OCTET:     result = (c_ulong)v.is.Octet; break;
        case V_SHORT:     result = (c_ulong)v.is.Short; break;
        case V_LONG:      result = (c_ulong)v.is.Long; break;
        case V_LONGLONG:  result = (c_ulong)v.is.LongLong; break;
        case V_USHORT:    result = (c_ulong)v.is.UShort; break;
        case V_ULONG:     result = (c_ulong)v.is.ULong; break;
        case V_ULONGLONG: result = (c_ulong)v.is.ULongLong; break;
        case V_WCHAR:     result = (c_ulong)v.is.WChar; break;
        case V_BOOLEAN:   result = (c_ulong)v.is.Boolean; break;
        case V_FLOAT:     result = getSumImage((c_char *)&v.is.Float, sizeof(v.is.Float)); break;
        case V_DOUBLE:    result = getSumImage((c_char *)&v.is.Double, sizeof(v.is.Double)); break;
        case V_STRING:    result = getSumImage((c_char *)v.is.String, strlen(v.is.String)); break;
        case V_FIXED:     result = getSumImage((c_char *)v.is.Fixed, strlen(v.is.Fixed)); break;
        case V_OBJECT:    result = getSumImage((c_char *)&v.is.Object, sizeof(v.is.Object)); break;
        default: break;
    }
    return result;
}


c_ulong
kernelGetHashKeyFromKeyValueList (
    kernelKeyValueList list)
{
    c_ulong key = 0;
    c_ulong i;

    for ( i = 0; i < list->size; i++ ) {
        key += getHashKeyFromValue(list->list[i]);
    }

    return key;
}



gapi_boolean
kernelKeyValueListEqual (
    kernelKeyValueList l1,
    kernelKeyValueList l2)
{
    gapi_boolean equal = TRUE;
    c_ulong i;

    if ( l1->size == l2->size ) {
        for ( i = 0; equal && (i < l1->size); i++ ) {
            if ( c_valueCompare(l1->list[i], l2->list[i]) != C_EQ ) {
                equal = FALSE;
            }
        }
    } else {
        equal = FALSE;
    }

    return equal;
}

void
kernelKeyValueListFree (
    kernelKeyValueList list)
{
    c_ulong i;

    if ( list->list ) {
        for ( i = 0; i < list->size; i++ ) {
            if ( list->list[i].kind == V_STRING ) {
                gapi_free(list->list[i].is.String);
            }
        }
    }
    os_free(list);
}


#define FIELDNAME_PREFIX "userData."
static gapi_char *
keyNameFromField (
    c_field field)
{
    gapi_char *name;

    name = c_fieldName(field);
    if ( strstr(name, FIELDNAME_PREFIX) == name ) {
        name = &name[strlen(FIELDNAME_PREFIX)];
    }

    return name;
}
#undef FIELDNAME_PREFIX

typedef struct {
    const gapi_char *keyList;
    gapi_boolean     equal;
} checkTopicKeyListArg;


static c_equality
topicKeyCompare (
    c_voidp o,
    c_iterResolveCompareArg arg)
{
    c_equality result = C_NE;
    gapi_char *n1 = (gapi_char *) o;
    gapi_char *n2 = (gapi_char *) arg;

    if ( strcmp(n1, n2) == 0 ) {
        result = C_EQ;
    }
    return result;
}


static void
checkTopicKeyList (
    v_entity e,
    c_voidp arg)
{
    c_array keyList = v_topicMessageKeyList(e);
    checkTopicKeyListArg *info  = (checkTopicKeyListArg *) arg;
    c_long i;
    gapi_char *name;
    c_long size;
    c_iter iter;
    gapi_boolean equal = TRUE;

    iter = c_splitString(info->keyList, " \t,");

    if ( iter ) {
        size = c_arraySize(keyList);

        for ( i = 0; equal && (i < size); i++ ) {
            name = (gapi_char *)c_iterResolve(iter,
                                              topicKeyCompare,
                                              keyNameFromField(keyList[i]));
            if ( !name ) {
                equal = FALSE;
                OS_REPORT_2(OS_API_INFO,
                            "gapi::kernelCheckTopicKeyList", 0,
                            "incompatible key <%s> found for topic <%s>",
                            keyNameFromField(keyList[i]),
                            v_entityName(e));
            }
        }
        name = c_iterTakeFirst(iter);
        while ( name ) {
            os_free(name);
            name = c_iterTakeFirst(iter);
        }
        c_iterFree(iter);
    }
    info->equal = equal;
}


gapi_boolean
kernelCheckTopicKeyList (
    u_topic topic,
    const gapi_char *keyList)
{
    checkTopicKeyListArg argument;

    argument.keyList = keyList;
    argument.equal   = FALSE;


    u_entityAction(u_entity(topic), checkTopicKeyList, &argument);
    return argument.equal;
}

static void
topicGetKeys(
    v_entity e,
    c_voidp arg)
{
    c_array keyList = v_topicMessageKeyList(e);
    gapi_char     **keys  = (gapi_char **) arg;
    gapi_char      *ptr;
    c_ulong i;
    c_ulong size;
    c_ulong total = 0;
    c_ulong len = 0;

    size = c_arraySize(keyList);

    for ( i = 0; i < size; i++ ) {
        total += strlen(c_fieldName(keyList[i])) + 1;
    }

    if ( total > 0 ) {
        *keys = (gapi_char *)os_malloc(total);
        memset(*keys, 0, total);
    } else {
        *keys = (gapi_char *)os_malloc(1);
        **keys = '\0';
    }

    ptr = *keys;
    for ( i = 0; i < size; i++ ) {
        if ( i == 0 ) {
            len = os_sprintf(ptr, "%s", keyNameFromField(keyList[i]));
        } else {
            len = os_sprintf(ptr, ",%s", keyNameFromField(keyList[i]));
        }
        ptr = &(ptr[len]);
    }
}

gapi_char *
kernelTopicGetKeys (
    u_topic topic)
{
    gapi_char *keys = NULL;

    u_entityAction(u_entity(topic), topicGetKeys, &keys);

    return keys;
}

gapi_boolean
gapi_kernelReaderQosCopyIn (
    const gapi_dataReaderQos *srcQos,
    v_readerQos               dstQos)
{
    gapi_boolean copied = FALSE;

    /* Important note: The sequence related to policies is created on heap and
     * will be copied into the database by the kernel itself, therefore the
     * c_array does not need to be allocated with c_arrayNew, but can be
     * allocated on heap.
     */

    if (dstQos->userData.value) {
        os_free(dstQos->userData.value);
        dstQos->userData.value = NULL;
    }
    dstQos->userData.size = srcQos->user_data.value._length;
    if (dstQos->userData.size) {
        dstQos->userData.value = os_malloc (dstQos->userData.size);
    }
    if ((srcQos->user_data.value._length == 0) || dstQos->userData.value) {
        dstQos->durability.kind = srcQos->durability.kind;
        kernelCopyInDuration(&srcQos->deadline.period, &dstQos->deadline.period);
        kernelCopyInDuration(&srcQos->latency_budget.duration,
                             &dstQos->latency.duration);
        dstQos->liveliness.kind = srcQos->liveliness.kind;
        kernelCopyInDuration(&srcQos->liveliness.lease_duration,
                             &dstQos->liveliness.lease_duration);
        dstQos->reliability.kind = srcQos->reliability.kind;
        kernelCopyInDuration(&srcQos->reliability.max_blocking_time,
                             &dstQos->reliability.max_blocking_time);
        dstQos->reliability.synchronous = srcQos->reliability.synchronous;
        dstQos->orderby.kind = srcQos->destination_order.kind;
        dstQos->history.kind = srcQos->history.kind;
        dstQos->history.depth = srcQos->history.depth;
        dstQos->resource.max_samples = srcQos->resource_limits.max_samples;
        dstQos->resource.max_instances = srcQos->resource_limits.max_instances;
        dstQos->resource.max_samples_per_instance = srcQos->resource_limits.max_samples_per_instance;
        dstQos->ownership.kind = srcQos->ownership.kind;

        if (srcQos->user_data.value._length) {
            memcpy (dstQos->userData.value, srcQos->user_data.value._buffer, srcQos->user_data.value._length);
        }
        kernelCopyInDuration(&srcQos->time_based_filter.minimum_separation,
                             &dstQos->pacing.minSeperation);
        kernelCopyInDuration(&srcQos->reader_data_lifecycle.autopurge_nowriter_samples_delay,
                             &dstQos->lifecycle.autopurge_nowriter_samples_delay);
        kernelCopyInDuration(&srcQos->reader_data_lifecycle.autopurge_disposed_samples_delay,
                             &dstQos->lifecycle.autopurge_disposed_samples_delay);


        /*
         * Deprecated 'enable_invalid_samples' and new 'invalid_sample_visibility'
         * are both mapped to the 'enable_invalid_samples' v_readerQos.
         * (When GAPI_ALL_INVALID_SAMPLES is implemented, v_readerQos should be modified
         * to use invalid_sample_visibility as enum type)
         */
        if (!srcQos->reader_data_lifecycle.enable_invalid_samples) {
            dstQos->lifecycle.enable_invalid_samples = FALSE;
        } else {
            dstQos->lifecycle.enable_invalid_samples = (srcQos->reader_data_lifecycle.invalid_sample_visibility.kind == GAPI_MINIMUM_INVALID_SAMPLES);
        }



        dstQos->lifespan.used = srcQos->reader_lifespan.use_lifespan;
        kernelCopyInDuration(&srcQos->reader_lifespan.duration,
                             &dstQos->lifespan.duration);

        dstQos->share.enable = srcQos->share.enable;
        if ( srcQos->share.enable ) {
            dstQos->share.name = gapi_strdup(srcQos->share.name);
        } else {
            dstQos->share.name = NULL;
        }
        dstQos->userKey.enable = srcQos->subscription_keys.use_key_list;
        if ( srcQos->subscription_keys.use_key_list ) {
            dstQos->userKey.expression = gapi_stringSeq_to_String(&srcQos->subscription_keys.key_list, ",");
        } else {
            dstQos->userKey.expression = NULL;
        }
        copied = TRUE;
    }

    return copied;
}

gapi_boolean
gapi_kernelReaderQosCopyOut (
    const v_readerQos   srcQos,
    gapi_dataReaderQos *dstQos)
{
    gapi_boolean copied = TRUE;

    if ( dstQos->user_data.value._maximum > 0 ) {
        if ( dstQos->user_data.value._release ) {
            gapi_free(dstQos->user_data.value._buffer);
        }
    }

    if ( (srcQos->userData.size > 0) && srcQos->userData.value ) {
        dstQos->user_data.value._buffer = gapi_octetSeq_allocbuf(srcQos->userData.size);
        if ( dstQos->user_data.value._buffer ) {
            dstQos->user_data.value._maximum = srcQos->userData.size;
            dstQos->user_data.value._length  = srcQos->userData.size;
            dstQos->user_data.value._release = TRUE;
            memcpy(dstQos->user_data.value._buffer, srcQos->userData.value, srcQos->userData.size);
        } else {
            copied = FALSE;
        }
    } else {
            dstQos->user_data.value._maximum = 0;
            dstQos->user_data.value._length  = 0;
            dstQos->user_data.value._release = FALSE;
            dstQos->user_data.value._buffer = NULL;
    }

    if ( copied ) {
        dstQos->durability.kind = srcQos->durability.kind;
        kernelCopyOutDuration(&srcQos->deadline.period, &dstQos->deadline.period);
        kernelCopyOutDuration(&srcQos->latency.duration, &dstQos->latency_budget.duration);
        dstQos->liveliness.kind = srcQos->liveliness.kind;
        kernelCopyOutDuration(&srcQos->liveliness.lease_duration, &dstQos->liveliness.lease_duration);
        dstQos->reliability.kind = srcQos->reliability.kind;
        kernelCopyOutDuration(&srcQos->reliability.max_blocking_time, &dstQos->reliability.max_blocking_time);
        dstQos->reliability.synchronous = srcQos->reliability.synchronous;
        dstQos->destination_order.kind = srcQos->orderby.kind;
        dstQos->history.kind  = srcQos->history.kind;
        dstQos->history.depth = srcQos->history.depth;
        dstQos->resource_limits.max_samples              = srcQos->resource.max_samples;
        dstQos->resource_limits.max_instances            = srcQos->resource.max_instances;
        dstQos->resource_limits.max_samples_per_instance = srcQos->resource.max_samples_per_instance;
        dstQos->ownership.kind = srcQos->ownership.kind;
        kernelCopyOutDuration(&srcQos->pacing.minSeperation, &dstQos->time_based_filter.minimum_separation);
        kernelCopyOutDuration(&srcQos->lifecycle.autopurge_nowriter_samples_delay,
                              &dstQos->reader_data_lifecycle.autopurge_nowriter_samples_delay);
        kernelCopyOutDuration(&srcQos->lifecycle.autopurge_disposed_samples_delay,
                              &dstQos->reader_data_lifecycle.autopurge_disposed_samples_delay);

        /*
         * Deprecated 'enable_invalid_samples' and new 'invalid_sample_visibility'
         * are both mapped to the 'enable_invalid_samples' v_readerQos.
         * (When GAPI_ALL_INVALID_SAMPLES is implemented, v_readerQos should be modified
         * to use invalid_sample_visibility as enum type)
         */
        dstQos->reader_data_lifecycle.enable_invalid_samples = srcQos->lifecycle.enable_invalid_samples;
        if (srcQos->lifecycle.enable_invalid_samples == TRUE) {
            dstQos->reader_data_lifecycle.invalid_sample_visibility.kind = GAPI_MINIMUM_INVALID_SAMPLES;
        } else {
            dstQos->reader_data_lifecycle.invalid_sample_visibility.kind = GAPI_NO_INVALID_SAMPLES;
        }

        dstQos->reader_lifespan.use_lifespan = srcQos->lifespan.used;
        kernelCopyOutDuration(&srcQos->lifespan.duration, &dstQos->reader_lifespan.duration);


        dstQos->share.enable = srcQos->share.enable;
        if ( srcQos->share.enable ) {
            assert(srcQos->share.name);
            dstQos->share.name = gapi_string_dup(srcQos->share.name);
        } else {
            dstQos->share.name = NULL;
        }

        dstQos->subscription_keys.use_key_list = srcQos->userKey.enable;
        if ( srcQos->userKey.enable ) {
            if ( !gapi_string_to_StringSeq(srcQos->userKey.expression, ",", &dstQos->subscription_keys.key_list) ) {
                copied = FALSE;
            }
        } else {
            gapi_stringSeq_set_length(&dstQos->subscription_keys.key_list, 0UL);
        }
    }

    return copied;
}

