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
/**@file api/cm/xml/include/cmx_dataReader.h
 * 
 * Represents a dataReader in Splice in XML format.
 */
#ifndef CMX_DATAREADER_H
#define CMX_DATAREADER_H

#include "c_typebase.h"
#include "c_time.h"

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
 * Creates a new dataReader. It is creates by creating its user layer 
 * counterpart and serializing it into XML format.
 * 
 * An XML dataReader looks like:
   @verbatim
   <entity>
       <pointer>...</pointer>
       <handle_index>...</handle_index>
       <handle_serial>..</handle_serial>
       <name>...</name>
       <kind>DATAREADER</kind>
       <enabled>...</enabled>
   </entity>
   @endverbatim
 * @param subscriber The XML representation of a subscriber, where to
 *                   attach the dataReader to.
 * @param name The name for the dataReader.
 * @param view The view expression (OQL).
 * @param qos The QoS for the dataReader. If NULL, the default is taken.
 * @return The XML representation of the created dataReader or NULL if 
 *         it could not be created.
 */
OS_API c_char*         cmx_dataReaderNew                       (const c_char* subscriber,
                                                                const c_char* name,
                                                                const c_char* view,
                                                                const c_char* qos);

OS_API const c_char*   cmx_dataReaderWaitForHistoricalData     (const c_char* dataReader,
                                                                const c_time timeout);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif /* CMX_DATAREADER*/
