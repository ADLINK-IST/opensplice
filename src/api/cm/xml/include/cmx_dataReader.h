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
                                                                const os_duration timeout);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif /* CMX_DATAREADER*/
