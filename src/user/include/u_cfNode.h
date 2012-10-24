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
#ifndef U_CFNODE_H
#define U_CFNODE_H

#include "c_typebase.h"
#include "u_types.h"

#if defined (__cplusplus)
extern "C" {
#endif
#include "os_if.h"

#ifdef OSPL_BUILD_USER
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

C_CLASS(u_cfNode);

/**
 * \brief The <code>u_cfNode</code> cast method.
 *
 * This method casts an object to a <code>u_cfNode</code> object. Since user 
 * layer objects are allocated on heap, no type checking is performed.
 */
#define u_cfNode(o) ((u_cfNode)(o))

/**
 * \brief Returns the kind of configuration node.
 *
 * Since this is the base class of all configuration nodes, it contains 
 * an identification of the kind of node. The following nodes are known:
 * - <code>V_CFATTRIBUTE</code>: the attribute node, i.e. a name/value pair
 * - <code>V_CFELEMENT</code>: the element node, which has attributes and 
 *                             contains children (elements and/or data nodes)
 * - <code>V_CFDATA</code>: the data node, i.e. a value
 *
 * \param node The proxy to the kernel configuration node.
 *
 * \return the kind of configuration node.
 */
OS_API v_cfKind
u_cfNodeKind(
    u_cfNode node);

/**
 * \brief Returns the name of the configuration node.
 *
 * Every node has a name. Depending on the context the name might be unique. The
 * returned name is a copy of the configuration node name. The caller must free it
 * with <code>os_free</code>.
 *
 * \param node The proxy to the kernel configuration node.
 *
 * \return the name of the configuration node.
 */
OS_API c_char *
u_cfNodeName(
    u_cfNode node);

OS_API void
u_cfNodeFree (
    u_cfNode node);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif /* U_CFNODE_H */
