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
#ifndef V_CFELEMENT_H
#define V_CFELEMENT_H

/**
 * \file kernel/include/v_cfElement.h
 * \brief This file defines the interface of the kernel configuration element.
 *
 * A configuration element has attributes and children. A child can be an
 * other element or a data element (<code>v_cfData</code>).
 */

#if defined (__cplusplus)
extern "C" {
#endif

/**
 * \brief The <code>v_cfElement</code> cast method.
 *
 * This method casts an object to a <code>v_cfElement</code> object.
 * Before the cast is performed, the type of the object is checked to
 * be <code>v_cfElement</code> or one of its subclasses.
 */
#define v_cfElement(o) (C_CAST(o,v_cfElement))

#include "kernelModule.h"
#include "v_kernel.h"
#include "os_if.h"

#ifdef OSPL_BUILD_KERNEL
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

/**
 * \brief The <code>v_cfElement</code> constructor.
 *
 * The tag of a configuration element is not unique, since it identifies 
 * the type of element.
 *
 * \param config the configuration it belongs to.
 * \param tagName the tag of the element
 * 
 * \return <code>NULL</code> if memory allocation fails, otherwise
 *         a reference to a newly instantiated configuration element.         
 */
OS_API v_cfElement
v_cfElementNew (
    v_configuration config,
    const c_char *tagName);

/**
 * \brief The initialisation method to initialise the attributes of this class.
 *
 * \param element a reference to the configuration element object
 * \param config the configuration it belongs to.
 * \param tagName the name tag of the element
 */
OS_API void
v_cfElementInit (
    v_cfElement element,
    v_configuration config,
    const c_char *tagName);

/**
 * \brief Adds an attribute to the configuration element.
 *
 * An attribute is uniquely identified by its name. If no attribute with the
 * specified name exists in this configuration element it is inserted and
 * returned to the user. Otherwise the existing attribute of the configuration
 * element is returned. So this method always returns the attribute with the
 * specified name given by the <code>attribute</code> parameter.
 *
 * \param element   the element object to operate on
 * \param attribute the attribute to add
 *
 * \return The attribute of this configuration element.
 */
OS_API v_cfAttribute
v_cfElementAddAttribute (
    v_cfElement element,
    v_cfAttribute attribute);

/**
 * \brief Adds a child node to the configuration element.
 *
 * A child is not uniquely identified by its name. Therefore every node is
 * added as a child. When the node is already a child of the configuration
 * element it is not added again. So the method is idempotent.
 *
 * \param element the element object to operate on
 * \param child   the child node to add
 *
 * \return the child added child element.
 */
OS_API v_cfNode
v_cfElementAddChild(
    v_cfElement element,
    v_cfNode child);

/**
 * \brief Returns an iterator containing all attributes of the configuration
 *        element.
 *
 * The reference count of the <code>v_cfAttribute</code> objects returned are
 * not increased, since configuration nodes can not be deleted from the kernel.
 * When the configuration element does not contain any attributes an empty
 * iterator is returned.
 *
 * \param element the element object to operate on
 *
 * \return a reference to an iterator containing <code>v_cfAttribute</code>
 *         nodes.
 */
OS_API c_iter
v_cfElementGetAttributes(
    v_cfElement element);

/**
 * \brief Returns an iterator containing all children of the configuration
 *        element.
 *
 * \param element the element object to operate on
 *
 * \return a reference to an iterator containing <code>v_cfData</code> and
 *         <code>v_cfElement</code> nodes.
 */
OS_API c_iter
v_cfElementGetChildren(
    v_cfElement element);

/**
 * \brief Returns the attribute with the given name.
 *
 * The method returns the attribute with the given name. When no attribute
 * with the given name exists, <code><code>NULL</code></code> is returned.
 *
 * \param element       the element object to operate on
 * \param attributeName the name of the attribute
 *
 * \return a reference to the attribute with given name or
 *         <code>NULL</code> if no such attribute exists.
 */
OS_API v_cfAttribute
v_cfElementAttribute(
    v_cfElement element,
    const c_char *attributeName);

/**
 * \brief Returns the value of the attribute identified by the given name.
 *
 * This method finds the attribute identified by the given name and returns
 * its value. When no attribute exists within the configuration element an
 * undefined value is returned (i.e. <code>value.kind == V_UNDEFINED</code>).
 * 
 * \param element       the element object to operate on
 * \param attributeName the name of the attribute, which value is requested
 *
 * \return the value of the attribute identified by the given name.
 */
OS_API c_value
v_cfElementAttributeValue(
    v_cfElement element,
    const c_char *attributeName);

/**
 * \brief Returns an iterator containing all configuration nodes selected by
 *        the XPATH expression.
 *
 * Only the following abbreviated syntax is supported:
 * expr ::= tagName | tagName '/' expr
 * tagName ::= the tag of an element
 *
 * Examples:
 * - <code>"Foo"</code>: all child elements of the given element with the tag
 *                       'Foo' are returned. 
 * - <code>"Foo/Bar"</code>: all 'Foo' child elements containing 'Bar'
 *                           children of the given element are returned.
 *
 * \param element   the element object to operate on
 * \param xpathExpr the XPATH expression
 *
 * \return a reference to an iterator containing configuration nodes that
 *         comply to the XPATH expression.
 */
OS_API c_iter
v_cfElementXPath(
    v_cfElement element,
    const c_char *xpathExpr);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif /* V_CFELEMENT_H */
