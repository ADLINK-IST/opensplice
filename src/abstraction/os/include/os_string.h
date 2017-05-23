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

/** \file os_string.h
 *  \brief string utility functions
 */

#ifndef OS_STRING_H
#define OS_STRING_H

#include "os_defs.h"

#if defined (__cplusplus)
extern "C" {
#endif

#ifdef OSPL_BUILD_CORE
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif

/**
 * \brief Find first occurrence of character in null terminated string
 *
 * @param str String to search for given characters
 * @param chrs Characters to search for in string
 * @param inc OS_TRUE to find first character included in given characters,
 *            OS_FALSE to find first character not included.
 * @return Pointer to first occurrence of character in string, or NULL
 */
OS_API os_char *
os_strchrs (
    const os_char *str,
    const os_char *chrs,
    os_boolean inc);

/**
 * \brief Find last occurrence of character in null terminated string
 *
 * @param str String to search for given characters
 * @param chrs Characters to search for in string
 * @param inc OS_TRUE to find last character included in given characters,
 *            OS_FALSE to find last character not included.
 * @return Pointer to last occurrence of character in string, or NULL
 */
OS_API os_char *
os_strrchrs (
    const os_char *str,
    const os_char *chrs,
    os_boolean inc);

/**
 * \brief Replace substring of null terminated string
 *
 * @param str Pointer to string
 * @param srch Character sequence to search for and replace
 * @param subst String to substitute character sequence with, or NULL to erase
 * @param max Maximum number of times to replace search, or 0 for unlimited
 * @return Pointer to newly allocated string with max occurrences of srch
 *         replaced, str if nothing changed, or NULL on failure
 */
OS_API os_char *
os_str_replace (
    const os_char *str,
    const os_char *srch,
    const os_char *subst,
    os_int max);

/**
 * \brief Replace substring of null terminated string if delimited by one or
 *        more characters specified as boundary characters
 *
 * @param str Pointer to subject
 * @param delim Character considered to be word boundary, or null to use
 *              whitespace.
 * @param word Word that must be substituted
 * @param subst String to substitute word with, or NULL to erase
 * @param max Maximum number of times to replace word, or 0 for unlimited
 * @return Pointer to newly allocated string with all occurrences of word
 *         replaced, str if nothing changed, or NULL on failure
 */
OS_API os_char *
os_str_word_replace (
    const os_char *str,
    const os_char *delim,
    const os_char *word,
    const os_char *subst,
    os_int max);

/**
 * \brief Strip specified characters from beginning of string
 *
 * @param str Null terminated string to trim
 * @param trim Null terminated string of characters to trim, or NULL to trim
 *             whitespace
 * @return Pointer to newly allocated trimmed string, str if nothing changed,
 *         or NULL on failure
 */
OS_API os_char *
os_str_ltrim (
    const os_char *str,
    const os_char *trim);

/**
 * \brief Strip specified characters from end of string
 *
 * @param str Null terminated string to trim
 * @param trim Null terminated string of characters to trim, or NULL to trim
 *             whitespace
 * @return Pointer to newly allocated trimmed string, str if nothing changed,
 *         or NULL on failure
 */
OS_API os_char *
os_str_rtrim (
    const os_char *str,
    const os_char *trim);

/**
 * \brief Strip specified characters from beginning and end of string
 *
 * @param str Null terminated string to trim
 * @param trim Null terminated string of characters to trim, or NULL to trim
 *             whitespace
 * @return Pointer to newly allocated trimmed string, str if no changes
 *         were made, or NULL on memory allocation failure
 */
OS_API os_char *
os_str_trim (
    const os_char *str,
    const os_char *trim);

/**
 * \brief Duplicate at most a specified number of bytes from a string
 *
 * @assert str != NULL
 * @assert len > 0
 * @param str pointer to string to use as source, with either at least len
 *            allocated or '\0'-terminated if memory pointed to is smaller than
 *            len.
 * @return Pointer to newly allocated string, with
 *         strlen(result) == min(strlen(str), len).
 */
OS_API char *
os_strndup (
    const char *str,
    os_size_t len) __nonnull_all__
                   __attribute_malloc__
                   __attribute_returns_nonnull__
                   __attribute_warn_unused_result__;

/**
 * \brief Get integer value for digit
 *
 * @param chr Character to interpret as digit
 * @return Integer value of digit or -1 on failure
 */
OS_API os_int
os_todigit (
    const os_int chr);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif /* OS_STRING_H */
