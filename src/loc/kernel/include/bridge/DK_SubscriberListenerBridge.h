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
#ifndef DLRL_KERNEL_SUBSCRIBER_LISTENER_BRIDGE_H
#define DLRL_KERNEL_SUBSCRIBER_LISTENER_BRIDGE_H

#if defined (__cplusplus)
extern "C" {
#endif

/* \brief Should return a pointer containing userdata information which will be used in a similar manner as
 * user data provided when doing a call on a kernel operation from a language specific context.
 *
 * This function is called when the DLRL is triggered by a DCPS middleware thread, when this happens the DLRL does 
 * not have the possibly required language specific user data to pass along to the various callback functions.
 * This function is used to get the user data and allow the DLRL to continue as normal. Its important to realize that 
 * the DLRL kernel doesnt need the user data itself, but only passes along the result to any callbacks it makes.
 *
 * Mutex claims during this operation:<ul>
 * <li>The update mutex of a <code>DK_CacheAdmin</code> is locked, since no parameters are given in this operation
 * is not possible to determine which <code>DK_CacheAdmin</code> is locked.</li></ul>
 *
 * \return <code>NULL</code> or thread specific user data.
 */
typedef void* (*DK_SubscriberListenerBridge_us_getCallbackThreadUserData)();

typedef struct DK_SubscriberListenerBridge_s{
     DK_SubscriberListenerBridge_us_getCallbackThreadUserData getCallbackThreadUserData; /* may be a NIL pointer */
} DK_SubscriberListenerBridge;

extern DK_SubscriberListenerBridge subscriberListenerBridge;


#if defined (__cplusplus)
}
#endif

#endif /* DLRL_KERNEL_SUBSCRIBER_LISTENER_BRIDGE_H */
