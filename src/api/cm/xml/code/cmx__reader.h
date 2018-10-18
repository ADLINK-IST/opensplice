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
void    cmx_readerDataTypeAction    (v_public entity,
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
