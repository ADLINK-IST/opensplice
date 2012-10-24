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
#ifndef V_OBSERVER_H
#define V_OBSERVER_H

/**
 * \class v_observer
 *
 * The observer observes a set of observables. When the state of an observable
 * changes, the observers are notified. An observer can specify the kind of
 * events it is interested in to prevent unnecessary notifications.
 *
 * Every derived class must obey rules explained in the following paragraphs.
 *
 * <b>Notification</b>:
 * When a derived class implements its own notification method it must be called
 * from <code>v_observerNotify</code>. So the implementation of 
 * <code>v_observerNotify</code> is modified each time a derived observer
 * implements its own notification method.
 *
 * <b>Thread safety</b>:
 * \todo current implementation does not obey this rule!
 * Each derived observer should use <code>v_observerLock</code> and
 * <code>v_observerUnlock</code> to protect its data. It may introduce its own
 * mutex (or lock) to protect its data, but must take into account the 
 * scheduling penalties it introduces. If an observer deviates from this rule
 * it MUST be documented!
 * What scheduling penalties are introduced when the observer mutex is not
 * used? The waiting task (thread or process) might run twice to into a lock,
 * still owned by the triggering task: first the observer mutex and next the
 * mutex of the derived class.
 */

#include "v_kernel.h"
#include "v_event.h"
#include "os_if.h"

#ifdef OSPL_BUILD_KERNEL
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

/**
 * \brief The <code>v_observer</code> cast method.
 *
 * This method casts a kernel object to a <code>v_observer</code> object.
 * Before the cast is performed, the type of the object is checked to
 * be <code>v_observer</code> or one of its subclasses.
 */
#define v_observer(o) (C_CAST(o,v_observer))

/**
 * Notify the observer.
 * This method notifies the observer.
 * The event parameter specifies the reason of the notification.
 * Each derived class of v_observer may implement a notify method,
 * which is called on observer notification. When a derived class does
 * implement a notify method, the implementation of v_observerNotify must
 * be extended to call that method. It is up to each derived class to
 * perform additional actions on the notification (e.g. administrating
 * the event). The observer is only notified when the observer is interested
 * in the given event (specified by the eventmask).
 *
 * Note: The observer must be locked (using <code>v_observerLock</code>) before
 * calling this method!
 *
 * \param _this    The reference to an observer object.
 * \param event    The event object specifying the reason of notification.
 * \param userData The data that has been associated with this observer when
 *                 added to an observable.
 */
OS_API void
v_observerNotify(
    v_observer _this,
    v_event event,
    c_voidp userData);

/**
 * Wait until the condition of the observer becomes true.
 * Before the caller is blocked, the condition for the observer is checked.
 * While the condition is false, the caller is blocked.
 * The condition is determined by:
 * - the condition method implemented by the subclass and
 * - whether an event is received the observer is interested in.
 *
 * \param _this The reference to an observer object.
 * \return      A mask of the event kinds that have taken place.
 */
OS_API c_ulong
v_observerWait(
    v_observer _this);

/**
 * Wait until the condition of the observer becomes true or more time elapses
 * than the specified timeout.
 * This method is identical to <code>v_observerWait</code> but now a timeout
 * can be specified. When more time elapses than the specified timeout parameter
 * the waiting observer is woken-up regardless whether an event is received.
 *
 * \param _this   The reference to an observer object.
 * \param timeout Specifies the maximum time to wait on an event.
 * \return        A mask of the event kinds that have taken place.
 */
OS_API c_ulong
v_observerTimedWait(
    v_observer _this,
    const c_time timeout);

/**
 * Add the event(s) the observer is interested in.
 *
 * An observer can specify the event(s) it is interested in.
 * The event(s) is added to the current mask set (logical OR).
 * The previous value of the event mask is returned by this method.
 *
 * \param _this The reference to an observer object.
 * \param event The interested event(s).
 * \return      The previous set of interested events as a mask.
 */
OS_API c_ulong
v_observerSetEvent (
    v_observer _this,
    c_ulong event);

/**
 * Clear the event(s) the observer is no longer interested in.
 *
 * An observer can specify the event(s) it is no longer interested in.
 * The event(s) is removed from the current mask set (logical AND of NOT event).
 * The previous value of the event mask is returned by this method.
 *
 * \param _this The reference to an observer object.
 * \param event The interested event(s).
 * \return      The previous set of interested events as a mask.
 */
OS_API c_ulong
v_observerClearEvent (
    v_observer _this,
    c_ulong event);

/**
 * Get the set of events the observer is interested in.
 *
 * An observer can specify the events it is interested in.
 * The set of interested events is returned as a mask.
 *
 * \param _this The reference to an observer object.
 * \return      The set of interested events as a mask.
 */
OS_API c_ulong
v_observerGetEventMask(
    v_observer _this);

/**
 * Specify the events the observer is interested in.
 *
 * To prevent unnecessary wake-ups an observer is able to specify the events
 * it is interested in. This set of events can be specified as a bit-mask.
 * Any pending events are cleared by this call.
 *
 * \param _this The reference to an observer object.
 * \param mask  The set of interested events
 *
 *  \return     This method will return the previous event mask.
 */
OS_API c_ulong
v_observerSetEventMask(
    v_observer _this,
    c_ulong mask);

#define v_observerLock(_this) \
        c_mutexLock(&v_observer(_this)->mutex)

#define v_observerUnlock(_this) \
        c_mutexUnlock(&v_observer(_this)->mutex)

#define v_observerWaitCount(_this) \
        (v_observer(_this)->waitCount)

#undef OS_API

#endif
