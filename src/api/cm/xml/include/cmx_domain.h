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
/**@file api/cm/xml/include/cmx_domain.h
 * 
 * Represents a domain in Splice in XML format.
 */
#ifndef CMX_DOMAIN_H
#define CMX_DOMAIN_H

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
 * Creates a new domain. It is creates by creating its user layer 
 * counterpart and serializing it into XML format.
 * 
 * An XML domain looks like:
   @verbatim
   <entity>
       <pointer>...</pointer>
       <handle_index>...</handle_index>
       <handle_serial>..</handle_serial>
       <name>...</name>
       <kind>DOMAIN</kind>
       <enabled>...</enabled>
   </entity>
   @endverbatim
 * @param participant The XML representation of the participant to attach the 
 *                    domain to.
 * @param name The name for the domain.
 * @return The XML representation of the created domain or NULL if it could
 *         not be created.
 */
OS_API c_char*     cmx_domainNew        (const c_char* participant,
                                         const c_char* name);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif /* CMX_DOMAIN*/
