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
/**@file api/cm/xml/code/cmx__query.h
 * 
 * Offers internal routines on a query.
 */
#ifndef CMX__QUERY_H
#define CMX__QUERY_H

#include "c_typebase.h"
#include "v_query.h"

#if defined (__cplusplus)
extern "C" {
#endif
#include "cmx_query.h"

/**
 * Initializes the query specific part of the XML representation of the 
 * supplied kernel query. This function should only be used by the 
 * cmx_entityNewFromWalk function.
 * 
 * @param entity The entity to create a XML representation of.
 * @return The query specific part of the XML representation of the entity.
 */
c_char* cmx_queryInit (v_query entity);

#if defined (__cplusplus)
}
#endif

#endif /* CMX__QUERY_H */
