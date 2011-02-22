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
/**@file api/cm/xml/include/cmx_query.h
 * 
 * Represents a query in Splice in XML format.
 */
#ifndef CMX_QUERY_H
#define CMX_QUERY_H

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
 * Creates a new query. It is creates by creating its user layer 
 * counterpart and serializing it into XML format.
 * 
 * An XML query looks like:
   @verbatim
   <entity>
       <pointer>...</pointer>
       <handle_index>...</handle_index>
       <handle_serial>..</handle_serial>
       <name>...</name>
       <kind>QUERY</kind>
       <enabled>...</enabled>
       <expression>...</expression>
   </entity>
   @endverbatim
 * @param reader The XML representation of the reader to attach the  query to.
 * @param name The name for the query.
 * @param expression The query expression.
 * @return The XML representation of the created publisher or NULL if it could
 *         not be created.
 */
OS_API c_char*     cmx_queryNew    (const c_char* reader,
                                    const c_char* name,
                                    const c_char* expression);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif /* CMX_QUERY*/
