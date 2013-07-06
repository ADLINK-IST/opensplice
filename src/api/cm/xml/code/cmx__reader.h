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
/**@file api/cm/xml/code/cmx__reader.h
 *
 * Offers internal routines on a reader.
 */
#ifndef CMX__READER_H
#define CMX__READER_H

#include "c_typebase.h"
#include "v_kernel.h"
#include "v_readerSample.h"

#if defined (__cplusplus)
extern "C" {
#endif
#include "cmx_reader.h"

/**
 * Initializes the reader specific part of the XML representation of the
 * supplied kernel reader. This function should only be used by the
 * cmx_entityNewFromWalk function.
 *
 * @param entity The entity to create a XML representation of.
 * @return The reader specific part of the XML representation of the entity.
 */
c_char* cmx_readerInit              (v_reader entity);

/**
 * Entity action routine to resolve the data type of the contents of the
 * database of the reader.
 *
 * @param entity The reader kernel entity.
 * @param args Must be of type struct cmx_readerArg. This will be filled with
 *             the XML representation of the data type during the execution of
 *             this function.
 */
void    cmx_readerDataTypeAction    (v_entity entity,
                                     c_voidp args);

/**
 * Copy routine that is used for copying data when reading or taking data.
 * The function constructs the XML representation of the supplied object.
 *
 * @param o The sample to copy.
 * @param args Must be of type struct cmx_readerArg. This will be filled with
 *             the XML representation of the sample during the execution of
 *             this function.
 * @return Always FALSE.
 */
v_actionResult  cmx_readerCopy      (c_object o,
                                     c_voidp args);

v_actionResult  cmx_readerReadCopy  (c_object o,
                                     c_voidp args);

#if defined (__cplusplus)
}
#endif

#endif /* CMX__READER_H */
