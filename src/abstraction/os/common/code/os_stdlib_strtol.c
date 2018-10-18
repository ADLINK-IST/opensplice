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
#include <ctype.h>
#include "os_errno.h"
#include "os_string.h"

static unsigned long long
os__strtoull__ (
    const os_char *str,
    os_char **endptr,
    os_int32 base,
    unsigned long long max)
{
    os_int err = 0;
    os_int num;
    os_size_t cnt = 0;
    unsigned long long tot = 0;

    assert (str != NULL);

    if (base == 0) {
        if (str[0] == '0') {
            if ((str[1] == 'x' || str[1] == 'X') && os_todigit (str[2]) < 16) {
                base = 16;
                cnt = 2;
            } else {
                base = 8;
            }
        } else {
            base = 10;
        }
    } else if (base == 16) {
        if (str[0] == '0' && (str[1] == 'x' || str[1] == 'X')) {
            cnt = 2;
        }
    } else if (base < 2 || base > 36) {
        err = EINVAL;
    }

    while (!err && (num = os_todigit (str[cnt])) >= 0 && num < base) {
        if (tot <= (max / (unsigned) base)) {
            tot *= (unsigned) base;
            tot += (unsigned) num;
            cnt++;
        } else {
            err = ERANGE;
            tot = max;
        }
    }

    if (endptr != NULL) {
        *endptr = (os_char *)str + cnt;
    }

    if (err) {
        os_setErrno(err);
    }

    return tot;
}

long long
os_strtoll(
    const os_char *str,
    os_char **endptr,
    os_int32 base)
{
    os_size_t cnt = 0;
    long long tot = 1;
    unsigned long long max = OS_MAX_INTEGER(long long);

    assert (str != NULL);

    for (; isspace(str[cnt]); cnt++) {
        /* ignore leading whitespace */
    }

    if (str[cnt] == '-') {
        tot = -1;
        max++;
        cnt++;
    } else if (str[cnt] == '+') {
        cnt++;
    }

    tot *= (long long) os__strtoull__ (str + cnt, endptr, base, max);

    if (endptr && *endptr == (str + cnt)) {
       *endptr = (os_char *)str;
    }

    return tot;
}

unsigned long long
os_strtoull (
    const os_char *str,
    os_char **endptr,
    os_int32 base)
{
    os_size_t cnt = 0;
    unsigned long long tot = 1;
    unsigned long long max = OS_MAX_INTEGER(unsigned long long);

    assert (str != NULL);

    for (; isspace(str[cnt]); cnt++) {
        /* ignore leading whitespace */
    }

    if (str[cnt] == '-') {
        tot = (unsigned long long) -1;
        cnt++;
    } else if (str[cnt] == '+') {
        cnt++;
    }

    tot *= os__strtoull__ (str + cnt, endptr, base, max);

    if (endptr && *endptr == (str + cnt)) {
       *endptr = (os_char *)str;
    }

    return tot;
}

long long
os_atoll(
    const char *str)
{
    return os_strtoll(str, NULL, 10);
}

unsigned long long
os_atoull(
    const char *str)
{
    return os_strtoull(str, NULL, 10);
}

os_char *
os_ulltostr (
    unsigned long long num,
    os_char *str,
    os_size_t len,
    os_char **endptr)
{
    os_char chr, *ptr;
    os_size_t cnt;
    os_size_t lim = 0;
    os_size_t tot = 0;

    assert (str != NULL);

    if (len > 1) {
        lim = len - 1;

        do {
            str[tot] = (os_char)('0' + (int)(num % 10));
            num /= 10;

            if (tot == (lim - 1)) {
                if (num > 0ULL) {
                    /* Simply using memmove would have been easier, but the
                       function is safe to use in asynchronous code like this.
                       Normally this code should not affect performance,
                       because ideally the buffer is sufficiently large
                       enough. */
                    for (cnt = 0; cnt < tot; cnt++) {
                        str[cnt] = str[cnt + 1];
                    }
                }
            } else if (num > 0ULL) {
                tot++;
            }
        } while (num > 0ULL);

        lim = tot + 1;
    }

    for (cnt = 0; cnt < (tot - cnt); cnt++) {
        chr = str[tot - cnt];
        str[tot - cnt] = str[cnt];
        str[cnt] = chr;
    }

    if (len == 0) {
        str = NULL;
        ptr = NULL;
    } else {
        str[lim] = '\0';
        ptr = str + lim;
    }

    if (endptr != NULL) {
        *endptr = ptr;
    }

    return str;
}

os_char *
os_lltostr (
   long long num,
   os_char *str,
   os_size_t len,
   os_char **endptr)
{
    unsigned long long pos;
    os_char *ptr;
    os_size_t cnt = 0;

    assert (str != NULL);

    if (len == 0) {
        str = NULL;
        ptr = NULL;
    } else if (len == 1) {
        str[0] = '\0';
        ptr = str;
    } else {
        if (num < 0LL) {
            if (num == OS_MIN_INTEGER(long long)) {
                pos = (unsigned long long)OS_MAX_INTEGER(long long) + 1;
            } else {
                pos = (unsigned long long) -num;
            }

            str[cnt++] = '-';
        } else {
            pos = (unsigned long long) num;
        }

        (void)os_ulltostr (pos, str + cnt, len - cnt, &ptr);
    }

    if (endptr != NULL) {
        *endptr = ptr;
    }

    return str;
}
