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

#include "os_stdlib.h"
#include "os_string.h"
#include "os_heap.h"

#define OS_STR_SPACE " \t\r\n"

os_char *
os_strchrs (
    const os_char *str,
    const os_char *chrs,
    os_boolean inc)
{
    os_boolean eq;
    os_char *ptr = NULL;
    os_size_t i, j;

    assert (str != NULL);
    assert (chrs != NULL);

    for (i = 0; str[i] != '\0' && ptr == NULL; i++) {
        for (j = 0, eq = OS_FALSE; chrs[j] != '\0' && eq == OS_FALSE; j++) {
            if (str[i] == chrs[j]) {
                eq = OS_TRUE;
            }
        }
        if (eq == inc) {
            ptr = (os_char *)str + i;
        }
    }

    return ptr;
}

os_char *
os_strrchrs (
    const os_char *str,
    const os_char *chrs,
    os_boolean inc)
{
    os_boolean eq;
    os_char *ptr = NULL;
    os_size_t i, j;

    assert (str != NULL);
    assert (chrs != NULL);

    for (i = 0; str[i] != '\0'; i++) {
        for (j = 0, eq = OS_FALSE; chrs[j] != '\0' && eq == OS_FALSE; j++) {
            if (str[i] == chrs[j]) {
                eq = OS_TRUE;
            }
        }
        if (eq == inc) {
            ptr = (os_char *)str + i;
        }
    }

    return ptr;
}

os_char *
os_str_replace (
    const os_char *str,
    const os_char *srch,
    const os_char *subst,
    os_int max)
{
    os_char *off, *lim;
    os_int cnt = 0;
    os_ssize_t len, tot;
    os_ssize_t xtra;
    os_char *alloc, *ptr;

    assert (str != NULL);
    assert (srch != NULL);

    if (subst == NULL) {
        subst = "";
    }

    len = (os_ssize_t) strlen (srch);
    tot = (os_ssize_t) strlen (str);
    xtra = (os_ssize_t) strlen (subst) - len;

    ptr = off = lim = (os_char *)str;
    do {
        off = (os_char *)strstr (lim, srch);
        if (off != NULL) {
            cnt++;
            lim = off + len;

            if (ptr == str) {
                alloc = os_malloc ((os_size_t) (tot + xtra) + 1 /* '\0' */);
            } else if (xtra > 0) {
                alloc = os_realloc (ptr, (os_size_t) (tot + xtra) + 1 /* '\0' */);
            } else {
                alloc = ptr;
            }

            if (alloc != NULL) {
                /* copy head into position if memory was (re)allocated */
                if (alloc != ptr) {
                    if (ptr == str) {
                        memmove (alloc, ptr, (os_size_t) (off - ptr));
                    } else {
                        lim = alloc + (lim - ptr);
                    }
                    off = alloc + (off - ptr);
                }
                /* copy tail into position */
                memmove (off + (len + xtra), lim, (os_size_t) (tot - ((off + len) - alloc)));
                lim = off + (len + xtra);
                tot += xtra;
                /* null terminate */
                alloc[tot] = '\0';
                /* copy substitute into position */
                memcpy (off, subst, (os_size_t) (len + xtra));
            } else if (ptr != str) {
                os_free (ptr);
            }

            ptr = alloc;
        }
    } while ((max == 0 || cnt < max) && ptr != NULL && off != NULL);

    return ptr;
}

os_char *
os_str_word_replace (
    const os_char *str,
    const os_char *delim,
    const os_char *word,
    const os_char *subst,
    os_int max)
{
    os_char *off, *lim;
    os_int cnt = 0;
    os_size_t diff, len, tot;
    os_ssize_t xtra;
    os_char *alloc, *ptr;

    assert (str != NULL);
    assert (word != NULL);

    if (delim == NULL) {
        delim = OS_STR_SPACE;
    }
    if (subst == NULL) {
        subst = "";
    }

    len = strlen (word);
    tot = strlen (str);
    xtra = (os_ssize_t) strlen (subst) - (os_ssize_t) len;

    ptr = off = lim = (os_char *)str;
    do {
        off = os_strchrs (lim, delim, OS_FALSE);
        if (off != NULL) {
            diff = tot - (os_size_t)(off - ptr);
            if (diff > len) {
                lim = os_strchrs (off + len, delim, OS_TRUE);
                if (lim != off + len) {
                    lim = os_strchrs (off, delim, OS_TRUE);
                }
                if (lim != NULL) {
                    diff = (os_size_t) (lim - off);
                } else {
                    assert (off[diff] == '\0');
                    lim = off + diff;
                }
            } else {
                assert (off[diff] == '\0');
                lim = off + diff;
            }

            if (diff == len && strncmp (off, word, len) == 0) {
                cnt++;
                if (ptr == str) {
                    alloc = os_malloc ((os_size_t) (((os_ssize_t)tot + xtra) + 1) /* '\0' */);
                } else if (xtra > 0) {
                    alloc = os_realloc (ptr, (os_size_t) (((os_ssize_t)tot + xtra) + 1) /* '\0' */);
                } else {
                    alloc = ptr;
                }

                /* reposition pointers */
                if (alloc != NULL) {
                    /* copy head into position if memory was (re)allocated */
                    if (alloc != ptr) {
                        if (ptr == str) {
                            memmove (alloc, ptr, (os_size_t) (off - ptr));
                        } else {
                            lim = alloc + (lim - ptr);
                        }
                        off = alloc + (off - ptr);
                    }
                    /* copy tail into position */
                    memmove (
                        off + ((os_ssize_t)diff + xtra), lim, tot - (os_size_t) ((off + diff) - alloc));
                    lim = off + ((os_ssize_t)diff + xtra);
                    tot = (os_size_t) ((os_ssize_t)tot + xtra);
                    /* null terminate */
                    alloc[tot] = '\0';
                    /* copy substitute into position */
                    memcpy (off, subst, (os_size_t) ((os_ssize_t)len + xtra));
                } else if (ptr != str) {
                    os_free (ptr);
                }

                ptr = alloc;
            }
        }
    } while ((max == 0 || cnt < max) && ptr != NULL &&
                                        off != NULL &&
                                        lim != NULL);

    return ptr;
}

os_char *
os_str_ltrim (
    const os_char *str,
    const os_char *trim)
{
    os_char *off, *ptr;

    assert (str != NULL);

    if (trim == NULL) {
        trim = OS_STR_SPACE;
    }

    off = os_strchrs (str, trim, OS_FALSE);
    if (off != NULL) {
        if (off != str) {
            ptr = os_strdup (off);
        } else {
            ptr = (os_char *)str;
        }
    } else {
        ptr = os_strdup ("");
    }

    return ptr;
}

os_char *
os_str_rtrim (
    const os_char *str,
    const os_char *trim)
{
    os_char *lim, *ptr;
    os_size_t excess_chars;

    assert (str != NULL);

    if (trim == NULL) {
        trim = OS_STR_SPACE;
    }

    lim = os_strrchrs (str, trim, OS_FALSE);
    excess_chars = strlen(lim) - strlen(trim);
    if (lim != NULL) {
        /* if byte following last occurrence of non-trim character is null
           terminating byte, nothing needs to be replaced */
        if (lim[1] != '\0') {
            ptr = os_strndup (str, (os_size_t)((lim - str) + excess_chars));
        } else {
            ptr = (os_char *)str;
        }
    } else {
        ptr = os_strdup ("");
    }

    return ptr;
}

os_char *
os_str_trim (
    const os_char *str,
    const os_char *trim)
{
    os_char *lim, *off, *ptr;

    assert (str != NULL);

    if (trim == NULL) {
        trim = OS_STR_SPACE;
    }

    off = os_strchrs (str, trim, OS_FALSE);
    if (off != NULL) {
        lim = os_strrchrs (off, trim, OS_FALSE);
        assert (lim != NULL);
        if (lim[1] != '\0') {
            ptr = os_strndup (off, (os_size_t)((lim - off) + 1));
        } else if (off != str) {
            ptr = os_strdup (off);
        } else {
            ptr = (os_char *)str;
        }
    } else {
        ptr = os_strdup ("");
    }

    return ptr;
}

#undef OS_STR_SPACE

char *
os_strndup (
    const os_char *str,
    os_size_t max)
{
    char *ptr;
    size_t cnt;

    assert(str);
    assert(max);

    /* Manual strnlen(...) implementation, because many platforms don't have
     * strnlen.
     *
     * The following feature test enables it on the mainstream supported
     * platforms, but for now the loop below suffices.
     *
     * _XOPEN_SOURCE >= 700 || _POSIX_C_SOURCE >= 200809L ||
     *     (defined(_MSC_VER) && (!defined(_WIN32_WCE) || (_WIN32_WCE >= 600)))
     */
    for (cnt = 0; cnt < max && str[cnt] != '\0'; cnt++) {
        /* Do nothing */
    }

    ptr = os_malloc (cnt + 1);
    memcpy (ptr, str, cnt);
    ptr[cnt] = '\0';

    return ptr;
}

os_int
os_todigit (
    const os_int chr)
{
    os_int num = -1;

    if (chr >= '0' && chr <= '9') {
        num = chr - '0';
    } else if (chr >= 'a' && chr <= 'z') {
        num = 10 + (chr - 'a');
    } else if (chr >= 'A' && chr <= 'Z') {
        num = 10 + (chr - 'A');
    }

    return num;
}
