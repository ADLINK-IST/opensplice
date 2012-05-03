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
#ifndef U_CFELEMENT_H
#define U_CFELEMENT_H

#include "c_typebase.h"
#include "c_iterator.h"

#include "u_cfAttribute.h"

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

C_CLASS(u_cfElement);

/**
 * \brief The <code>u_cfElement</code> cast method.
 *
 * This method casts an object to a <code>u_cfElement</code> object. Since user
 * layer objects are allocated on heap, no type checking is performed.
 */
#define u_cfElement(o) ((u_cfElement)(o))

/**
 * \brief The <code>u_cfElement</code> destructor.
 *
 * The destructor frees the proxy to the kernel configuration element.
 *
 * \param element the proxy to the kernel configuration element
 */
OS_API void
u_cfElementFree(
    u_cfElement element);

/**
 * \brief Returns a collection of all children of this element.
 *
 * The children of a configuration element can be of type:
 * <code>V_CFELEMENT</code> and <code>V_CFDATA</code>. When the element is an
 * empty element an empty collection is returned.
 *
 * \param element the proxy to the kernel configuration element
 *
 * \return collection of all children of the given element.
 */
OS_API c_iter
u_cfElementGetChildren(
    u_cfElement element);

/**
 * \brief Returns a collection of all attributes of this element.
 *
 * The returned collection contains configuration nodes of type
 * <code>V_CFATTRIBUTE</code>. When the given element has no attributes, an
 * empty collection is returned.
 *
 * \param element the proxy to the kernel configuration element
 *
 * \return collection of all children of the given element.
 */
OS_API c_iter
u_cfElementGetAttributes(
    u_cfElement element);

/**
 * \brief Returns a proxy to the configuration attribute of the given element,
 *        identified by the given name.
 *
 * Every attribute within an element is uniquely identified by a name. This
 * method
 * returns a proxy to the real kernel configuration attribute of this element.
 *
 * \param element the proxy to the kernel configuration element
 * \param name the name of the attribute to find
 *
 * \return a proxy to the configuration attribute of the configuration element,
 *         or <code>NULL</code> when no attribute with the given name exists
 *         within the configuration element.
 */
OS_API u_cfAttribute
u_cfElementAttribute(
    u_cfElement element,
    const c_char *name);

/**
 * \brief Retrieves the specified attribute value as a string.
 *
 * The specified attribute value is only stored in the last parameter,
 * when the attribute exists and the value could be succesfully retrieved.
 *
 * \param element the proxy to the kernel configuration element
 * \param attributeName the name of the attribute to retrieve the value from
 * \param str the storage location of the attribute value
 *
 * \return TRUE, when the attribute exists and the value is correctly retrieved
 *               as string type
 *         FALSE, otherwise
 */
OS_API c_bool
u_cfElementAttributeStringValue(
    u_cfElement element,
    const c_char *attributeName,
    c_char **str);

/**
 * \brief Retrieves the specified attribute value as a boolean.
 *
 * All whitespaces at the beginning of the attribute value is skipped and
 * consequently it is checked whether the value contains "TRUE" or "FALSE"
 * case insensitive.
 * The specified attribute value is only stored in the last parameter,
 * when the attribute exists and the value could be succesfully retrieved.
 *
 * \param element the proxy to the kernel configuration element
 * \param attributeName the name of the attribute to retrieve the value from
 * \param b the storage location of the attribute value
 *
 * \return TRUE, when the attribute exists and the value is correctly retrieved
 *               as boolean
 *         FALSE, otherwise
 */
OS_API c_bool
u_cfElementAttributeBoolValue(
    u_cfElement element,
    const c_char *attributeName,
    c_bool *b);

/**
 * \brief Retrieves the specified attribute value as a long.
 *
 * The specified attribute value is only stored in the last parameter,
 * when the attribute exists and the value could be succesfully retrieved.
 *
 * \param element the proxy to the kernel configuration element
 * \param attributeName the name of the attribute to retrieve the value from
 * \param str the storage location of the attribute value
 *
 * \return TRUE, when the attribute exists and the value is correctly retrieved
 *               as long
 *         FALSE, otherwise
 */
OS_API c_bool
u_cfElementAttributeLongValue(
    u_cfElement element,
    const c_char *attributeName,
    c_long *l);

/**
 * \brief Retrieves the specified attribute value as unsigned long with as input
 * a numeric value with at the end a friendly name character (K,M,G).
 * For example 10M result in 10485760.
 *
 * The specified attribute value is only stored in the last parameter,
 * when the attribute exists and the value could be succesfully retrieved.
 *
 * \param element the proxy to the kernel configuration element
 * \param attributeName the name of the attribute to retrieve the value from
 * \param size the storage location of the attribute value
 *
 * \return TRUE, when the attribute exists and the value is correctly retrieved
 *               as unsigned long
 *         FALSE, otherwise
 */
OS_API c_bool
u_cfElementAttributeSizeValue(
    u_cfElement element,
    const c_char *attributeName,
    c_size *size);

/**
 * \brief Retrieves the specified attribute value as unsigned long.
 *
 * The specified attribute value is only stored in the last parameter,
 * when the attribute exists and the value could be succesfully retrieved.
 *
 * \param element the proxy to the kernel configuration element
 * \param attributeName the name of the attribute to retrieve the value from
 * \param str the storage location of the attribute value
 *
 * \return TRUE, when the attribute exists and the value is correctly retrieved
 *               as unsigned long
 *         FALSE, otherwise
 */
OS_API c_bool
u_cfElementAttributeULongValue(
    u_cfElement element,
    const c_char *attributeName,
    c_ulong *ul);

/**
 * \brief Retrieves the specified attribute value as floating point.
 *
 * The specified attribute value is only stored in the last parameter,
 * when the attribute exists and the value could be succesfully retrieved.
 *
 * \param element the proxy to the kernel configuration element
 * \param attributeName the name of the attribute to retrieve the value from
 * \param str the storage location of the attribute value
 *
 * \return TRUE, when the attribute exists and the value is correctly retrieved
 *               as floating point
 *         FALSE, otherwise
 */
OS_API c_bool
u_cfElementAttributeFloatValue(
    u_cfElement element,
    const c_char *attributeName,
    c_float *f);

/**
 * \brief Returns an collection containing proxies to configuration nodes selected by
 *        the XPATH expression.
 *
 * Only the following abbreviated syntax is supported:
 * expr ::= nodeName | nodeName '/' expr
 * nodeName ::= the name of a configuration node
 *
 * Examples:
 * - <code>"Foo"</code>: all child elements of the given element with the name
 *                       'Foo' are returned.
 * - <code>"Foo/Bar"</code>: all 'Foo' child elements containing 'Bar'
 *                           children of the given element are returned.
 * - <code>"#text"</code>: all data nodes of the given element.
 *
 * When no children of the element comply to the given XPATH expression, an
 * empty collection is returned.
 *
 * \param element the proxy to the kernel configuration element
 * \param xpathExpr the XPATH expression
 *
 * \return a collection containing proxies to configuration nodes that
 *         comply to the XPATH expression.
 */
OS_API c_iter
u_cfElementXPath(
    u_cfElement element,
    const c_char *xpathExpr);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif /* U_CFELEMENT_H */
