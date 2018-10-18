/*
 *                         Vortex OpenSplice
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
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
void    cmx_writerDataTypeAction    (v_public entity,
                                     c_voidp args);

/**
 * Copy routine which copies the supplied XML data into a c_object and writes
 * it into the system.
 *
 * @param entity The writer entity.
 * @param args Must be of type struct cmx_writerArg. It must contain the XML
 *             data that needs to be written.
 */
void    cmx_writerCopy              (v_public entity,
                                     c_voidp args);

/**
 * Copy routine which copies the supplied XML data into a c_object and disposes
 * it in the system.
 *
 * @param entity The writer entity.
 * @param args Must be of type struct cmx_writerArg. It must contain the XML
 *             data that needs to be disposed.
 */
void    cmx_writerDisposeCopy       (v_public entity,
                                     c_voidp args);


/**
 * Copy routine which copies the supplied XML data into a c_object and writes
 * it into the system.
 *
 * @param entity The writer entity.
 * @param args Must be of type struct cmx_writerArg. It must contain the XML
 *             data that needs to be written.
 */
void    cmx_writerWriteDisposeCopy  (v_public entity,
                                     c_voidp args);

/**
 * Copy routine which copies the supplied XML data into a c_object and writes
 * it into the system.
 *
 * @param entity The writer entity.
 * @param args Must be of type struct cmx_writerArg. It must contain the XML
 *             data that needs to be written.
 */
void    cmx_writerRegisterCopy      (v_public entity,
                                     c_voidp args);

/**
 * Copy routine which copies the supplied XML data into a c_object and writes
 * it into the system.
 *
 * @param entity The writer entity.
 * @param args Must be of type struct cmx_writerArg. It must contain the XML
 *             data that needs to be written.
 */
void    cmx_writerUnregisterCopy    (v_public entity,
                                     c_voidp args);
#if defined (__cplusplus)
}
#endif

#endif /* CMX__WRITER_H */
