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
/**@file api/cm/xml/code/cmx__domain.h
 * 
 * Offers internal routines on a domain.
 */
#ifndef CMX__DOMAIN_H
#define CMX__DOMAIN_H

#include "c_typebase.h"
#include "v_partition.h"

#if defined (__cplusplus)
extern "C" {
#endif
#include "cmx_domain.h"

/**
 * Initializes the domain specific part of the XML representation of the 
 * supplied kernel domain. This function should only be used by the 
 * cmx_entityNewFromWalk function.
 * 
 * @param entity The entity to create a XML representation of.
 * @return The domain specific part of the XML representation of the entity.
 */
c_char* cmx_domainInit  (v_partition entity);

#if defined (__cplusplus)
}
#endif

#endif /* CMX__DOMAIN_H */
