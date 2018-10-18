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
#ifndef V_CFNODE_H
#define V_CFNODE_H

/**
 * \file kernel/include/v_cfNode.h
 * \brief This file defines the interface of the kernel configuration node.
 *
 * This is the base class of all configuration classes. Although it is an
 * abstract class, it does have a <code>new()</code> function for convenience.
 *
 * Every node has a name. Whether the name uniquely identifies a configuration
 * node depends on the subclass of the node. The following subclasses are
 * defined:
 * - <code>v_cfAttribute</code>: the name uniquely identifies an attribute.
 * - <code>v_cfData</code>: the name of data elements is constant "#text"
 * - <code>v_cfElement</code>: the name is the tag name of an element.
 *
 * 
 */

#if defined (__cplusplus)
extern "C" {
#endif

#include "kernelModuleI.h"
#include "v_kernel.h"
#include "os_if.h"

#ifdef OSPL_BUILD_CORE
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

/**
 * \brief The <code>v_cfNode</code> cast method.
 *
 * This method casts an object to a <code>v_cfNode</code> object.
 * Before the cast is performed, the type of the object is checked to
 * be <code>v_cfNode</code> or one of its subclasses.
 */
#define v_cfNode(o) (C_CAST(o,v_cfNode))

/**
 * \brief A convenience method to create an object of any descendant of
 *  <code>v_cfNode</code>.
 *
 * This method does not call <code>v_cfNodeInit()</code>!
 *
 * \param config the configuration it belongs to.
 * \param kind   a constant indicating, what subclass must be instantiated.
 *
 * \return NULL if memory allocation fails, otherwise
 *         a reference to the newly instantiated subclass.
 */
OS_API v_cfNode
v_cfNodeNew(
    v_configuration config,
    v_cfKind kind);

/**
 * \brief The initialisation method to initialise the attributes of this
 *        class.
 *
 * \param node   a reference to an instance of type <code>v_cfNode</code>
 * \param config the configuration it belongs to.
 * \param kind   constant identifying the real type of the instance
 * \param name   the name of the node.
 */
OS_API void
v_cfNodeInit (
    v_cfNode node,
    v_configuration config,
    v_cfKind kind,
    const c_char *name);

/**
 * \brief Returns the name of the node.
 *
 * \param node the node object to operate on
 *
 * \return a reference to the name of the node.
 */
OS_API const c_char *
v_cfNodeGetName (
    v_cfNode node);

/**
 * \brief Returns a constant that identifies the real type of the node.
 *
 * The following constants are defined:
 * - <code>V_CFATTRIBUTE</code>: type <code>v_cfAttribute</code>
 * - <code>V_CFDATA</code>: type <code>v_cfData</code>
 * - <code>V_CFELEMENT</code>: type <code>v_cfElement</code>
 *
 * \param node the node object to operate on.
 *
 * \return the constant identifying the real type of the node.
 */
OS_API v_cfKind
v_cfNodeKind(
    v_cfNode node);


OS_API v_configuration
v_cfNodeConfiguration(
    v_cfNode node);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif /* V_CFNODE_H */
