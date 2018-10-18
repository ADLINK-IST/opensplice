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
/**@file api/cm/xml/include/cmx_publisher.h
 *
 * Represents a publisher in Splice in XML format.
 */
#ifndef CMX_PUBLISHER_H
#define CMX_PUBLISHER_H

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
 * Creates a new publisher. It is creates by creating its user layer
 * counterpart and serializing it into XML format.
 *
 * An XML publisher looks like:
   @verbatim
   <entity>
       <pointer>...</pointer>
       <handle_index>...</handle_index>
       <handle_serial>..</handle_serial>
       <name>...</name>
       <kind>PUBLISHER</kind>
       <enabled>...</enabled>
   </entity>
   @endverbatim
 * @param participant The XML representation of the participant to attach the
 *                    publisher to.
 * @param name The name for the publisher.
 * @param qos The qos for the publisher. If NULL is supplied, the default is
 *            taken.
 * @return The XML representation of the created publisher or NULL if it could
 *         not be created.
 */
OS_API c_char*     cmx_publisherNew        (const c_char* participant,
                                            const c_char* name,
                                            const c_char* qos);


OS_API const c_char* cmx_publisherCoherentBegin (const c_char* publisher);

OS_API const c_char* cmx_publisherCoherentEnd   (const c_char* publisher);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif /* CMX_PUBLISHER*/
