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

/** \file os/common/code/os_stdlib_strtod.c
 *  \brief Double/Float to/from string conversions
 *
 * OSPL uses '.' as decimal point.
 * Most stdlib like functions are locale dependent.
 * This means that they can't be simply used when the
 * LC_NUMERIC part of the locale is set to ',' like
 * the locales nl_NL and fr_FR.
 * This would clash with OSPL using '.'.
 *
 * The functions in this file provide locale
 * independent conversions.
 */

#include "os_report.h"

/*
 * Determine the maximum size that a string should have to be
 * able to contain a double.
 * See the following site for the calculation explanation:
 * http://stackoverflow.com/questions/1701055/what-is-the-maximum-length-in-chars-needed-to-represent-any-double-value
 */
#include <float.h>
#define DOUBLE_STRING_MAX_LENGTH (3 + DBL_MANT_DIG - DBL_MIN_EXP)

/*
 * VALID_DOUBLE_CHAR(c) is used to determine if the given char
 * can be valid when it appears in a string that should represent
 * a double.
 * It is used to detect the end of a double string representation.
 * Because it doesn't consider context, it is possible that more
 * characters are detected after the double (fi. when a few white
 * spaces tail the double). This isn't that bad, because the call
 * to strtod itself will handle these extra characters properly.
 */
#define VALID_DOUBLE_CHAR(c) ( (isspace(c)            ) || /* (leading) whitespaces   */ \
                               (isxdigit(c)           ) || /* (hexa)decimal digits    */ \
                               (c == '.'              ) || /* ospl LC_NUMERIC         */ \
                               (c == os_lcNumericGet()) || /* locale LC_NUMERIC       */ \
                               (c == '+') || (c == '-') || /* signs                   */ \
                               (c == 'x') || (c == 'X') || /* hexadecimal indication  */ \
                               (c == 'e') || (c == 'E') || /* exponent chars          */ \
                               (c == 'p') || (c == 'P') || /* binary exponent chars   */ \
                               (c == 'a') || (c == 'A') || /* char for NaN            */ \
                               (c == 'n') || (c == 'N') || /* char for NaN & INFINITY */ \
                               (c == 'i') || (c == 'I') || /* char for INFINITY       */ \
                               (c == 'f') || (c == 'F') || /* char for INFINITY       */ \
                               (c == 't') || (c == 'T') || /* char for INFINITY       */ \
                               (c == 'y') || (c == 'Y') )  /* char for INFINITY       */



/** \brief Detect and return the LC_NUMERIC char of the locale.
 */
static char
os_lcNumericGet(void)
{
    static char lcNumeric = ' ';

    /* Detect lcNumeric only once. */
    if (lcNumeric == ' ') {
        /* There could be multiple threads here, but it is still save and works.
         * Only side effect is that possibly multiple os_reports are traced. */
        char num[] = { '\0', '\0', '\0', '\0' };
        snprintf(num, 4, "%3f", 2.2);
        lcNumeric = num [1];
        if (lcNumeric != '.') {
            OS_REPORT(OS_WARNING, "os_stdlib", 0,
                      "Locale with LC_NUMERIC \'%c\' detected, which is not '.'. This can decrease performance.",
                      lcNumeric);
        }
    }

    return lcNumeric;
}


/** \brief Replace lcNumeric char with '.' in floating point string when locale dependent
 *      functions use a non '.' LC_NUMERIC, while we want locale indepenent '.'.
 */
static void
os_lcNumericReplace(char *str) {
    /* We only need to replace when standard functions
     * did not put a '.' in the result string. */
    char lcNumeric = os_lcNumericGet();
    if (lcNumeric != '.') {
        str = os_index(str, lcNumeric);
        if (str != NULL) {
            *str = '.';
        }
    }
}


/** \brief Locale independent strtod wrapper.
 */
double
os_strtod(const char *nptr, char **endptr) {
    double ret;

    if (os_lcNumericGet() == '.') {
        /* The current locale uses '.', so we can use the
         * standard functions as is. */
        ret = strtod(nptr, endptr);
    } else {
        /* The current locale uses ',', so we can not use the
         * standard functions as is, but have to do extra work
         * because ospl uses "x.x" doubles (notice the dot).
         * Just copy the string and replace the LC_NUMERIC. */
        char  nptrCopy[DOUBLE_STRING_MAX_LENGTH];
        char *nptrCopyIdx;
        char *nptrCopyEnd;
        char *nptrIdx;

        /* It is possible that the given nptr just starts with a double
         * representation but continues with other data.
         * To be able to copy not too much and not too little, we have
         * to scan across nptr until we detect the doubles' end. */
        nptrIdx = (char*)nptr;
        nptrCopyIdx = nptrCopy;
        nptrCopyEnd = nptrCopyIdx + DOUBLE_STRING_MAX_LENGTH - 1;
        while (VALID_DOUBLE_CHAR(*nptrIdx) && (nptrCopyIdx < nptrCopyEnd)) {
            if (*nptrIdx == '.') {
                /* Replace '.' with locale LC_NUMERIC to get strtod to work. */
                *nptrCopyIdx = os_lcNumericGet();
            } else {
                *nptrCopyIdx = *nptrIdx;
            }
            nptrIdx++;
            nptrCopyIdx++;
        }
        *nptrCopyIdx = '\0';

        /* Now that we have a copy with the proper locale LC_NUMERIC,
         * we can use strtod() for the conversion. */
        ret = strtod(nptrCopy, &nptrCopyEnd);

        /* Calculate the proper end char when needed. */
        if (endptr != NULL) {
            *endptr = (char*)nptr + (nptrCopyEnd - nptrCopy);
        }
    }
    return ret;
}


/** \brief Locale independent strtof wrapper.
 */
float
os_strtof(const char *nptr, char **endptr) {
    /* Just use os_strtod(). */
    return (float)os_strtod(nptr, endptr);
}


/** \brief Locale independent snprintf wrapper, which mirrors os_strtod.
 */
int
os_dtostr(double src, char *str, size_t size) {
    int i;
    /* Use locale dependent standard function. */
    i = snprintf(str, size, "%0.15g", src);
    /* Make sure the result is a locale independent "x.x" double. */
    os_lcNumericReplace(str);
    return i;
}


/** \brief Locale independent snprintf wrapper, which mirrors os_strtof.
 */
int
os_ftostr(float src, char *str, size_t size) {
    int i;
    /* Use locale dependent standard function. */
    i = snprintf(str, size, "%0.7g", src);
    /* Make sure the result is a locale independent "x.x" float. */
    os_lcNumericReplace(str);
    return i;
}
