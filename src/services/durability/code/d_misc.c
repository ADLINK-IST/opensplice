/*
 *                         Vortex OpenSplice
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
 *
 *   Licensed under the ADLINK Software License Agreement Rev 2.7 2nd October
 *   2014 (the "License"); you may not use this file except in compliance with
 *   the License.
 *   You may obtain a copy of the License at:
 *                      $OSPL_HOME/LICENSE
 *
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */
#include "d__misc.h"
#include "d__durability.h"
#include "d__configuration.h"
#include "d__thread.h"
#include "d__admin.h"
#include "d__fellow.h"
#include "d__table.h"
#include "d__nameSpace.h"
#include "d__sampleChainListener.h"
#include "d__mergeAction.h"
#include "u_observable.h"
#include "v_entity.h"
#include "v_group.h"
#include "v_groupInstance.h"
#include "v_topic.h"
#include "v_partition.h"
#include "v_builtin.h"
#include "c_base.h"
#include "c_typebase.h"
#include "os_report.h"
#include "os_stdlib.h"
#include "os_abstract.h"

const char *d_builtinTopics[] = {
    V_TOPICINFO_NAME,
    V_PARTICIPANTINFO_NAME,
    V_PUBLICATIONINFO_NAME,
    V_SUBSCRIPTIONINFO_NAME,
    V_CMPARTICIPANTINFO_NAME,
    V_CMDATAWRITERINFO_NAME,
    V_CMDATAREADERINFO_NAME,
    V_CMPUBLISHERINFO_NAME,
    V_CMSUBSCRIBERINFO_NAME,
    V_HEARTBEATINFO_NAME,
    V_DELIVERYINFO_NAME,
    NULL   /* Must be the last element, used as delimiter */
};

/**
 * \brief The error codes and messages used in client durability.
 *
 * The errorCode is a 32-bit number.
 * Bit number 23 is used as differentiator between a syntax error in
 * the request and the server being unable to respond to a synatically
 * correct message.
 *
 * Error code 0 indicates no error.
 * Error codes 1 - 2^23-1 indicates a syntax error.
 * Error codes 2^23 - 2^24-1 indicates a server error.
 * Error codes 2^24 - 2^32-1 are reserved for future use.
 */
const struct error d_Error[] = {
    /* no error */
    { D_DDS_RETCDE_NO_ERROR,                         "" },
    /* Syntax errors  -- 1 - 8388607 (= 1 .. 2^23-1) */
    { D_DDS_RETCDE_INCOMPATIBLE_VERSION,             "Incompatible version" },
    { D_DDS_RETCDE_NO_TOPIC,                         "topic is NULL"},
    { D_DDS_RETCDE_INVALID_START_TIME,               "Invalid startTime" },
    { D_DDS_RETCDE_INVALID_END_TIME,                 "Invalid endTime" },
    { D_DDS_RETCDE_INVALID_TIME_RANGE,               "startTime is later than endTime" },
    { D_DDS_RETCDE_INVALID_SERIAZATION,              "Invalid serializationFormat" },
    { D_DDS_RETCDE_NO_PARTITIONS,                    "Empty 'partitions' field" },
    { D_DDS_RETCDE_INVALID_MAX_SAMPLES,              "Invalid maxSamples" },
    { D_DDS_RETCDE_INVALID_MAX_INSTANCES,            "Invalid maxInstances" },
    { D_DDS_RETCDE_INVALID_MAX_SAMPLES_PER_INSTANCE, "Invalid maxSamplesPerInstance" },
    /* Syntax OK, but server is not able process -- 8388608 - (= 2^23 - 2^24-1) - */
    { D_DDS_RETCDE_SERVER_IS_NOT_ALIGNER,            "Server is not configured to be aligner for the requested dataset" },
    { D_DDS_RETCDE_SERVER_IS_NOT_RESPONSIBLE,        "Request will be handled by a different durability service" },
    { D_DDS_RETCDE_READER_NOT_KNOWN,                 "The HistoricalDataReader(s) have not yet been dicovered by the server" },
    { D_DDS_RETCDE_NO_MASTER_SELECTED,               "Currently a master has not been selected" },
    { D_DDS_RETCDE_GROUP_NOT_FOUND,                  "Currently no group found that match the request" },
    /* Currently unused ... 2^24 - 2^32-1 */
    { D_DDS_RETCDE_INVALID,                          "Invalid errorCode" }   /* END-MARKER, MUST BE THE LAST ELEMENT */
};


char *
d_getErrorMessage(
    c_ulong errorCode)
{
    c_ulong i=0;
    char *msg = NULL;
    c_bool cont = TRUE;

    while (cont) {
        if ( (d_Error[i].errorCode == errorCode) ||
             (d_Error[i].errorCode == 4294967295) ) {
            cont = FALSE;
            msg = d_Error[i].errorMessage;
        }
        i++;
    }
    return msg;
}


#define D_TOTAL_KEYWORDS 12
#define D_MIN_WORD_LENGTH 9
#define D_MAX_WORD_LENGTH 16
#define D_MIN_HASH_VALUE 9
#define D_MAX_HASH_VALUE 27
/* maximum key range = 19, duplicates = 0 */

#ifdef __GNUC__
__inline
#else
#ifdef __cplusplus
inline
#endif
#endif
static unsigned int
hashBuiltinTopicNames (str, len)
     register const char *str;
     register unsigned int len;
{
  static const unsigned char asso_values[] =
    {
      28, 28, 28, 28, 28, 28, 28, 28, 28, 28,
      28, 28, 28, 28, 28, 28, 28, 28, 28, 28,
      28, 28, 28, 28, 28, 28, 28, 28, 28, 28,
      28, 28, 28, 28, 28, 28, 28, 28, 28, 28,
      28, 28, 28, 28, 28, 28, 28, 28, 28, 28,
      28, 28, 28, 28, 28, 28, 28, 28, 28, 28,
      28, 28, 28, 28, 28, 28, 28, 28, 28, 28,
      28, 28, 28, 28, 28, 28, 28, 28, 28, 28,
      28, 28, 15, 28, 28, 28, 28, 10, 28, 28,
      28, 28, 28, 28, 28, 28, 28,  5,  0,  5,
      28, 28, 28, 28, 28,  0, 28, 28,  0, 28,
       5, 28,  0, 28,  5, 28, 28, 28, 28, 28,
      28, 28, 28, 28, 28, 28, 28, 28, 28, 28,
      28, 28, 28, 28, 28, 28, 28, 28, 28, 28,
      28, 28, 28, 28, 28, 28, 28, 28, 28, 28,
      28, 28, 28, 28, 28, 28, 28, 28, 28, 28,
      28, 28, 28, 28, 28, 28, 28, 28, 28, 28,
      28, 28, 28, 28, 28, 28, 28, 28, 28, 28,
      28, 28, 28, 28, 28, 28, 28, 28, 28, 28,
      28, 28, 28, 28, 28, 28, 28, 28, 28, 28,
      28, 28, 28, 28, 28, 28, 28, 28, 28, 28,
      28, 28, 28, 28, 28, 28, 28, 28, 28, 28,
      28, 28, 28, 28, 28, 28, 28, 28, 28, 28,
      28, 28, 28, 28, 28, 28, 28, 28, 28, 28,
      28, 28, 28, 28, 28, 28, 28, 28, 28, 28,
      28, 28, 28, 28, 28, 28
    };
  return len + asso_values[(unsigned char)str[6]];
}

const char *
d_inBuiltinTopicNames (str, len)
     register const char *str;
     register size_t len;
{
    /* Names of built-in topics are written here without using the defines
     * as the hash function needs updating as well if the topic names change.
     */
    static const char * const d_topicWordlist[] =
    {
      "", "", "", "", "", "", "", "", "",
      "DCPSTopic",
      "",
      "CMPublisher",
      "DCPSDelivery",
      "CMParticipant",
      "",
      "DCPSPublication",
      "DCPSSubscription",
      "CMSubscriber",
      "DCPSHeartbeat",
      "",
      "DCPSParticipant",
      "DCPSCandMCommand",
      "CMDataWriter",
      "", "", "", "",
      "CMDataReader"
    };

  if (len <= D_MAX_WORD_LENGTH && len >= D_MIN_WORD_LENGTH){
      unsigned int key = hashBuiltinTopicNames (str, (unsigned) len);

      if (key <= D_MAX_HASH_VALUE){
          register const char *s = d_topicWordlist[key];

          if (*str == *s && !strcmp (str + 1, s + 1))
            return s;
        }
    }
    return 0;
}


void
d_free (
    c_voidp allocated )
{
    if (allocated) {
        os_free(allocated);
    }
}

c_voidp
d_malloc (
    size_t      size,
    const char * errorText )
{
    OS_UNUSED_ARG(errorText);
    return os_malloc(size);
}

static void
d_doPrint(
    d_configuration config,
    const char* format,
    va_list args,
    const char* header
    )
{
    char description[1024];

    if(config->tracingOutputFile){
        os_vsnprintf(description, sizeof(description)-1, format, args);
        description [sizeof(description)-1] = '\0';
        if (header != NULL) {
            fprintf(config->tracingOutputFile, "%s%s", header, description);
        }
        else {
            fprintf(config->tracingOutputFile, "%s", description);
        }
        fflush(config->tracingOutputFile);

        if (config->tracingSynchronous) {
            os_fsync(config->tracingOutputFile);
        }
    }
}


void
d_printTimedEvent(
    d_durability durability,
    d_level level,
    const char *format,
    ...)
{
    va_list args;
    d_configuration config;
    char header[132];

    config = d_durabilityGetConfiguration(durability);

    if (config && (((c_ulong)level) >= ((c_ulong)config->tracingVerbosityLevel)))
    {
        d_printState(durability, config, d_threadSelfName(), header);
        va_start (args, format);
        d_doPrint(config, format, args, header);
        va_end (args);
    }
}

void
d_printEvent(
    d_durability durability,
    d_level level,
    const char *format,
    ...)
{
    va_list args;
    d_configuration config;

    config = d_durabilityGetConfiguration(durability);

    if (config && (((c_ulong)level) >= ((c_ulong)config->tracingVerbosityLevel)))
    {
        va_start (args, format);
        d_doPrint(config, format, args, NULL);
        va_end (args);
    }
}

void
d_reportLocalGroup(
    _In_ d_durability d,
    _In_ _Const_ v_group group)
{
    const c_char* durability;
    v_topicQos qos;

    qos = v_topicQosRef(group->topic);
    switch(qos->durability.v.kind){
        case V_DURABILITY_VOLATILE:
            durability = "VOLATILE";
            break;
        case V_DURABILITY_TRANSIENT_LOCAL:
            durability = "TRANSIENT LOCAL";
            break;
        case V_DURABILITY_TRANSIENT:
            durability = "TRANSIENT";
            break;
        case V_DURABILITY_PERSISTENT:
            durability = "PERSISTENT";
            break;
        default:
            durability = "<<UNKNOWN>>";
            (void)durability;
            assert(FALSE);
            break;
    }

    d_printTimedEvent(d, D_LEVEL_FINEST,
                      "Group found: %s.%s (%s)\n",
                      v_partitionName(v_groupPartition(group)),
                      v_topicName(v_groupTopic(group)),
                      durability);
}



void
d_printState(
    d_durability durability,
    d_configuration config,
    const char* threadName,
    char* header)
{
    os_timeW time;
    d_serviceState kind;
    const c_char* state;

    if (config->tracingOutputFile) {
        kind = d_durabilityGetState(durability);

        switch(kind){
            case D_STATE_INIT:
                state = "INIT";
                break;
            case D_STATE_DISCOVER_FELLOWS_GROUPS:
                state = "DISCOVER_FELLOWS_GROUPS";
                break;
            case D_STATE_DISCOVER_PERSISTENT_SOURCE:
                state = "DISCOVER_PERSISTENT_SOURCE";
                break;
            case D_STATE_INJECT_PERSISTENT:
                state = "INJECT_PERSISTENT";
                break;
            case D_STATE_DISCOVER_LOCAL_GROUPS:
                state = "DISCOVER_LOCAL_GROUPS";
                break;
            case D_STATE_FETCH_INITIAL:
                state = "FETCH_INITIAL";
                break;
            case D_STATE_FETCH:
                state = "FETCH";
                break;
            case D_STATE_ALIGN:
                state = "ALIGN";
                break;
            case D_STATE_FETCH_ALIGN:
                state = "FETCH_ALIGN";
                break;
            case D_STATE_COMPLETE:
                state = "COMPLETE";
                break;
            case D_STATE_TERMINATING:
                state = "TERMINATING";
                break;
            case D_STATE_TERMINATED:
                state = "TERMINATED";
                break;
            default:
                state = "<<UNKNOWN>>";
                break;
        }

        if (config->tracingTimestamps == TRUE) {
            os_char buf[OS_CTIME_R_BUFSIZE];
            time = os_timeWGet();
            if (config->tracingRelativeTimestamps == TRUE) {
                /* relative timestamps, use the monotonic clock for timestamping log messages */
                os_duration delta = os_timeWDiff(time, config->startWTime);
                time = os_timeWAdd(OS_TIMEW_ZERO, delta);
            }
            os_ctimeW_r(&time, buf, OS_CTIME_R_BUFSIZE);
            os_sprintf(header, "%s %"PA_PRItime" %s (%s) -> ",
                buf, OS_TIMEW_PRINT(time), state, threadName);
        } else {
            os_sprintf(header, "%s (%s) -> ", state, threadName);
        }
    }
}

static void
d_findBaseAction(
    v_public entity,
    c_voidp args)
{
    struct baseFind* data;

    data = (struct baseFind*)args;
    data->base = c_getBase(entity);
}


c_base
d_findBase(
    d_durability durability)
{
    u_service service;
    struct baseFind data;

    service = d_durabilityGetService(durability);
    (void)u_observableAction(u_observable(service), d_findBaseAction, &data);

    return data.base;
}

c_bool
d_patternMatch(
    const char* str,
    const char* pattern)
{
    c_value p,n,r;

    p.kind = n.kind = V_STRING;
    p.is.String = (char *)pattern;
    n.is.String = (char *)str;
    r = c_valueStringMatch(p,n);
    return r.is.Boolean;
}


c_bool
d_isBuiltinGroup(
    _In_z_ _Const_ d_partition partition,
    _In_z_ _Const_ d_topic topic)
{
    assert(partition);
    assert(topic);
    if(strcmp(partition, V_BUILTIN_PARTITION) != 0) {
        return FALSE;
    } else {
        int i;
        for (i = 0; d_builtinTopics[i] != NULL; i++) {
            if (strcmp (topic, d_builtinTopics[i]) == 0) {
                return TRUE;
            }
        }
        return FALSE;
    }
}

c_bool
d_isHeartbeatGroup(
    _In_z_ _Const_ d_partition partition,
    _In_z_ _Const_ d_topic topic)
{
    if(strcmp(partition, V_BUILTIN_PARTITION) != 0) {
        return FALSE;
    } else if (strcmp(topic, V_HEARTBEATINFO_NAME) != 0) {
        return FALSE;
    } else {
        return TRUE;
   }
}

c_bool
d_shmAllocAssert(
    c_voidp ptr,
    const char* errorMessage)
{
    if(ptr == NULL){
        if(errorMessage){
            d_printTimedEvent(d_threadsDurability(), D_LEVEL_SEVERE, errorMessage);
        }
        d_printTimedEvent(d_threadsDurability(), D_LEVEL_SEVERE,
            "Unrecoverable error: shared memory allocation failed; terminating.");
        OS_REPORT(OS_ERROR, D_CONTEXT_DURABILITY, 0,
            "Unrecoverable error: shared memory allocation failed; terminating");
        d_durabilityTerminate(d_threadsDurability(), TRUE);
        return FALSE;
    }
    return TRUE;
}

c_bool
d_shmAllocAllowed(void)
{
    c_base base;
    c_memoryThreshold status;

    base = d_findBase(d_threadsDurability());

    if(base) {
        status = c_baseGetMemThresholdStatus(base);

        if(status == C_MEMTHRESHOLD_SERV_REACHED){
            d_printTimedEvent(d_threadsDurability(), D_LEVEL_SEVERE,
                "Unrecoverable error: service memory threshold reached; terminating.");
            OS_REPORT(OS_ERROR, D_CONTEXT_DURABILITY, 0,
                "Unrecoverable error: service memory threshold reached; terminating.");
            d_durabilityTerminate(d_threadsDurability(), TRUE);
            return FALSE;
        }
    }
    return TRUE;
}


/**
 * \brief Map a d_completeness value to a _DDS_Completeness_t
 *        value
 *
 * The durability service internaly uses d_completeness for the
 * completeness of groups. Client-durability uses a  _DDS_Completeness_t
 * value.
 */
_DDS_Completeness_t
d_mapCompleteness(
    d_completeness completeness)
{
    switch (completeness) {
        case D_GROUP_KNOWLEDGE_UNDEFINED:
            /* D_GROUP_KNOWLEDGE_UNDEFINED is used by durability to indicate
             * that the group is not known
             */
            return D_DDS_COMPLETENESS_UNKNOWN;
        case D_GROUP_UNKNOWN:
        case D_GROUP_INCOMPLETE:
            /* D_GROUP_UNKNOWN and D_GROUP_INCOMPLETE are used by durability
             * to indicate that the group is known but not yet complete
             */
            return D_DDS_COMPLETENESS_INCOMPLETE;
        case D_GROUP_COMPLETE:
            /* D_GROUP_COMPLETE is used by durability to indicate that the
             * group is complete
             */
            return D_DDS_COMPLETENESS_COMPLETE;
        default:
            assert(FALSE);
            return D_GROUP_KNOWLEDGE_UNDEFINED;
    }
}

unsigned short d_swap2uToBE (unsigned short x)
{
#ifdef PA_LITTLE_ENDIAN
    return (unsigned short) ((x >> 8) | (x << 8));
#else
    return x;
#endif
}

unsigned d_swap4uToBE (unsigned x)
{
#ifdef PA_LITTLE_ENDIAN
    return (x >> 24) | ((x >> 8) & 0xff00) | ((x << 8) & 0xff0000) | (x << 24);
#else
    return x;
#endif
}


unsigned long long d_swap8uToBE (unsigned long long x)
{
#ifdef PA_LITTLE_ENDIAN
    const unsigned newhi = d_swap4uToBE ((unsigned) x);
    const unsigned newlo = d_swap4uToBE ((unsigned) (x >> 32));
    return ((unsigned long long) newhi << 32) | (unsigned long long) newlo;
#else
    return x;
#endif
}


/**
 * \brief Copy out function for c_sequence<string>
 *
 * This function copies the sequence of string to
 * an iter.
 */
c_bool
sequenceOfStringCopyOut(
    c_iter *to,
    const c_sequence from)
{
    c_ulong length, i;
    size_t len;
    c_string *fromStr, toStr;
    c_bool result;

    assert(to);

    result = TRUE;
    fromStr = (c_string*)from;
    /* The generic copy routines do not always handle c_sequenceSize
     * correctly, but luckily we can just as well use c_arraySize.
     */
    length = c_arraySize(from);
    for (i = 0; i < length; i++) {
        len = strlen(fromStr[i]);
        toStr = os_malloc(len+1);  /* add '\0' */
        toStr = os_strncpy(toStr, fromStr[i], len);
        toStr[len] = '\0';
        *to = c_iterAppend(*to, toStr);
    } /* for */
    return result;
}


d_durabilityKind
d_durabilityKindFromKernel(
    v_durabilityKind kind)
{
    switch(kind){
        case V_DURABILITY_VOLATILE:         return D_DURABILITY_VOLATILE;
        case V_DURABILITY_TRANSIENT_LOCAL:  return D_DURABILITY_TRANSIENT_LOCAL;
        case V_DURABILITY_TRANSIENT:        return D_DURABILITY_TRANSIENT;
        case V_DURABILITY_PERSISTENT:       return D_DURABILITY_PERSISTENT;
    }
    assert(FALSE);
    return D_DURABILITY_VOLATILE;
}


const char *
d_compressionKVImage(
    d_compressionKV compression)
{
    const char *image;
    switch (compression) {
    case D_COMPRESSION_NONE:    image = "none";     break;
    case D_COMPRESSION_LZF:     image = "lzf";      break;
    case D_COMPRESSION_SNAPPY:  image = "snappy";   break;
    case D_COMPRESSION_ZLIB:    image = "zlib";     break;
    case D_COMPRESSION_CUSTOM:  image = "custom";   break;
    default:                    image = "(unknown)";break;
    }
    return image;
}


/* Prints trace messages in case the mask fits the trace settting.
 * The messages are only printed when verbosity level is D_LEVEL_FINEST
 */
void
d_trace (c_ulong mask, const char *fmt, ...)
{
    d_configuration config;
    d_durability durability = d_threadsDurability();

    assert(d_durabilityIsValid(durability));

    config = d_durabilityGetConfiguration(durability);
    if ((config) && (mask & config->traceMask) && (config->tracingVerbosityLevel == D_LEVEL_FINEST)) {
        char message[1024];
        va_list args;

        va_start(args, fmt);
        (void)os_vsnprintf(message, sizeof(message)-1, fmt, args);
        va_end(args);
        message[sizeof(message)-1] = '\0';
        d_printTimedEvent(durability, D_LEVEL_FINEST, "[0x%08lx] %s", mask & config->traceMask, message);
    }
}



void d_tracegroupGenKeystr(char *keystr, size_t keystr_size, v_groupInstance gi)
{
    d_durability durability = d_threadsDurability();
    d_configuration config = d_durabilityGetConfiguration(durability);
    if (config && (config->traceMask & D_TRACE_GROUP)) {
        v_groupInstanceKeyToString(gi, keystr, keystr_size);
    }
}

void d_tracegroupGenMsgKeystr(char *keystr, size_t keystr_size, v_group g, v_message msg)
{
    d_durability durability = d_threadsDurability();
    d_configuration config = d_durabilityGetConfiguration(durability);
    if (config && (config->traceMask & D_TRACE_GROUP)) {
        c_array messageKeyList = v_topicMessageKeyList(v_groupTopic(g));
        c_ulong i, nrOfKeys = c_arraySize(messageKeyList);
        size_t pos = 0;
        assert (keystr_size >= 4); /* for ...\0 */
        for (i = 0; i < nrOfKeys; i++)
        {
            c_value v = c_fieldValue(messageKeyList[i], msg);
            char *vimg = c_valueImage(v);
            int n = snprintf(keystr + pos, keystr_size - pos, "%s%s", (i == 0) ? "" : ";", vimg);
            c_valueFreeRef(v);
            os_free(vimg);
            if (n > 0) { pos += (size_t)n; } else { break; }
        }
        if (i < nrOfKeys || pos >= keystr_size) {
            if (pos >= keystr_size - 4) {
                pos = keystr_size - 4;
            }
            strcpy(keystr + pos, "...");
        }
    }
}

void d_tracegroupInstance(v_groupInstance gi, d_durability durability, const char *prefix)
{
    d_configuration config = d_durabilityGetConfiguration(durability);

    if (config && (config->traceMask & D_TRACE_GROUP)) {
        v_groupSample s;
        char keystr[1024];
        d_tracegroupGenKeystr(keystr, sizeof (keystr), gi);
        d_trace(D_TRACE_GROUP, "%sInstance %p state=%u epoch=%"PA_PRItime" key={%s}\n", prefix, (void*)gi, gi->state, OS_TIMEE_PRINT(gi->epoch), keystr);
        s = v_groupSample(gi->oldest);
        while (s != NULL) {
            v_message msg = v_groupSampleTemplate(s)->message;
            d_trace(D_TRACE_GROUP, "%s  Sample %p msg %p state=%u time=%"PA_PRItime" wrgid=%u:%u:%u\n",
                  prefix, (void*)s, (void*)msg, msg->_parent.nodeState, OS_TIMEW_PRINT(msg->writeTime), msg->writerGID.systemId, msg->writerGID.localId, msg->writerGID.serial);
            s = s->newer;
        }
    }
}

static c_bool tracegroupHelper(c_object obj, void *durability)
{
    d_tracegroupInstance ((v_groupInstance)obj, durability, "  ");
    return 1;
}


void d_tracegroup(d_durability durability, v_group g, const char *info)
{
    d_configuration config = d_durabilityGetConfiguration(durability);

    if (config && (config->traceMask & D_TRACE_GROUP)) {
        d_trace(D_TRACE_GROUP, "Group %s.%s lastDisposeAllTime=%"PA_PRItime" - %s\n",
            v_partitionName(v_groupPartition(g)), v_topicName(v_groupTopic(g)), OS_TIMEW_PRINT(g->lastDisposeAllTime), info);
        v_groupWalkInstances(g, tracegroupHelper, durability);
    }
}

os_compare
d_qualityCompare(
    d_quality q1,
    d_quality q2)
{
    /* Quality is internally modelled as an os_timeW */
    return os_timeWCompare(q1, q2);
}


void
d_timestampFromTimeW(d_timestamp *t1, os_timeW *t2, c_bool Y2038Ready)
{
    assert(t1 && t2);

    if (Y2038Ready) {
        t1->seconds = (c_long)((OS_TIMEW_GET_VALUE(*t2)) >> 32);
        t1->nanoseconds = (c_ulong)((OS_TIMEW_GET_VALUE(*t2) & 0xFFFFFFFFU));
    } else {
        *t1 = c_timeFromTimeW(*t2);
    }
}


void
d_timestampToTimeW(os_timeW *t1, d_timestamp *t2, c_bool Y2038Ready)
{
    assert(t1 && t2);

    if (Y2038Ready) {
        *t1 = OS_TIMEW_INIT(0, ((os_uint64)((t2->seconds)) << 32) | (os_uint64)(t2->nanoseconds));
    } else {
        *t1 = c_timeToTimeW(*t2);
    }
}


void
d_qualityExtFromQuality(d_qualityExt *q1, d_quality *q2, c_bool Y2038Ready)
{
    /* quality is an os_timeW and qualityExt is a d_timestamp,
     * so we can use d_timestampFromTimeW
     */
    d_timestampFromTimeW(q1, q2, Y2038Ready);
}


void
d_qualityExtToQuality(d_quality *q1, d_qualityExt *q2, c_bool Y2038Ready)
{
    /* qualityExt is a d_timestamp and quality is a os_timeW,
     * so we can use d_timestampToTimeW
     */
    d_timestampToTimeW(q1, q2, Y2038Ready);
}


void
d_productionTimestampToSeqnum(os_uint32 *seqnum, d_timestamp *t)
{
    if ((t->nanoseconds) & (1U << 30)) {
         /* Bit 30 of the nanoseconds fields is set so interpret
          * the seconds field as a sequence number
          */
         *seqnum = (os_uint32)(t->seconds);
     } else {
         /* No sequence number support, use 0 as sequence number
          * to ensure that message are always processed
          */
         *seqnum = (os_uint32)0;
     }
}

void
d_productionTimestampFromSeqnum(d_timestamp *t, os_uint32 *seqnum)
{
    if (t->nanoseconds & (1ul << 30)) {
        /* Sequence numbers are supported.
         * Fill the seconds field of the productionTimestamp
         * with the sequence number. The nanoseconds field
         * remains unchanged
         */
        t->seconds = (c_long)(*seqnum);
    } else {
        /* No support for sequence numbers.
         * Fill the productionTimestamp with the elapsed time
         * while retaining the flags.
         * NOTE: because seconds are casted to as c_long
         * they may not exceed 0x7FFFFFFF. Effectively, this
         * means that a node must reboot every 68 years .
         */
        os_timeE t1 = os_timeEGet();
        os_uint32 flags = t->nanoseconds & ((1u << 31) | (1u << 30));
        t->seconds = (c_long)OS_TIMEE_GET_SECONDS(t1);
        t->nanoseconds = OS_TIMEE_GET_NANOSECONDS(t1);
        /* Restore the flags */
        t->nanoseconds = t->nanoseconds | flags;
    }
}

/* Calculate the corresponding elapsed time from a wall clock time.
 *
 */
os_timeE
d_timeWToTimeE(os_timeW t)
{
    os_duration d;
    os_timeE te;

    d = os_timeWDiff(os_timeWGet(), t);
    te = os_timeEAdd(os_timeEGet(), d);
    return te;
}

void d_traceMergeAction (d_mergeAction mergeAction, const char *info)
{
    char str[1024];
    d_tableIter tableIter;
    d_fellow fellow;
    size_t pos = 0;
    d_chain chain;

    /* determine the list of fellows to be addressed */
    str[0] = '\0';
    fellow = d_fellow(d_tableIterFirst(mergeAction->fellows, &tableIter));
    while (fellow) {
        int n = snprintf(str + pos, sizeof(str) - pos, "%s%u", (strcmp(str, "") == 0) ? "" : ",", fellow->address->systemId);
        if (n > 0) { pos += (size_t)n; } else { break; }
        fellow = d_fellow(d_tableIterNext(&tableIter));
    }
    /* print the contents of the queue before */
    d_trace(D_TRACE_CHAINS, "%s: merge action %p, conflict %u, nameSpace %s, fellows [%s]\n",
            info, (void *)mergeAction, mergeAction->conflict->id, mergeAction->nameSpace->name, str);

    chain = d_chain(d_tableIterFirst(mergeAction->chains, &tableIter));
    while (chain) {
        d_traceChain(chain);
        chain = d_chain(d_tableIterNext(&tableIter));
    }
}

