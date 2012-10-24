/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2011 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE
 *
 *   for full copyright notice and license terms.
 *
 */
#ifndef IN_CLIENTMONITOR_H_
#define IN_CLIENTMONITOR_H_

#include "in_commonTypes.h"
#include "in_runnable.h"
#include "u_participant.h"
#include "c_time.h"

/** \brief Callback operation
 *
 */
typedef void (*in_clientMonitorParticipantAction) (
    in_runnable _this,
    v_state sampleState,
    v_state instanceState,
    v_message msg,
    struct v_participantInfo *data);

/** \brief Callback operation */
typedef void (*in_clientMonitorSubscriptionAction) (
    in_runnable _this,
    v_state sampleState,
    v_state instanceState,
    v_message msg,
    struct v_subscriptionInfo *data);

/** \brief Callback operation */
typedef void (*in_clientMonitorPublicationAction) (
    in_runnable _this,
    v_state sampleState,
    v_state instanceState,
    v_message msg,
    struct v_publicationInfo *data);

/** \brief Callback operation */
typedef void (*in_clientMonitorPeriodicAction) (
    in_runnable _this);


/** \brief class
 *
 * Attaching to participants, subscriptions, and publications, invoking
 * callbacks in case the status changes */
OS_CLASS(in_clientMonitor);
OS_STRUCT(in_clientMonitor)
{
	in_runnable runnable;
	u_participant participant;
    c_time periodic;
    in_clientMonitorParticipantAction   participantAction;
    in_clientMonitorSubscriptionAction  subscriptionAction;
    in_clientMonitorPublicationAction   publicationAction;
    in_clientMonitorPeriodicAction      periodicAction;
};

/** \brief Initializer */
void
in_clientMonitorInit(in_clientMonitor _this,
    in_runnable runnable,
    u_participant participant,
    c_time periodic,
    in_clientMonitorParticipantAction   participantAction,
    in_clientMonitorSubscriptionAction  subscriptionAction,
    in_clientMonitorPublicationAction   publicationAction,
    in_clientMonitorPeriodicAction      periodicAction);

/** \brief Finalizer */
void
in_clientMonitorDeinit(in_clientMonitor _this);

/** \brief Event loop until runnable is terminated */
os_boolean
in_clientMonitorRun(in_clientMonitor _this);

void
in_clientMonitorTrigger(in_clientMonitor _this);

#endif /* IN_CLIENTMONITOR_H_ */
