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
#ifndef V_SERVICESTATE_H
#define V_SERVICESTATE_H

#include "kernelModule.h"

#if defined (__cplusplus)
extern "C" {
#endif
#include "os_if.h"

#ifdef OSPL_BUILD_KERNEL
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

/**
 * \brief The <code>v_serviceState</code> cast method.
 *
 * This method casts an object to a <code>v_serviceState</code> object.
 * Before the cast is performed, the type of the object is checked to
 * be <code>v_serviceState</code> or one of its subclasses.
 */
#define v_serviceState(o) (C_CAST(o,v_serviceState))

/**
 * \brief Changes the state kind attribute of the service state object.
 *
 * The state kind indicates the state of the service. The following
 * table defines the state transitions that are allowed. The row of
 * the table defines the current state and the column represents the
 * new state (i.e. the state to transfer to).
 * <p>
 * <table BORDER COLS=7 WIDTH="100%" NOSAVE >
 * <tr ALIGN=CENTER NOSAVE>
 *   <td></td>
 *   <td>STATE_NONE</td>
 *   <td>STATE_INITIALISING</td>
 *   <td>STATE_OPERATIONAL</td>
 *   <td>STATE_TERMINATING</td>
 *   <td>STATE_TERMINATED</td>
 *   <td>STATE_DIED</td>
 * </tr>
 * <tr ALIGN=CENTER NOSAVE>
 *   <td>STATE_NONE</td>
 *   <td align=center>x</td><td>x</td><td>-</td><td>-</td><td>-</td><td>-</td>
 * </tr>
 * <tr ALIGN=CENTER NOSAVE>
 *   <td>STATE_INITIALISING</td>
 *   <td>-</td><td>x</td><td>x</td><td>x</td><td>-</td><td>x</td>
 * </tr>
 * <tr ALIGN=CENTER NOSAVE>
 *   <td>STATE_OPERATIONAL</td>
 *   <td>-</td><td>-</td><td>x</td><td>x</td><td>-</td><td>x</td>
 * </tr>
 * <tr ALIGN=CENTER NOSAVE>
 *   <td>STATE_TERMINATING</td>
 *   <td>-</td><td>-</td><td>-</td><td>x</td><td>x</td><td>x</td>
 * </tr>
 * <tr ALIGN=CENTER NOSAVE>
 *   <td>STATE_TERMINATED</td>
 *   <td>-</td><td>-</td><td>-</td><td>-</td><td>x</td><td>-</td>
 * </tr>
 * <tr ALIGN=CENTER NOSAVE>
 *   <td>STATE_DIED</td>
 *   <td>-</td><td>x</td><td>-</td><td>-</td><td>-</td><td>x</td>
 * </tr>
 * </table>
 *
 *
 * \param serviceState   the service state object to operate on.
 * \param newState  the new state for this service.
 *
 * \return TRUE,  if the state change is allowed and succeeded.<br>
 *         FALSE, if the state change is not allowed.
 */
OS_API c_bool
v_serviceStateChangeState(
    v_serviceState serviceState,
    v_serviceStateKind newState);

/**
 * \brief Returns the state kind of the service state object.
 *
 * The following kind of states are defined: <code>STATE_NONE</code>,
 * <code>STATE_INITIALISING</code>, <code>STATE_OPERATIONAL</code>,
 * <code>STATE_TERMINATING</code>, <code>STATE_TERMINATED</code> and
 * <code>STATE_DIED</code>.
 *
 * \param serviceState      the service state object to operate on.
 *
 * \return the state kind of the service state object.
 */
OS_API v_serviceStateKind
v_serviceStateGetKind(
    v_serviceState serviceState);

/**
 * \brief Returns the name of the service this service state object
 *        belongs to.
 *
 * Each service is uniquely identified by a name and each service has
 * a state object. The state object outlives the service object and
 * therefore also contains a name to uniquely identify the service state
 * object.
 *
 * \param serviceState the service state object to operate on.
 *
 * \return a reference to the name of the service this service state object
 *         represents.
 */
OS_API const c_char *
v_serviceStateGetName(
    v_serviceState serviceState);

/**
 * \todo remove this function and replace it in the garbage collector by
 * a call to change the state.
 */
OS_API void
v_serviceStateNotify(
    v_serviceState serviceState,
    v_serviceStateKind kind);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif /* V_SERVICESTATE_H */
