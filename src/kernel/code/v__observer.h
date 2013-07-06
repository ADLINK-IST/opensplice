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

#ifndef V__OBSERVER_H
#define V__OBSERVER_H

#if defined (__cplusplus)
extern "C" {
#endif

#include "v_observer.h"

#define v__observerClearEventFlags(_this) \
        (v_observer(_this)->eventFlags = 0)

/**
 * The initialisation of an observer object.
 * This method initialises all attributes of the observer class and
 * is called by all derived classes.
 *
 * \param _this the reference to an observer object.
 * \param name  the name of the observer.
 */
void
v_observerInit (
    v_observer _this,
    const c_char *name,
    v_statistics s,
    c_bool enable);

/**
 * The de-initialisation of an observer object.
 * This method releases all used resources by the observer object and
 * is called by all derived classes.
 *
 * \param _this  the reference to an observer object.
 */
void
v_observerDeinit (
    v_observer _this);

void
v_observerFree (
    v_observer _this);

/**
 * Retrieves and stores event data in the observer in one transaction.
 * 
 * Retrieves and stores event data in the observer in a thread-safe manner.
 * It must be thread-safe, since the data can be used to determine whether
 * the observer must be triggered or not.
 * 
 * \param _this     the reference to an observer object.
 * \param eventData the event data to store in the observer.
 * \return          the overwritten event data in the observer.
 */
c_voidp
v_observerSetEventData(
    v_observer _this,
    c_voidp eventData);

c_ulong
v__observerWait(
    v_observer _this);

c_ulong
v__observerTimedWait(
    v_observer _this,
    const c_time time);

#if defined (__cplusplus)
}
#endif

#endif
