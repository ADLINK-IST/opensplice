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
 * @file api/cm/xml/code/cmx__writer.h
 * 
 * Offers internal routines on a writer.
 */
#ifndef CMX__WRITER_H
#define CMX__WRITER_H

#include "c_typebase.h"
#include "v_writer.h"

#if defined (__cplusplus)
extern "C" {
#endif
#include "cmx_writer.h"

/**
 * Initializes the writer specific part of the XML representation of the 
 * supplied kernel writer. This function should only be used by the 
 * cmx_entityNewFromWalk function.
 * 
 * @param entity The entity to create a XML representation of.
 * @return The writer specific part of the XML representation of the entity.
 */
c_char* cmx_writerInit              (v_writer entity);

/**
 * Entity action routine to resolve the data type of the writer.
 * 
 * @param entity The writer kernel entity.
 * @param args Must be of type struct cmx_writerTypeArg. This will be filled 
 *             with the XML representation of the data type during the execution 
 *             of this function.
 */
void    cmx_writerDataTypeAction    (v_entity entity,
                                     c_voidp args);

/**
 * Copy routine which copies the supplied XML data into a c_object and writes
 * it into the system.
 * 
 * @param entity The writer entity.
 * @param args Must be of type struct cmx_writerArg. It must contain the XML 
 *             data that needs to be written.
 */
void    cmx_writerCopy              (v_entity entity,
                                     c_voidp args);

/**
 * Copy routine which copies the supplied XML data into a c_object and disposes
 * it in the system.
 * 
 * @param entity The writer entity.
 * @param args Must be of type struct cmx_writerArg. It must contain the XML 
 *             data that needs to be disposed.
 */
void    cmx_writerDisposeCopy       (v_entity entity,
                                     c_voidp args);


/**
 * Copy routine which copies the supplied XML data into a c_object and writes
 * it into the system.
 * 
 * @param entity The writer entity.
 * @param args Must be of type struct cmx_writerArg. It must contain the XML 
 *             data that needs to be written.
 */
void    cmx_writerWriteDisposeCopy  (v_entity entity,
                                     c_voidp args);

/**
 * Copy routine which copies the supplied XML data into a c_object and writes
 * it into the system.
 * 
 * @param entity The writer entity.
 * @param args Must be of type struct cmx_writerArg. It must contain the XML 
 *             data that needs to be written.
 */
void    cmx_writerRegisterCopy      (v_entity entity,
                                     c_voidp args);

/**
 * Copy routine which copies the supplied XML data into a c_object and writes
 * it into the system.
 * 
 * @param entity The writer entity.
 * @param args Must be of type struct cmx_writerArg. It must contain the XML 
 *             data that needs to be written.
 */
void    cmx_writerUnregisterCopy    (v_entity entity,
                                     c_voidp args);
#if defined (__cplusplus)
}
#endif

#endif /* CMX__WRITER_H */
