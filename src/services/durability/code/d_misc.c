#include "d_misc.h"
#include "d__misc.h"
#include "d_durability.h"
#include "d_configuration.h"
#include "v_entity.h"
#include "v_group.h"
#include "v_topic.h"
#include "v_domain.h"
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
    va_list args)
{
    char description[512];

    if(config->tracingOutputFile){
        vsnprintf(description, sizeof(description)-1, format, args);
        description [sizeof(description)-1] = '\0';
        fprintf(config->tracingOutputFile, description);
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

    config = d_durabilityGetConfiguration(durability);

    if (config && (((c_ulong)level) >= ((c_ulong)config->tracingVerbosityLevel)))
    {
        d_printState(durability, config, threadName);
        va_start (args, format);
        d_doPrint(config, format, args);
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
        d_doPrint(config, format, args);
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
    const char* threadName)
{
    os_time time;
    d_durabilityKind kind;
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
            fprintf(config->tracingOutputFile, "%d.%9.9d %s (%s) -> ",
                    time.tv_sec, time.tv_nsec, state, threadName);
        } else {
            fprintf(config->tracingOutputFile, "%s (%s) -> ", state, threadName);
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

