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
/**
 * @file api/cm/xml/code/cmx__dataReader.h
 * 
 * Offers internal routines on a dataReader.
 */
#ifndef CMX__DATAREADER_H
#define CMX__DATAREADER_H

#include "c_typebase.h"
#include "v_dataReader.h"

#if defined (__cplusplus)
extern "C" {
#endif
#include "cmx__dataReader.h"

/**
 * Initializes the dataReader specific part of the XML representation of the 
 * supplied kernel dataReader. This function should only be used by the 
 * cmx_entityNewFromWalk function.
 * 
 * @param entity The entity to create a XML representation of.
 * @return The dataReader specific part of the XML representation of the entity.
 */
c_char* cmx_dataReaderInit (v_dataReader entity);

#if defined (__cplusplus)
}
#endif

#endif /* CMX__DATAREADER_H */
