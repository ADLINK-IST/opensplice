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

#ifndef U__QOS_H
#define U__QOS_H

#if defined (__cplusplus)
extern "C" {
#endif

#include "u_partitionQos.h"
#include "u_participantQos.h"
#include "u_topicQos.h"
#include "u_writerQos.h"
#include "u_readerQos.h"
#include "u_publisherQos.h"
#include "u_subscriberQos.h"
#include "u_dataViewQos.h"

v_qos u_qosNew(v_qos template);

#if defined (__cplusplus)
}
#endif

#endif /* U__QOS_H */
