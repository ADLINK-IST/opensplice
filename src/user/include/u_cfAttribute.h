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
#ifndef U_CFATTRIBUTE_H
#define U_CFATTRIBUTE_H

#include "u_types.h"

#if defined (__cplusplus)
extern "C" {
#endif
#include "os_if.h"

#ifdef OSPL_BUILD_CORE
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
OS_API u_bool
u_cfAttributeStringValue(
    const u_cfAttribute attr,
    os_char **str);

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
OS_API u_bool
u_cfAttributeBoolValue(
    const u_cfAttribute attr,
    u_bool *b);

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
OS_API u_bool
u_cfAttributeLongValue(
    const u_cfAttribute attr,
    os_int32 *lv);

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
OS_API u_bool
u_cfAttributeULongValue(
    const u_cfAttribute attr,
    os_uint32 *ul);

/**
 * \brief Retrieves the attribute value as an unsigned char type.
 *
 * The attribute value is only stored in the last parameter, when it is
 * succesfully retrieved.
 *
 * \param attribute the proxy to the kernel configuration attribute
 * \param uc the storage location of the attribute value
 *
 * \return TRUE, when the value is correctly retrieved as unsigned char type
 *         FALSE, otherwise
 */
OS_API u_bool
u_cfAttributeOctetValue(
    const u_cfAttribute attr,
    os_uchar *uc);

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
OS_API u_bool
u_cfAttributeSizeValue(
    const u_cfAttribute attr,
    u_size *size);

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
OS_API u_bool
u_cfAttributeFloatValue(
    const u_cfAttribute attr,
    os_float *f);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif /* U_CFATTRIBUTE_H */
