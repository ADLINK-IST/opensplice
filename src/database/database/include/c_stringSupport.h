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
#ifndef C_STRINGSUPPORT_H
#define C_STRINGSUPPORT_H

#include "c_typebase.h"
#include "c_iterator.h"
#include "os_if.h"

#ifdef OSPL_BUILD_DB
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif

#if defined (__cplusplus)
extern "C" {
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

OS_API c_bool      c_isOneOf        (c_char c, const c_char *symbolList);
OS_API c_bool      c_isDigit        (c_char c);
OS_API c_bool      c_isLetter       (c_char c);

OS_API c_char     *c_skipSpaces     (const c_char *str);
OS_API c_char     *c_skipIdentifier (const c_char *str, const c_char *punctuationList);
OS_API c_char     *c_skipUntil      (const c_char *str, const c_char *symbolList);
OS_API c_iter      c_splitString    (const c_char *str, const c_char *delimiters);
OS_API c_equality  c_compareString  (const c_char *s1, const c_char *s2);
/** \brief string trimmer
*
* Returns a newly allocated string with the same contents as the original string
* without the leading and trailing whitespace characters.
*
* Precondition:
*   s != NULL
* Postcondition:
*   None
*
* Possible results:
* - return trimmed s
* - if original string only contains spaces, return null-terminated empty string
*/
OS_API c_char     *c_trimString     (const c_char *s);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif

