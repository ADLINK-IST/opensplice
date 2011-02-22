/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2011 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE 
 *
 *   for full copyright notice and license terms. 
 *
 */
/**@file api/cm/xml/include/cmx_writer.h
 * Represents a writer in Splice in XML format.
 */
#ifndef CMX_WRITER_H
#define CMX_WRITER_H

#include "c_typebase.h"

#if defined (__cplusplus)
extern "C" {
#endif
#include "os_if.h"

#ifdef OSPL_BUILD_CMXML
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

/**
 * Creates a new writer. It is creates by creating its user layer 
 * counterpart and serializing it into XML format.
 * 
 * An XML writer looks like:
   @verbatim
   <entity>
       <pointer>...</pointer>
       <handle_index>...</handle_index>
       <handle_serial>..</handle_serial>
       <name>...</name>
       <kind>WRITER</kind>
       <enabled>...</enabled>
   </entity>
   @endverbatim
 * @param publisher The XML representation of the publisher to attach the 
 *                  writer to.
 * @param name The name for the writer.
 * @param topic The XML representation of the topic that the writer must write.
 * @param qos The qos for the writer. If NULL is supplied, the default is taken.
 * @return The XML representation of the created writer or NULL if it could
 *         not be created.
 */
OS_API c_char*         cmx_writerNew       (const c_char* publisher,
                                            const c_char* name,
                                            const c_char* topic,
                                            const c_char* qos);

/**
 * Resolves the data type of the data the supplied writer writes.
 * 
 * @param The XML representation of the writer, which data type to resolve.
 * @return The XML representation of the type the writer writes.
 */
OS_API c_char*         cmx_writerDataType  (const c_char* writer);

/**
 * Writes the supplied data using the supplied writer.
 * 
 * @param writer The XML representation of the writer to write with.
 * @param data The XML representation of the data to write.
 * @return Whether or not the write succeeded. When succeeded:
 * @verbatim<result>OK</result>@endverbatim is returned,
 * @verbatim<result>FAILED</result>@endverbatim otherwise.
 */
OS_API const c_char*   cmx_writerWrite     (const c_char* writer, 
                                            const c_char* data);

/**
 * Disposes the supplied data using the supplied writer.
 * 
 * @param writer The XML representation of the writer to write with.
 * @param data The XML representation of the data to dispose.
 * @return Whether or not the dispose succeeded. When succeeded:
 * @verbatim<result>OK</result>@endverbatim is returned,
 * @verbatim<result>FAILED</result>@endverbatim otherwise.
 */                              
OS_API const c_char*   cmx_writerDispose   (const c_char* writer, 
                                            const c_char* data);

/**
 * WriteDisposes the supplied data using the supplied writer.
 * 
 * @param writer The XML representation of the writer to write with.
 * @param data The XML representation of the data to writeDispose.
 * @return Whether or not the dispose succeeded. When succeeded:
 * @verbatim<result>OK</result>@endverbatim is returned,
 * @verbatim<result>FAILED</result>@endverbatim otherwise.
 */                              
OS_API const c_char*   cmx_writerWriteDispose   (const c_char* writer, 
                                                 const c_char* data);

/**
 * Registers the supplied data using the supplied writer.
 * 
 * @param writer The XML representation of the writer to write with.
 * @param data The XML representation of the data to register.
 * @return Whether or not the dispose succeeded. When succeeded:
 * @verbatim<result>OK</result>@endverbatim is returned,
 * @verbatim<result>FAILED</result>@endverbatim otherwise.
 */                              
OS_API const c_char*   cmx_writerRegister       (const c_char* writer, 
                                                 const c_char* data);

/**
 * Unregisters the supplied data using the supplied writer.
 * 
 * @param writer The XML representation of the writer to write with.
 * @param data The XML representation of the data to register.
 * @return Whether or not the dispose succeeded. When succeeded:
 * @verbatim<result>OK</result>@endverbatim is returned,
 * @verbatim<result>FAILED</result>@endverbatim otherwise.
 */                              
OS_API const c_char*   cmx_writerUnregister     (const c_char* writer, 
                                                 const c_char* data);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif /* CMX_WRITER*/
