/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2013 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE
 *
 *   for full copyright notice and license terms.
 *
 */
#include "mm_metakindNames.h"
#include "u_user.h"
#include "c_base.h"
#include "c__base.h"
#include "ut_collection.h"
#include "os_stdlib.h"
#include "c_module.h"

#include "mm_orc.h"

#include <sys/types.h>
#include <regex.h>
#include <errno.h>

#define MM_HEADER_SIZE  16  /* Assumed header size */

/* c_base definitions */
#define CONFIDENCE (0x504F5448)

#define c_oid(o)    ((c_object)(C_ADDRESS(o) + HEADERSIZE))
#define c_header(o) ((c_header)(C_ADDRESS(o) - HEADERSIZE))

#define MIN_DB_SIZE     (150000)
#define MAXREFCOUNT     (50000)
#define MAXALIGNMENT    (C_ALIGNMENT(c_value))
#define ALIGNSIZE(size) ((((size-1)/MAXALIGNMENT)+1)*MAXALIGNMENT)
#define MEMSIZE(size)   ((size)+HEADERSIZE)

#define ResolveType(s,t) c_type(c_metaResolve(c_metaObject(s),#t))

C_CLASS(c_baseBinding);
#ifndef _DA_
#ifndef __DA_
C_CLASS(c_header);

C_STRUCT(c_header) {
#ifndef NDEBUG
    c_ulong confidence;
#ifdef OBJECT_WALK
    c_object nextObject;
    c_object prevObject;
#endif
#endif
    c_ulong refCount;
    c_type type;
};
#endif
#endif


C_STRUCT(monitor_orc) {
    c_ulong extendCountLimit;
    char *filterExpression;
    regex_t expression;
    ut_collection refTree;
    int totalObjectCount;
    os_address totalSizeCount;
    c_bool delta;
};

static const c_long HEADERSIZE = ALIGNSIZE(C_SIZEOF(c_header));

C_CLASS(refLeaf);
C_STRUCT(refLeaf) {
    c_type tr;
    unsigned int rc;
    unsigned int prc;
};


static c_type nullType = (c_type)0xffffffff;

static void
freeNode (
    c_object o,
    c_voidp arg
    )
{
    free (o);
}

static os_equality
compareLeafs (
    c_object o1,
    c_object o2,
    c_voidp args
    )
{
    if (o1 < o2) {
        return OS_LT;
    } else if (o1 > o2) {
        return OS_GT;
    }
    return OS_EQ;
}

monitor_orc
monitor_orcNew (
    c_long extendCountLimit,
    const char*filterExpression,
    c_bool delta
    )
{
    char expressionError [1024];
    monitor_orc o = malloc (C_SIZEOF(monitor_orc));

    if (o)
    {
       o->extendCountLimit = extendCountLimit;
       if (filterExpression)
       {
          o->filterExpression = os_strdup(filterExpression);
       }
       else
       {
          o->filterExpression = NULL;
       }
       if (o->filterExpression)
       {
          if (regcomp (&o->expression, o->filterExpression, REG_EXTENDED) != 0)
          {
             regerror (errno,
                       &o->expression,
                       expressionError,
                       sizeof(expressionError));
             printf ("Filter expression error: %s\r\n", expressionError);
             fflush(stdout);

             regfree (&o->expression);
             free (o->filterExpression);
             o->filterExpression = NULL;
          }
       }
       o->refTree = ut_tableNew (compareLeafs, NULL);
       o->totalSizeCount = 0;
       o->totalObjectCount = 0;
       o->delta = delta;
    }

    return o;
}

static void
monitor_object (
    c_object o,
    monitor_orc trace
    )
{
    refLeaf ord;
    c_type tr = c_header(o)->type;

    if (tr == NULL) {
        tr = nullType;
    }
    ord = (refLeaf)ut_get (trace->refTree, tr);
    if (ord) {
        ord->rc++;
    } else {
        ord = malloc (C_SIZEOF(refLeaf));
        ord->tr = tr;
        ord->rc = 1;
        ord->prc = 0;
        ut_tableInsert(ut_table(trace->refTree), tr, ord);
    }
}

void
monitor_orcFree (
    monitor_orc o
    )
{
    if (o->filterExpression) {
        free (o->filterExpression);
        regfree (&o->expression);
    }
    ut_collectionFree (o->refTree, freeNode, NULL);

    free (o);
}

static void
printScope (
    c_metaObject scope
    )
{
    if (scope == NULL) {
        return;
    }
    if (scope->definedIn) {
        printScope (scope->definedIn);
        printf ("::%s", scope->name);
    } else {
        if (scope->name) {
            printf ("::%s", scope->name);
        }
    }
}

static c_bool
display_orc (
    c_object o,
    c_voidp args
    )
{
    refLeaf ord = (refLeaf)o;
    monitor_orc trace = (monitor_orc)args;
    regmatch_t match[1];

    if (ord->tr == nullType) {
        if (trace->delta) {
            if (abs(ord->rc - ord->prc) >= trace->extendCountLimit) {
                printf ("%6d                            <undefined>\r\n", (ord->rc - ord->prc));
            }
        } else {
            if (ord->rc >= trace->extendCountLimit) {
                printf ("%6d                            <undefined>\r\n", ord->rc);
            }
        }
    fflush(stdout);

    } else {
        switch (c_baseObject(ord->tr)->kind) {
        case M_COLLECTION:
            switch (c_collectionType(ord->tr)->kind) {
            case C_UNDEFINED:
            case C_LIST:
            case C_ARRAY:
            case C_BAG:
            case C_SET:
            case C_MAP:
            case C_DICTIONARY:
            case C_SEQUENCE:
            case C_STRING:
            case C_WSTRING:
            case C_QUERY:
            case C_SCOPE:
            case C_COUNT:
                if (trace->delta) {
                    if (abs(ord->rc - ord->prc) >= trace->extendCountLimit) {
                        if (trace->filterExpression) {
                            if (regexec (&trace->expression, c_metaObject(ord->tr)->name, 1, match, 0) == REG_NOMATCH) {
                                break;
                            }
                        }
                        printf ("%6d (%6d) %6d %10d %-15s %-15s ",
                            ord->rc - ord->prc,
                            ord->rc,
                            (int)c_type(ord->tr)->size,
                            (int)(c_type(ord->tr)->size * (ord->rc - ord->prc)),
                            baseKind[c_baseObject(ord->tr)->kind],
                            collectionKind[c_collectionType(ord->tr)->kind]);
                        printScope (c_metaObject(ord->tr)->definedIn);
                        printf ("::%s\r\n", c_metaObject(ord->tr)->name);
                    fflush(stdout);

                        trace->totalSizeCount += c_type(ord->tr)->size * (ord->rc - ord->prc);
                        trace->totalObjectCount += ord->rc - ord->prc;
                    }
                } else {
                    if (ord->rc >= trace->extendCountLimit) {
                        if (trace->filterExpression) {
                            if (regexec (&trace->expression, c_metaObject(ord->tr)->name, 1, match, 0) == REG_NOMATCH) {
                                break;
                            }
                        }
                        printf ("%6d %6d %10d %-15s %-15s ",
                            ord->rc,
                            (int)c_type(ord->tr)->size,
                            (int)(c_type(ord->tr)->size * ord->rc),
                            baseKind[c_baseObject(ord->tr)->kind],
                            collectionKind[c_collectionType(ord->tr)->kind]);
                        printScope (c_metaObject(ord->tr)->definedIn);
                        printf ("::%s\r\n", c_metaObject(ord->tr)->name);
                    fflush(stdout);
                        trace->totalSizeCount += c_type(ord->tr)->size * ord->rc;
                        trace->totalObjectCount += ord->rc;
                    }
                }
                break;
            }
            break;
        case M_PRIMITIVE:
        case M_TYPEDEF:
        case M_ENUMERATION:
        case M_UNION:
        case M_STRUCTURE:
        case M_INTERFACE:
        case M_CLASS:
        case M_EXCEPTION:
            if (trace->delta) {
                if (abs(ord->rc - ord->prc) >= trace->extendCountLimit) {
                    if (trace->filterExpression) {
                        if (regexec (&trace->expression, c_metaObject(ord->tr)->name, 1, match, 0) == REG_NOMATCH) {
                            break;
                        }
                    }
                    printf ("%6d (%6d) %6d %10d %-15s ",
                        ord->rc - ord->prc,
                        ord->rc,
                        (int)c_type(ord->tr)->size,
                        (int)(c_type(ord->tr)->size * (ord->rc - ord->prc)),
                        baseKind[c_baseObject(ord->tr)->kind]);
                    printf ("                ");
                    printScope (c_metaObject(ord->tr)->definedIn);
                    printf ("::%s\r\n", c_metaObject(ord->tr)->name);
                fflush(stdout);
                    trace->totalSizeCount += c_type(ord->tr)->size * (ord->rc - ord->prc);
                    trace->totalObjectCount += ord->rc - ord->prc;
                }
            } else {
                if (ord->rc >= trace->extendCountLimit) {
                    if (trace->filterExpression) {
                        if (regexec (&trace->expression, c_metaObject(ord->tr)->name, 1, match, 0) == REG_NOMATCH) {
                            break;
                        }
                    }
                    printf ("%6d %6d %10d %-15s ",
                        ord->rc,
                        (int)c_type(ord->tr)->size,
                        (int)(c_type(ord->tr)->size * ord->rc),
                        baseKind[c_baseObject(ord->tr)->kind]);
                    printf ("                ");
                    printScope (c_metaObject(ord->tr)->definedIn);
                    printf ("::%s\r\n", c_metaObject(ord->tr)->name);
                    fflush(stdout);
                    trace->totalSizeCount += c_type(ord->tr)->size * ord->rc;
                    trace->totalObjectCount += ord->rc;
                }
            }
            break;
        case M_UNDEFINED:
        case M_ANNOTATION:
        case M_ATTRIBUTE:
        case M_CONSTANT:
        case M_CONSTOPERAND:
        case M_EXPRESSION:
        case M_LITERAL:
        case M_MEMBER:
        case M_OPERATION:
        case M_PARAMETER:
        case M_RELATION:
        case M_BASE:
        case M_UNIONCASE:
        case M_COUNT:
        case M_MODULE:
            break;
        }
    }
    ord->prc = ord->rc;
    ord->rc = 0;
    return TRUE;
}

void
monitor_orcAction (
    v_entity entity,
    c_voidp args
    )
{
#ifdef OBJECT_WALK
    c_base base;
    c_object or;
    monitor_orc trace = monitor_orc(args);
    time_t t;
    int count = 0;
    os_address totalSize;

    time (&t);
    base = c_getBase(entity);
    printf ("\r\n################# Start tracing ################## %s\r\n", ctime(&t));
    printf ("Limit             : %d\r\n", trace->extendCountLimit);
    if (trace->filterExpression) {
        printf ("Filter expression : %s\r\n", trace->filterExpression);
    }
    printf ("\r\n");
    fflush(stdout);

    for (or = base->firstObject; or != base->lastObject; ) {
        or = c_header(or)->nextObject;
        monitor_object (or, trace);
        count++;
    };
    ut_walk (trace->refTree, display_orc, trace);
    printf ("\r\n");
    printf ("        %d  for %d object headers (%d) and MM headers (%d)\r\n",
        trace->totalObjectCount * (C_SIZEOF(c_header)  + MM_HEADER_SIZE),
        trace->totalObjectCount,
        C_SIZEOF(c_header), MM_HEADER_SIZE);
    totalSize = trace->totalSizeCount + trace->totalObjectCount * (C_SIZEOF(c_header)  + MM_HEADER_SIZE);
    printf ("Total : %d  (%.2f KB)\r\n", totalSize, (double)totalSize/1024.0);
    fflush(stdout);

    trace->totalSizeCount = 0;
    trace->totalObjectCount = 0;
#endif
}
