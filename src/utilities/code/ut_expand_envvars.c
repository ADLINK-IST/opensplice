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
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>

#include "vortex_os.h"
#include "os_report.h"
#include "ut_expand_envvars.h"

typedef char * (*expand_fn)(const char *src0);

static void expand_append (char **dst, size_t *sz, size_t *pos, char c)
{
    if (*pos == *sz) {
        *sz += 1024;
        *dst = os_realloc (*dst, *sz);
    }
    (*dst)[*pos] = c;
    (*pos)++;
}

static char *expand_env (const char *name, char op, const char *alt, expand_fn expand)
{
    const char *env = os_getenv (name);
    switch (op)
    {
        case 0:
            return os_strdup (env ? env : "");
        case '-':
            return env && *env ? os_strdup (env) : expand (alt);
        case '?':
            if (env && *env) {
                return os_strdup (env);
            } else {
                char *altx = expand (alt);
                OS_REPORT (OS_ERROR, "configuration parser", 0, "%s: %s\n", name, altx);
                os_free (altx);
                return NULL;
            }
        case '+':
            return env && *env ? expand (alt) : os_strdup ("");
        default:
            abort ();
            return NULL;
    }
}

static char *expand_envbrace (const char **src, expand_fn expand)
{
    const char *start = *src + 1;
    char *name, *x;
    assert (**src == '{');
    (*src)++;
    while (**src && **src != ':' && **src != '}') {
        (*src)++;
    }
    if (**src == 0) {
        goto err;
    }
    name = os_malloc ((size_t) (*src - start) + 1);
    memcpy (name, start, (size_t) (*src - start));
    name[*src - start] = 0;
    if (**src == '}') {
        (*src)++;
        x = expand_env (name, 0, NULL, expand);
        os_free (name);
        return x;
    } else {
        const char *altstart;
        char *alt;
        char op;
        int nest = 0;
        assert (**src == ':');
        (*src)++;
        switch (**src) {
            case '-': case '+': case '?':
                op = **src;
                (*src)++;
                break;
            default:
                os_free(name);
                goto err;
        }
        altstart = *src;
        while (**src && (**src != '}' || nest > 0)) {
            if (**src == '{') {
                nest++;
            } else if (**src == '}') {
                assert (nest > 0);
                nest--;
            } else if (**src == '\\') {
                (*src)++;
                if (**src == 0) {
                    os_free(name);
                    goto err;
                }
            }
            (*src)++;
        }
        if (**src == 0) {
            os_free(name);
            goto err;
        }
        assert (**src == '}');
        alt = os_malloc ((size_t) (*src - altstart) + 1);
        memcpy (alt, altstart, (size_t) (*src - altstart));
        alt[*src - altstart] = 0;
        (*src)++;
        x = expand_env (name, op, alt, expand);
        os_free (alt);
        os_free (name);
        return x;
    }
err:
    OS_REPORT (OS_ERROR, "configuration parser", 0, "%*.*s: invalid expansion\n", (int) (*src - start), (int) (*src - start), start);
    return NULL;
}

static char *expand_envsimple (const char **src, expand_fn expand)
{
    const char *start = *src;
    char *name, *x;
    while (**src && (isalnum (**src) || **src == '_')) {
        (*src)++;
    }
    assert (*src > start);
    name = os_malloc ((size_t) (*src - start) + 1);
    memcpy (name, start, (size_t) (*src - start));
    name[*src - start] = 0;
    x = expand_env (name, 0, NULL, expand);
    os_free (name);
    return x;
}

static char *expand_envchar (const char **src, expand_fn expand)
{
    char name[2];
    assert (**src);
    name[0] = **src;
    name[1] = 0;
    (*src)++;
    return expand_env (name, 0, NULL, expand);
}

char *ut_expand_envvars_sh (const char *src0)
{
    /* Expands $X, ${X}, ${X:-Y}, ${X:+Y}, ${X:?Y} forms; $ and \ can be escaped with \ */
    const char *src = src0;
    size_t sz = strlen (src) + 1, pos = 0;
    char *dst = os_malloc (sz);
    while (*src) {
        if (*src == '\\') {
            src++;
            if (*src == 0) {
                OS_REPORT (OS_ERROR, "configuration parser", 0, "%s: incomplete escape at end of string\n", src0);
                os_free(dst);
                return NULL;
            }
            expand_append (&dst, &sz, &pos, *src++);
        } else if (*src == '$') {
            char *x, *xp;
            src++;
            if (*src == 0) {
                OS_REPORT (OS_ERROR, "configuration parser", 0, "%s: incomplete variable expansion at end of string\n", src0);
                os_free(dst);
                return NULL;
            } else if (*src == '{') {
                x = expand_envbrace (&src, &ut_expand_envvars_sh);
            } else if (isalnum (*src) || *src == '_') {
                x = expand_envsimple (&src, &ut_expand_envvars_sh);
            } else {
                x = expand_envchar (&src, &ut_expand_envvars_sh);
            }
            if (x == NULL) {
                os_free(dst);
                return NULL;
            }
            xp = x;
            while (*xp) {
                expand_append (&dst, &sz, &pos, *xp++);
            }
            os_free (x);
        } else {
            expand_append (&dst, &sz, &pos, *src++);
        }
    }
    expand_append (&dst, &sz, &pos, 0);
    return dst;
}

char *ut_expand_envvars (const char *src0)
{
    /* Expands ${X}, ${X:-Y}, ${X:+Y}, ${X:?Y} forms, but not $X */
    const char *src = src0;
    size_t sz = strlen (src) + 1, pos = 0;
    char *dst = os_malloc (sz);
    while (*src) {
        if (*src == '$' && *(src + 1) == '{') {
            char *x, *xp;
            src++;
            x = expand_envbrace (&src, &ut_expand_envvars);
            if (x == NULL) {
                os_free(dst);
                return NULL;
            }
            xp = x;
            while (*xp) {
                expand_append (&dst, &sz, &pos, *xp++);
            }
            os_free (x);
        } else {
            expand_append (&dst, &sz, &pos, *src++);
        }
    }
    expand_append (&dst, &sz, &pos, 0);
    return dst;
}
