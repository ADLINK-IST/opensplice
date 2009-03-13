#ifndef GAPI_SCHEDULER_H
#define GAPI_SCHEDULER_H

#include "gapi.h"

#include "v_kernel.h"

void
gapi_scheduleToKernel (
    const gapi_schedulingQosPolicy *gapi_sched,
    struct v_schedulePolicy *v_sched);

void
gapi_scheduleFromKernel (
    const struct v_schedulePolicy *v_sched,
    gapi_schedulingQosPolicy *gapi_sched);

void
gapi_threadAttrInit (
    const gapi_schedulingQosPolicy *gapi_sched,
    os_threadAttr *threadAttr);

#endif
