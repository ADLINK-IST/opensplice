/* OS */
#include <os_time.h>

/* SAC */
#include <dds_dcps.h>

/* C99 */
#include <dds/time.h>
#include <dds__time.h>

dds_time_t
dds_time (void)
{
    return (dds_time_t)OS_TIMEW_GET_VALUE(os_timeWGet());
}

void
dds_sleepfor (
    dds_duration_t n)
{
    if (n == DDS_INFINITY) {
        os_sleep(OS_DURATION_INFINITE);
    } else {
        os_sleep((os_duration)n);
    }
}

void
dds_sleepuntil (
    dds_time_t n)
{
    dds_sleepfor(dds_delta_from_now(n));
}

DDS_Duration_t
dds_duration_to_sac(
    dds_duration_t d)
{
    DDS_Duration_t value;

    if (d == DDS_INFINITY) {
        value.sec = DDS_DURATION_INFINITE_SEC;
        value.nanosec = DDS_DURATION_INFINITE_NSEC;
    } else {
        value.sec = OS_DURATION_GET_SECONDS((os_duration)d);
        value.nanosec = OS_DURATION_GET_NANOSECONDS((os_duration)d);
    }

    return value;
}

dds_duration_t
dds_duration_from_sac(
    DDS_Duration_t d)
{
    dds_duration_t value;

    if ((d.sec == DDS_DURATION_INFINITE_SEC) && (d.nanosec == DDS_DURATION_INFINITE_NSEC)) {
        value = DDS_INFINITY;
    } else {
        value = ((dds_duration_t)d.sec) * DDS_NSECS_IN_SEC + d.nanosec;
    }

    return value;
}

DDS_Time_t
dds_time_to_sac(
    dds_time_t t)
{
    DDS_Time_t value;

    if (t == DDS_NEVER) {
        value.sec = DDS_TIMESTAMP_INVALID_SEC;
        value.nanosec = DDS_TIMESTAMP_INVALID_NSEC;
    } else {
        value.sec = ((t)/DDS_NSECS_IN_SEC);
        value.nanosec = ((os_uint32)((t)%DDS_NSECS_IN_SEC));
    }

    return value;
}

dds_duration_t
dds_delta_from_now(
    dds_time_t n)
{
    dds_duration_t delta = DDS_INFINITY;
    if (n != DDS_NEVER) {
        os_timeW t;
        OS_TIMEW_SET_VALUE(t, n);
        delta = (dds_duration_t)os_timeWDiff(t, os_timeWGet());
    }
    return delta;
}
