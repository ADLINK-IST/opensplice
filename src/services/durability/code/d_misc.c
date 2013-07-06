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
#include "d_misc.h"
#include "d__misc.h"
#include "d_durability.h"
#include "d_configuration.h"
#include "v_entity.h"
#include "v_group.h"
#include "v_topic.h"
#include "v_partition.h"
#include "os_report.h"
#include "os_stdlib.h"

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
    os_uint32      size,
    const char * errorText )
{
    c_voidp allocated;

    allocated = os_malloc(size);
    if (allocated == NULL) {
        printf(RR_MALLOC_FAILED, errorText);
        OS_REPORT_1(OS_WARNING, D_SERVICE_NAME, 0, RR_MALLOC_FAILED, errorText);
    }
    return allocated;
}

void
d_conv_os2c_time(
    c_time *  time,
    os_time * osTime )
{
    time->seconds     = osTime->tv_sec;
    time->nanoseconds = osTime->tv_nsec;
}

static void
d_doPrint(
    d_configuration config,
    const char* format,
    va_list args,
    const char* header
    )
{
    char description[512];

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

        if(config->tracingSynchronous){
            os_fsync(config->tracingOutputFile);
        }
    }
}


void
d_printTimedEvent(
    d_durability durability,
    d_level level,
    const char *threadName,
    const char *format,
    ...)
{
    va_list args;
    d_configuration config;
    char header[100];

    config = d_durabilityGetConfiguration(durability);

    if (config && (((c_ulong)level) >= ((c_ulong)config->tracingVerbosityLevel)))
    {
        d_printState(durability, config, threadName, header);
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
    d_durability d,
    const char *threadName,
    v_group group)
{
    const c_char* durability;
    v_topicQos qos;

    if(group){
        qos = v_topicQosRef(group->topic);
        switch(qos->durability.kind){
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
                assert(FALSE);
                break;
        }
    } else {
        durability = "<<UNKNOWN>>";
        assert(FALSE);
    }
    d_printTimedEvent(d, D_LEVEL_FINE,
                      threadName, "Group found: %s.%s (%s)\n",
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
    os_time time;
    d_serviceState kind;
    const c_char* state;

    if(config->tracingOutputFile){
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

        if(config->tracingTimestamps == TRUE){
            time = os_timeGet();

            if(config->tracingRelativeTimestamps == TRUE){
                time = os_timeSub(time, config->startTime);
            }
            os_sprintf(header, "%d.%9.9d %s (%s) -> ",
                time.tv_sec, time.tv_nsec, state, threadName);
        } else {
            os_sprintf(header, "%s (%s) -> ", state, threadName);
        }
    }
}

c_base
d_findBase(
    d_durability durability)
{
    u_service service;
    struct baseFind data;

    service = d_durabilityGetService(durability);
    u_entityAction(u_entity(service), d_findBaseAction, &data);

    return data.base;
}

void
d_findBaseAction(
    v_entity entity,
    c_voidp args)
{
    struct baseFind* data;

    data = (struct baseFind*)args;
    data->base = c_getBase(entity);
}

c_bool
d_patternMatch(
    const char* str,
    const char* pattern )
{
    c_bool   stop = FALSE;
    c_bool   matches = FALSE;
    c_string strRef = NULL;
    c_string patternRef = NULL;

    /* QAC EXPECT 2106,2100; */
    while ((*str != 0) && (*pattern != 0) && (stop == FALSE)) {
        /* QAC EXPECT 2106,3123; */
        if (*pattern == '*') {
            /* QAC EXPECT 0489; */
            pattern++;
            /* QAC EXPECT 2106; */
            while ((*str != 0) && (*str != *pattern)) {
                /* QAC EXPECT 0489; */
                str++;
            }
            /* QAC EXPECT 2106; */
            if (*str != 0) {
                /* QAC EXPECT 0489; */
                strRef = (c_string)(str+1); /* just behind the matching char */
                patternRef = (c_string)(pattern-1); /* on the '*' */
            }
        /* QAC EXPECT 2106,3123; */
        } else if (*pattern == '?') {
            /* QAC EXPECT 0489; */
            pattern++;
            /* QAC EXPECT 0489; */
            str++;
        /* QAC EXPECT 2004,3401,0489,2106; */
        } else if (*pattern++ != *str++) {
            if (strRef == NULL) {
                matches = FALSE;
                stop = TRUE;
            } else {
                str = strRef;
                pattern = patternRef;
                strRef = NULL;
            }
        }
    }
    /* QAC EXPECT 3892,2106,2100; */
    if ((*str == (char)0) && (stop == FALSE)) {
        /* QAC EXPECT 2106,3123; */
        while (*pattern == '*') {
            /* QAC EXPECT 0489; */
            pattern++;
        }
        /* QAC EXPECT 3892,2106; */
        if (*pattern == (char)0) {
            matches = TRUE;
        } else {
            matches = FALSE;
        }
    } else {
        matches = FALSE;
    }
    return matches;
    /* QAC EXPECT 5101; */
}

