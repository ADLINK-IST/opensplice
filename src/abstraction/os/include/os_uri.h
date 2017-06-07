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
#ifndef OS_URI_H
#define OS_URI_H

#include "os_defs.h"
#include "os_classbase.h"

#if defined (__cplusplus)
extern "C" {
#endif

#ifdef OSPL_BUILD_CORE
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif

OS_CLASS(os_uri);

/* os_uriStrict can be used as a precondition if desired */
typedef enum {
    OS_URI_STRICT_TRUE, /* default */
    OS_URI_STRICT_FALSE,
    OS_URI_STRICT_AUTO
} os_uriStrict;

/* os_uriRelative can be used as a precondition if desired */
typedef enum {
    OS_URI_RELATIVE_AUTO, /* default */
    OS_URI_RELATIVE_TRUE,
    OS_URI_RELATIVE_FALSE
} os_uriRelative;

/**
 * \brief Allocate and initialize an os_uri object
 *
 * @return os_uri object or NULL on error
 */
OS_API os_uri
os_uriNew (void);

/**
 * \brief Free os_uri object and associated resources
 *
 * @param uri os_uri object
 * @return void
 */
OS_API void
os_uriFree (
    os_uri uri);

/**
 * \brief Try to read URI from input string and populate os_uri object
 *
 * @param uri os_uri object to populate
 * @param str String to read URI from
 * @return Pointer to first character that does not belong to the URI, the
 *         original value of str if the URI was invalid, or NULL on memory
 *         allocation error.
 */
OS_API os_char *
os_uriParse (
    os_uri uri,
    const os_char *str);

/**
 * \brief Write string representation of os_uri object to specified buffer
 *
 * @param str Pointer to buffer where string representation is to be stored
 * @param len Size of buffer referred to by str
 * @param uri os_uri object
 * @return The number characters that would be written to str had len been
 *         sufficiently large excluding the terminating null byte. If an output
 *         error was encountered, this function shall return a negative value.
 */
OS_API os_ssize_t
os_uriPrint (
    os_char *str,
    os_size_t len,
    const os_uri uri);

/**
 * \brief Return whether strict mode is enabled or disabled
 *
 * Strict mode is a workaround for backwards compatibility issues. The file://
 * prefix (and path without scheme) is special, because the user is allowed to
 * supply a Microsoft Windows path unencoded, i.e. it may contain spaces and
 * backslash characters.
 *
 * @param uri os_uri object
 * @return os_uriStrict
 */
OS_API os_uriStrict
os_uriGetStrict (
    const os_uri uri);

/**
 * \brief Enable or disable strict mode
 *
 * @param uri
 * @param strict OS_URI_STRICT_TRUE to enable, OS_URI_STRICT_FALSE to disable
 *        strict mode, or OS_URI_STRICT_AUTO to guess.
 * @return OS_RETCODE_OK on success
 *         OS_RETCODE_BAD_PARAMETER on OS_URI_STRICT_TRUE and non-strict path
 * @see os_uriGetStrict for a detailed explanation of strict mode
 */
OS_API os_int
os_uriSetStrict (
    os_uri uri,
    os_uriStrict strict);

/**
 * \brief Return if current uri is considered strict or not
 *
 * @param uri os_uri object
 * @return OS_TRUE if path is strict or OS_FALSE if it is not
 */
OS_API os_boolean
os_uriIsStrict (
    const os_uri uri);

/**
 * \brief Returns whether the uri is relative or not
 *
 * @param uri os_uri object
 * @return os_uriRelative
 */
OS_API os_uriRelative
os_uriGetRelative (
    const os_uri uri);

/**
 * \brief Enable or disable relative mode
 *
 * Relative mode is automatic by default and set to true or false after the
 * URI has been read from the input string. The caller can preset relative mode
 * to alter scanning mode.
 *
 * @param uri
 * @param relative
 * @return OS_RETCODE_OK on success
 *         OS_RETCODE_BAD_PARAMETER on OS_URI_RELATIVE_TRUE and scheme is set
 */
OS_API os_int
os_uriSetRelative (
    os_uri uri,
    os_uriRelative relative);

/**
 * \brief Return whether current uri is considered relative or not
 *
 * @param uri os_uri object
 * @return OS_FALSE if uri has scheme, or OS_TRUE if it does not
 */
OS_API os_boolean
os_uriIsRelative (
    const os_uri uri);

/**
 * \brief Return scheme subcomponent
 *
 * @param uri os_uri object
 * @return os_char pointer owned by os_uri object or NULL
 */
OS_API const os_char *
os_uriGetScheme (
    const os_uri uri);

/**
 * \brief Set scheme subcomponent
 *
 * @param uri os_uri object
 * @param scheme String containing scheme subcomponent, or NULL to discard
 * @return OS_RETCODE_OK on success
 *         OS_RETCODE_OUT_OF_RESOURCES on memory allocation failure
 *         OS_RETCODE_BAD_PARAMETER on invalid scheme or subcomponent collision
 */
OS_API os_int
os_uriSetScheme (
    os_uri uri,
    const os_char *scheme);

/**
 * \brief Return userinfo subcomponent
 *
 * The userinfo subcomponent may consist of a user name and, optionally,
 * scheme-specific information about how to gain authorization to access
 * the resource.
 *
 * @param uri os_uri object
 * @return Pointer to string owned by os_uri object or NULL if unavailable
 */
OS_API const os_char *
os_uriGetUserinfo (
    const os_uri uri);

/**
 * \brief Set userinfo subcomponent
 *
 * @param uri os_uri object
 * @param userinfo Pointer to valid userinfo, or NULL to discard
 * @return OS_RETCODE_OK on success
 *         OS_RETCODE_OUT_OF_RESOURCES on memory allocation failure
 *         OS_RETCODE_BAD_PARAMETER on invalid argument or subcomponent clash
 */
OS_API os_int
os_uriSetUserinfo (
    os_uri uri,
    const os_char *userinfo);

/**
 * \brief Returns host subcomponent
 *
 * @param uri os_uri object
 * @return Pointer to string owned by os_uri object or NULL if unavailable
 */
OS_API const os_char *
os_uriGetHost (
    const os_uri uri);

/**
 * \brief Set host subcomponent
 *
 * @param uri os_uri object
 * @param host Pointer to valid host, or NULL to discard
 * @return OS_RETCODE_OK on success
 *         OS_RETCODE_OUT_OF_RESOURCES on memory allocation failure
 *         OS_RETCODE_BAD_PARAMETER on invalid host or subcomponent clash
 */
OS_API os_int
os_uriSetHost (
    os_uri uri,
    const os_char *host);

/**
 * \brief Returns port subcomponent
 *
 * @param uri os_uri object
 * @return os_int value between 1 - 65535, or 0 if member is unavailable
 */
OS_API const os_char *
os_uriGetPort (
    const os_uri uri);

/**
 * \brief Set port subcomponent
 *
 * @param os_uri uri
 * @param port os_char value between 1 - 65535, or 0 to discard port number
 * @return OS_RETCODE_OK on success
 *         OS_RETCODE_BAD_PARAMETER on invalid port or subcomponent clash
 */
OS_API os_int
os_uriSetPort (
    os_uri uri,
    const os_char *port);

/**
 * \brief Get path subcomponent
 *
 * @param uri os_uri object
 * @return Pointer to string owned by os_uri object or NULL if unavailable
 * @see os_uriSetStrict for a detailed explanation of strict mode
 */
OS_API const os_char *
os_uriGetPath (
    const os_uri uri);

/**
 * \brief Set path subcomponent
 *
 * @param uri os_uri object
 * @param path Pointer to valid path, or NULL to discard.
 * @return OS_RETCODE_OK on success
 *         OS_RETCODE_OUT_OF_RESOURCES on memory allocation failure
 *         OS_RETCODE_BAD_PARAMETER on invalid path or subcomponent clash
 */
OS_API os_int
os_uriSetPath (
    os_uri uri,
    const os_char *path);

/**
 * \brief Get query subcomponent
 *
 * @param uri os_uri object
 * @return Pointer to string owned by os_uri object or NULL if unavailable
 */
OS_API const os_char *
os_uriGetQuery (
    const os_uri uri);

/**
 * \brief Set query subcomponent
 *
 * @param uri os_uri object
 * @param query Pointer to valid query, or NULL to discard
 * @return OS_RETCODE_OK on success
 *         OS_RETCODE_OUT_OF_RESOURCES on memory allocation failure
 *         OS_RETCODE_BAD_PARAMETER on invalid query
 */
OS_API os_int
os_uriSetQuery (
    os_uri uri,
    const os_char *query);

/**
 * \brief Get fragment subcomponent
 *
 * @param uri os_uri object
 * @return Pointer to string owned by os_uri object or NULL if unavailable
 */
OS_API const os_char *
os_uriGetFragment (
    const os_uri uri);

/**
 * \brief Set fragment subcomponent
 *
 * @param uri os_uri object
 * @param fragment Pointer to valid fragment, or NULL to discard
 * @return OS_RETCODE_OK on success
 *         OS_RETCODE_OUT_OF_RESOURCES on memory allocation failure
 *         OS_RETCODE_BAD_PARAMETER on invalid fragment
 */
OS_API os_int
os_uriSetFragment (
    os_uri uri,
    const os_char *str);

/**
 * \brief Read username from userinfo subcomponent of os_uri object
 *
 * The userinfo subcomponent is free-form, but is often formatted as
 * "user:password". This convenience function returns everything before
 * the first colon. If there's no colon in the userinfo subcomponent, it
 * returns the same information as os_uriGetUserinfo.
 *
 * @param uri os_uri object
 * @param value Pointer to object where the location of the newly allocated
 *              string is stored.
 * @return OS_RETCODE_OK on success
 *         OS_RETCODE_OUT_OF_RESOURCES on memory allocation failure
 *         OS_RETCODE_BAD_PARAMETER on invalid or non-existent key
 */
OS_API os_int
os_uriGetUser (
    const os_uri uri,
    os_char **user);

/**
 * \brief Read password from userinfo subcomponent of os_uri object
 *
 * This convenience function returns everything after the first colon.
 * OS_RETCODE_BAD_PARAMETER is returned if the userinfo subcomponent does not
 * contain a colon.
 *
 * Use of the format "user:password" is deprecated. Security sensitive
 * information should not be passed over the network unencrypted. Use of this
 * format is allowed locally, e.g. in configuration files, to specify a user
 * password combination for a given data source.
 *
 * @param uri Populated os_uri object
 * @param password Pointer to object where the location of the newly allocated
 *                 string is stored.
 * @return OS_RETCODE_OK on success
 *         OS_RETCODE_OUT_OF_RESOURCES on memory allocation failure
 *         OS_RETCODE_BAD_PARAMETER on invalid or non-existent key
 */
OS_API os_int
os_uriGetPassword (
    const os_uri uri,
    os_char **password);

/**
 * \brief Read value for from query subcomponent of os_uri object
 *
 * The query subcomponent of a os_uri object often contains key-value pairs in
 * the format <key>=<value> delimited by an ampersand character. This function
 * provides an easy way to retrieve the value for a specified key from queries
 * in such a format.
 *
 * @param uri Populated os_uri object
 * @param key Null-terminated string containing the key to lookup
 * @param value Pointer to object where the location of the newly allocated
 *              string is stored.
 * @return OS_RETCODE_OK on success
 *         OS_RETCODE_OUT_OF_RESOURCES on memory allocation failure
 *         OS_RETCODE_BAD_PARAMETER on invalid or non-existent key
 */
OS_API os_int
os_uriGetQueryField (
    const os_uri uri,
    const os_char *field,
    os_char **value);

#if 0
/* os_uriEncode is awaiting implementation */
OS_API os_result
os_uriEncode (
    os_char **pct_encoded,
    const os_char *plain);

/* os_uriDecode is awaiting implementation */
OS_API os_result
os_uriDecode (
    os_char **plain,
    const os_char *pct_encoded);
#endif

/**
 * \brief Scan subject sequence for hostname
 *
 * @param str Subject sequence to scan for hostname
 * @return Pointer to the final string, or the value of str if the subject
 *         sequence is empty or does not have the expected form.
 */
OS_API os_char *
os_scanRegName (
    const os_char *str);

/**
 * \brief Try to read IPv4 address from string
 *
 * @param str Subject sequence to scan for address
 * @return Pointer to the final string, or the value of str if the subject
 *         sequence is empty or does not have the expected form.
 */
OS_API os_char *
os_scanIPv4Address (
    const os_char *str);

/**
 * \brief Scan subject sequence for IPv6 address
 *
 * @param str Subject sequence to scan for address
 * @return Pointer to the final string, or the value of str if the subject
 *         sequence is empty or does not have the expected form.
 */
OS_API os_char *
os_scanIPv6Address (
    const os_char *str);

/**
 * \brief Scan subject sequence for IPv6 address, IPv4 address or hostname
 *
 * @param str Subject sequence to scan for address or hostname
 * @return Pointer to the final string, or the value of str if the subject
 *         sequence is empty or does not have the expected form.
 */
OS_API os_char *
os_scanHost (
    const os_char *str);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif /* OS_URI_H */
