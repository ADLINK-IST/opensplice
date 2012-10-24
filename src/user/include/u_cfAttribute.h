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
#ifndef U_CFATTRIBUTE_H
#define U_CFATTRIBUTE_H

#include "c_typebase.h"

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

C_CLASS(u_cfAttribute);

/**
 * \brief The <code>u_cfAttribute</code> cast method.
 *
 * This method casts an object to a <code>u_cfAttribute</code> object. Since user
 * layer objects are allocated on heap, no type checking is performed.
 */
#define u_cfAttribute(o) ((u_cfAttribute)(o))

/**
 * \brief The <code>u_cfAttribute</code> destructor.
 *
 * The destructor frees the proxy to the kernel configuration attribute.
 *
 * \param attribute the proxy to the kernel configuration attribute
 */
OS_API void
u_cfAttributeFree(
    u_cfAttribute attr);

/**
 * \brief Retrieves the attribute value as a string type.
 *
 * The attribute value is only stored in the last parameter, when it is
 * succesfully retrieved.
 *
 * \param attribute the proxy to the kernel configuration attribute
 * \param str the storage location of the attribute value
 *
 * \return TRUE, when the value is correctly retrieved as string type
 *         FALSE, otherwise
 */
OS_API c_bool
u_cfAttributeStringValue(
    u_cfAttribute attr,
    c_char **str);

/**
 * \brief Retrieves the attribute value as a boolean type.
 *
 * All whitespaces at the beginning of the value is skipped and consequently
 * it is checked whether the value contains "TRUE" or "FALSE" case insensitive.
 * The attribute value is only stored in the last parameter, when it is
 * succesfully retrieved.
 *
 * \param attribute the proxy to the kernel configuration attribute
 * \param b the storage location of the attribute value
 *
 * \return TRUE, when the value is correctly retrieved as boolean type
 *         FALSE, otherwise
 */
OS_API c_bool
u_cfAttributeBoolValue(
    u_cfAttribute attr,
    c_bool *b);

/**
 * \brief Retrieves the attribute value as a long type.
 *
 * The attribute value is only stored in the last parameter, when it is
 * succesfully retrieved.
 *
 * \param attribute the proxy to the kernel configuration attribute
 * \param lv the storage location of the attribute value
 *
 * \return TRUE, when the value is correctly retrieved as long type
 *         FALSE, otherwise
 */
OS_API c_bool
u_cfAttributeLongValue(
    u_cfAttribute attr,
    c_long *lv);

/**
 * \brief Retrieves the attribute value as an unsigned long type.
 *
 * The attribute value is only stored in the last parameter, when it is
 * succesfully retrieved.
 *
 * \param attribute the proxy to the kernel configuration attribute
 * \param ul the storage location of the attribute value
 *
 * \return TRUE, when the value is correctly retrieved as unsigned long type
 *         FALSE, otherwise
 */
OS_API c_bool
u_cfAttributeULongValue(
    u_cfAttribute attr,
    c_ulong *ul);

/**
 * \brief Retrieves the specified attribute value as unsigned long with as input
 * a numeric value with at the end a friendly name character (K,M,G).
 * For example 10M result in 10485760.
 *
 * The attribute value is only stored in the last parameter, when it is
 * succesfully retrieved.
 *
 * \param attribute the proxy to the kernel configuration attribute
 * \param size the storage location of the attribute value
 *
 * \return TRUE, when the value is correctly retrieved as unsigned long type
 *         FALSE, otherwise
 */
OS_API c_bool
u_cfAttributeSizeValue(
    u_cfAttribute attr,
    c_size *size);

/**
 * \brief Retrieves the attribute value as a floating point type.
 *
 * The attribute value is only stored in the last parameter, when it is
 * succesfully retrieved.
 *
 * \param attribute the proxy to the kernel configuration attribute
 * \param f the storage location of the attribute value
 *
 * \return TRUE, when the value is correctly retrieved as floating point type
 *         FALSE, otherwise
 */
OS_API c_bool
u_cfAttributeFloatValue(
    u_cfAttribute attr,
    c_float *f);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif /* U_CFATTRIBUTE_H */
