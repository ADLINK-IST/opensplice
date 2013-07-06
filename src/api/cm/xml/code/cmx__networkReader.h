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
/**@file api/cm/xml/code/cmx__networkReader.h
 * 
 * Offers internal routines on a dataReader.
 */
#ifndef CMX__NETWORKREADER_H
#define CMX__NETWORKREADER_H

#include "c_typebase.h"
#include "v_kernel.h"

#if defined (__cplusplus)
extern "C" {
#endif

/**
 * Initializes the networkReader specific part of the XML representation of the 
 * supplied kernel networkReader. This function should only be used by the 
 * cmx_entityNewFromWalk function.
 * 
 * @param entity The entity to create a XML representation of.
 * @return The networkReader specific part of the XML representation of the entity.
 */
c_char* cmx_networkReaderInit   (v_networkReader entity);

#if defined (__cplusplus)
}
#endif

#endif /* CMX__NETWORKREADER_H */
