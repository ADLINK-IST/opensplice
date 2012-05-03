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
#ifndef U_CFDATA_H
#define U_CFDATA_H

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
OS_API c_bool
u_cfDataStringValue(
    u_cfData data,
    c_char **str);

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
OS_API c_bool
u_cfDataBoolValue(
    u_cfData data,
    c_bool *b);

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
OS_API c_bool
u_cfDataLongValue(
    u_cfData data,
    c_long *l);

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
OS_API c_bool
u_cfDataULongValue(
    u_cfData data,
    c_ulong *ul);

/**
 * \brief Retrieves the data as a unsigned long value.
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
OS_API c_bool
u_cfDataSizeValue(
    u_cfData data,
    c_size *size);

/**
 * \brief Retrieves a string which contains a friendly name (K,M,G) as a unsigned long value.
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
OS_API c_bool
u_cfDataSizeValueFromString(
    c_char *str,
    c_size *size);



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
OS_API c_bool
u_cfDataFloatValue(
    u_cfData data,
    c_float *f);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif /* U_CFDATA_H */
