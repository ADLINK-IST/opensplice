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
