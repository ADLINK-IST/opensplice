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

#ifndef V__OBSERVABLE_H
#define V__OBSERVABLE_H

#if defined (__cplusplus)
extern "C" {
#endif

#include "v_observable.h"

void
v_observableFree (
    v_observable _this);

/**
 * The initialisation of an observable object.
 * This method initialises all attributes of the observable class and must
 * be called by every derived class.
 *
 * \param _this the reference to an observable object.
 * \param name  the name of the observable.
 */
void
v_observableInit (
    v_observable _this,
    const c_char *name,
    v_statistics s,
    c_bool enable);

/**
 * The de-initialisation of an observable object.
 * This method releases all used resources by the observable object and must
 * be called by every derived class.
 *
 * \param _this the reference to an observable object.
 */
void
v_observableDeinit (
    v_observable _this);

#define v_observableCount(_this) \
        c_setCount(v_observable(_this)->observers)

#if defined (__cplusplus)
}
#endif

#endif /* V__OBSERVABLE_H */
