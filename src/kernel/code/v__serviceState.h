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

#ifndef V__SERVICESTATE_H
#define V__SERVICESTATE_H

#include "v_serviceState.h"

/**
 * \brief The <code>v_serviceState</code> constructor.
 *
 * The constructor will create a new <code>v_serviceState</code> object.
 * The <code>type</code> parameter can be used to create the instance of
 * a subclass of <code>v_serviceState</code>. This can be used by services
 * to store their own state information in the kernel. Only the attributes 
 * defined in the <code>v_serviceState</code> class are initialised. It is
 * up to the service to initialise all attributes of the subclass.
 * <p>
 * Note: The given <code>type</code> is at runtime checked by assertion and
 *       therefore not checked in the release.
 *
 * \param kernel       the kernel 
 * \param name         the name of the service.
 * \param extStateName The name of a subclass of <code>v_serviceState</code>
 * \param leasePeriod  the duration of the validity of this service state.
 *
 * \return NULL if memory allocation failed, otherwise 
 *         a reference to a newly created 
 *         <code>v_serviceState</code> object.
 */
v_serviceState
v_serviceStateNew(
    v_kernel kernel,
    const c_char *name,
    const c_char *extStateName);

/**
 * \brief The initialisation function, which can be used by classes that 
 *        extend from this class.
 *
 * \param _this the service state object to operate on.
 * \param name the name of the service.
 * \param leasePeriod the duration of the validity of this service state.
 */
void
v_serviceStateInit(
    v_serviceState _this,
    const c_char *name);

/**
 * \brief The <code>v_serviceState</code> destructor.
 *
 * The object is really freed, if no more references to this object exist.
 * The function <code>v_serviceStateDeinit</code> is called to free all used
 * resources by this object.
 *
 * \param _this the service state object to operate on.
 */
void
v_serviceStateFree (
    v_serviceState _this);

/**
 * \brief The de-initialisation function, that frees all used resources 
 *        by this object.
 *
 * This function is called by <code>v_serviceStateFree</code>, but can 
 * also be called by destructor's of subclasses. All used resources by this
 * object are freed.
 *
 * \param _this the service state object to operate on.
 */
void
v_serviceStateDeinit (
    v_serviceState _this);

#endif
