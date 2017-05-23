/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR PrismTech
 *   Limited, its affiliated companies and licensors. All rights reserved.
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
#ifndef U_CFDATA_H
#define U_CFDATA_H

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

C_CLASS(u_cfData);

/**
 * \brief The <code>u_cfData</code> cast method.
 *
 * This method casts an object to a <code>u_cfData</code> object. Since user
 * layer objects are allocated on heap, no type checking is performed.
 */
#define u_cfData(o) ((u_cfData)(o))

/**
 * \brief The <code>u_cfData</code> destructor.
 *
 * The destructor frees the proxy to the kernel configuration data.
 *
 * \param data the proxy to the kernel configuration data
 */
OS_API void
u_cfDataFree(
    u_cfData data);

/**
 * \brief Retrieves the data as a string.
 *
 * The data is only stored in the last parameter, when it is
 * succesfully retrieved.
 *
 * \param data the proxy to the kernel configuration data
 * \param str the storage location of the data
 *
 * \return TRUE, when the data is correctly retrieved as string
 *         FALSE, otherwise
 */
OS_API u_bool
u_cfDataStringValue(
    const u_cfData data,
    os_char **str);

/**
 * \brief Retrieves the data as a boolean value.
 *
 * All whitespaces at the beginning of the data is skipped and consequently
 * it is checked whether the data contains "TRUE" or "FALSE" case insensitive.
 * The data is only stored in the last parameter, when it is succesfully
 * retrieved.
 *
 * \param data the proxy to the kernel configuration data
 * \param b the storage location of the data
 *
 * \return TRUE, when the data is correctly retrieved as a boolean type
 *         FALSE, otherwise
 */
OS_API u_bool
u_cfDataBoolValue(
    const u_cfData data,
    u_bool *b);

/**
 * \brief Retrieves the data as a long value.
 *
 * The data is only stored in the last parameter, when it is
 * succesfully retrieved.
 *
 * \param data the proxy to the kernel configuration data
 * \param str the storage location of the data
 *
 * \return TRUE, when the data is correctly retrieved as long value
 *         FALSE, otherwise
 */
OS_API u_bool
u_cfDataLongValue(
    const u_cfData data,
    os_int32 *l);

/**
 * \brief Retrieves the data as a unsigned long value.
 *
 * The data is only stored in the last parameter, when it is
 * succesfully retrieved.
 *
 * \param data the proxy to the kernel configuration data
 * \param str the storage location of the data
 *
 * \return TRUE, when the data is correctly retrieved as unsigned long value
 *         FALSE, otherwise
 */
OS_API u_bool
u_cfDataULongValue(
    const u_cfData data,
    os_uint32 *ul);

/**
 * \brief Retrieves the data as an unsigned size value with optional unit specifier
 *
 * In case data contains a floating-point number, it is rounded to the nearest integer.
 * Unit specifiers are determined by unittab_bytes, some examples: 100B, 1k, 10MiB
 *
 * The data is only stored in the last parameter, when it is
 * succesfully retrieved.
 *
 * \param data the proxy to the kernel configuration data
 * \param size the storage location of the data
 *
 * \return TRUE, when the data is correctly retrieved
 *         FALSE, otherwise
 */
OS_API u_bool
u_cfDataSizeValue(
    const u_cfData data,
    u_size *size);

/**
 * \brief Retrieves the data as a unsigned 64-bit integer value with optional unit specifier
 *
 * In case data contains a floating-point number, it is rounded to the nearest integer.
 * Unit specifiers are determined by unittab_bytes, some examples: 100B, 1k, 10MiB
 *
 * The data is only stored in the last parameter, when it is
 * succesfully retrieved.
 *
 * \param data the proxy to the kernel configuration data
 * \param size the storage location of the data
 *
 * \return TRUE, when the data is correctly retrieved as unsigned long value
 *         FALSE, otherwise
 */
OS_API u_bool
u_cfDataUInt64Value(
    const u_cfData data,
    os_uint64 *size);

/**
 * \brief Retrieves a size value from a value string with optional unit specifier
 *
 * In case the string contains a floating-point number, it is rounded to the nearest integer.
 * Unit specifiers are determined by unittab_bytes, some examples: 100B, 1k, 10MiB
 *
 * The data is only stored in the last parameter, when it is
 * succesfully retrieved.
 *
 * \param str the string representation of the configuration data
 * \param size the storage location of the data
 *
 * \return TRUE, when the data is correctly retrieved as unsigned long value
 *         FALSE, otherwise
 */
OS_API u_bool
u_cfDataSizeValueFromString(
    const os_char *str,
    u_size *size);

/**
 * \brief Retrieves a unsigned 64-bit value from a string with optional unit specifier
 *
 * In case the string contains a floating-point number, it is rounded to the nearest integer.
 * Unit specifiers are determined by unittab_bytes, some examples: 100B, 1k, 10MiB
 *
 * The data is only stored in the last parameter, when it is
 * succesfully retrieved.
 *
 * \param str the string representation of the configuration data
 * \param size the storage location of the data
 *
 * \return TRUE, when the data is correctly retrieved as unsigned long value
 *         FALSE, otherwise
 */
OS_API u_bool
u_cfDataUInt64ValueFromString(
    const os_char *str,
    os_uint64 *size);

/**
 * \brief Retrieves the data as a floating point value.
 *
 * The data is only stored in the last parameter, when it is
 * succesfully retrieved.
 *
 * \param data the proxy to the kernel configuration data
 * \param str the storage location of the data
 *
 * \return TRUE, when the data is correctly retrieved as floating point value
 *         FALSE, otherwise
 */
OS_API u_bool
u_cfDataFloatValue(
    const u_cfData data,
    os_float *f);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif /* U_CFDATA_H */
