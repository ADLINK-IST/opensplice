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
/**@file api/cm/xml/code/cmx__serviceState.h
 * 
 * Offers internal routines on a serviceState.
 */
#ifndef CMX__SERVICESTATE_H
#define CMX__SERVICESTATE_H

#include "c_typebase.h"
#include "v_service.h"

#if defined (__cplusplus)
extern "C" {
#endif

/**
 * Initializes the serviceState specific part of the XML representation of the 
 * supplied kernel serviceState. This function should only be used by the 
 * cmx_entityNewFromWalk function.
 * 
 * @param entity The entity to create a XML representation of.
 * @return The serviceState specific part of the XML representation of the 
 *         entity.
 */
c_char*             cmx_serviceStateInit            (v_serviceState entity);

/**
 * Constructs the string representation of the supplied serviceStateKind.
 * 
 * @param stateKind The kernel state kind.
 * @return The string representation of the supplied kind.
 */
const c_char*       cmx_serviceStateKindToString    (v_serviceStateKind stateKind);

/**
 * Constructs the kernel stateKind representation of the supplied string.
 * 
 * @param stateKind The string representation of the kind.
 * @return The matching stateKind.
 */
v_serviceStateKind  cmx_serviceStateKindFromString  (const c_char* stateKind);

#if defined (__cplusplus)
}
#endif

#endif /* CMX__SERVICESTATE_H */
