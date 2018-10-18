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
#include "mm_metakindNames.h"
#include "u_user.h"
#include "c_base.h"
#include "c__base.h"
#include "ut_collection.h"
#include "os_errno.h"
#include "os_abstract.h"
#include "os_stdlib.h"
#include "c_module.h"
#include "os_atomics.h"

#include "mm_trc.h"

#include <sys/types.h>
#include <regex.h>
#include <math.h>

#define MM_HEADER_SIZE  16  /* Assumed header size */

/* c_base definitions */
#define CONFIDENCE (0x504F5448)

#ifdef OBJECT_WALK
#define c_oid(o)    ((c_object)(C_ADDRESS(o) + HEADERSIZE))
#define c_header(o) ((c_header)(C_ADDRESS(o) - HEADERSIZE))
#endif

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
};
#endif
#endif

C_STRUCT(monitor_trc) {
    c_long objectCountLimit;
    char *filterExpression;
    regex_t expression;
    ut_table extTree;
    os_address totalCount;
    long totalExtentCount;
    orderKind oKind;
    int orderCount;
    int index;
    ut_table orderedList;
    unsigned int cycleNr;
    c_bool delta;
};

#ifdef OBJECT_WALK
static const c_address HEADERSIZE = ALIGNSIZE(C_SIZEOF(c_header));
#endif

C_CLASS(refLeaf);
C_STRUCT(refLeaf) {
    c_type tr;
    unsigned int ec;
    unsigned int ecp;
    unsigned int cycleNr;
};

static refLeaf
monitor_typeExtent (
    c_type o,
    monitor_trc trace
    )
{
    refLeaf ted = NULL;

    ted = (refLeaf)ut_get (ut_collection(trace->extTree), o);
    if (ted) {
        if (ted->cycleNr != trace->cycleNr) {
            ted->ecp = ted->ec;
            ted->ec = pa_ld32 (&o->objectCount);
            ted->cycleNr = trace->cycleNr;
        }
    } else {
        ted = malloc (C_SIZEOF(refLeaf));
        ted->tr = o;
        ted->ec = pa_ld32 (&o->objectCount);
        ted->ecp = 0;
        ted->cycleNr = trace->cycleNr;
        (void) ut_tableInsert(trace->extTree, o, ted);
    }
    return ted;
}

static os_equality
compareLeafs (
    c_object o1,
    c_object o2,
    c_voidp args
    )
{
    OS_UNUSED_ARG(args);
    if (o1 < o2) {
        return OS_LT;
    } else if (o1 > o2) {
        return OS_GT;
    }
    return OS_EQ;
}

/*
 * Tree sorting algorithm: first sort by indicated sortKind, then by Object pointer.
 * The second sorting algorithm is required to prevent different types sharing the
 * same sizes from overwriting eachother.
 */
static long long absdelta_uns (unsigned a, unsigned b)
{
    return (long long) (a > b ? a - b : b - a);
}

static os_equality
orderLeafs (
    c_object o1,
    c_object o2,
    c_voidp args
    )
{
    monitor_trc trace = (monitor_trc) args;
    long long n1 = 0, n2 = 0;

    /* First determine whether to sort deltas or current state. */
    if (trace->delta)
    {
        refLeaf ted1 = monitor_typeExtent (c_type(o1), trace);
        refLeaf ted2 = monitor_typeExtent (c_type(o2), trace);

        /* Sort deltas: now determine orderingKind. */
        switch (trace->oKind)
        {
            case ORDER_BY_COUNT:
                n1 = absdelta_uns (ted1->ec, ted1->ecp);
                n2 = absdelta_uns (ted2->ec, ted2->ecp);
                break;
            case ORDER_BY_SIZE:
                n1 = (long long) c_type(o1)->size;
                n2 = (long long) c_type(o2)->size;
                break;
            case ORDER_BY_TOTAL:
                n1 = absdelta_uns (ted1->ec, ted1->ecp) * (long long) c_type(o1)->size;
                n2 = absdelta_uns (ted2->ec, ted2->ecp) * (long long) c_type(o2)->size;
                break;
            default:
                n1 = 0;
                n2 = 0;
                assert(FALSE);
        }
    }
    else
    {
        /* Sort current state: now determine orderingKind. */
        switch (trace->oKind)
        {
            case ORDER_BY_COUNT:
                n1 = (long long) pa_ld32 (&c_type(o1)->objectCount);
                n2 = (long long) pa_ld32 (&c_type(o2)->objectCount);
                break;
            case ORDER_BY_SIZE:
                n1 = (long long) c_type(o1)->size;
                n2 = (long long) c_type(o2)->size;
                break;
            case ORDER_BY_TOTAL:
                n1 = (long long) pa_ld32 (&c_type(o1)->objectCount) * (long long) c_type(o1)->size;
                n2 = (long long) pa_ld32 (&c_type(o2)->objectCount) * (long long) c_type(o2)->size;
                break;
            default:
                n1 = 0;
                n2 = 0;
                assert(FALSE);
        }
    }

    if (n1 > n2) {
        return OS_LT;
    } else if (n1 < n2) {
        return OS_GT;
    }
    /* In case of equal size, then sort by object pointer to prevent different types from overwriting eachother. */
    if (o1 > o2) {
        return OS_LT;
    } else if (o1 < o2) {
        return OS_GT;
    }

    return OS_EQ;
}

monitor_trc
monitor_trcNew (
    c_long objectCountLimit,
    const char*filterExpression,
    orderKind oKind,
    int orderCount,
    c_bool delta)
{
    char expressionError [1024];
    monitor_trc o = malloc (C_SIZEOF(monitor_trc));

    if (o) {
        o->objectCountLimit = objectCountLimit;
        if (filterExpression) {
            o->filterExpression = os_strdup(filterExpression);
        } else {
            o->filterExpression = NULL;
        }
        if (o->filterExpression) {
            if (regcomp (&o->expression, o->filterExpression, REG_EXTENDED) != 0) {
                regerror (os_getErrno(), &o->expression, expressionError, sizeof(expressionError));
                printf ("Filter expression error: %s\r\n", expressionError);
                fflush(stdout);
                regfree (&o->expression);
                os_free (o->filterExpression);
                o->filterExpression = NULL;
            }
        }
        o->extTree = ut_tableNew (compareLeafs, NULL, NULL, NULL, NULL, NULL);
        o->delta = delta;
        o->oKind = oKind;
        o->orderCount = orderCount;
        o->index = 0;
        o->orderedList = NULL;
        o->cycleNr = 0;
    }
    return o;
}

void
monitor_trcFree (
    monitor_trc o
    )
{
    if (o) {
        if (o->filterExpression) {
            os_free (o->filterExpression);
            regfree (&o->expression);
        }
        ut_tableFree(o->extTree);
        o->extTree = NULL;
        free (o);
    }
}

void
printScope (
    c_metaObject scope
    )
{
    if (scope->definedIn) {
       printScope (scope->definedIn);
       printf ("::%s", scope->name);
       fflush(stdout);
    } else {
       if (scope->name) {
          printf ("::%s", scope->name);
          fflush(stdout);
       }
    }
}

static void
metaobject (
    c_metaObject o,
    void *trace_voidp
    )
{
    monitor_trc trace = (monitor_trc)trace_voidp;
    regmatch_t match[1];
    refLeaf ted;
    c_long result;

    if (c_baseObject(o)->kind < M_COUNT) {
        switch (c_baseObject(o)->kind) {
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
                break;
            case M_COLLECTION:
                switch (c_collectionType(o)->kind) {
                    case OSPL_C_UNDEFINED:
                    case OSPL_C_LIST:
                    case OSPL_C_ARRAY:
                    case OSPL_C_BAG:
                    case OSPL_C_SET:
                    case OSPL_C_MAP:
                    case OSPL_C_DICTIONARY:
                    case OSPL_C_SEQUENCE:
                    case OSPL_C_STRING:
                    case OSPL_C_WSTRING:
                    case OSPL_C_QUERY:
                    case OSPL_C_SCOPE:
                    case OSPL_C_COUNT:
                        if (trace->delta) {
                            ted = monitor_typeExtent (c_type(o), trace);
                            if (ted) {
                                result = ted->ec - ted->ecp;
                                if (result >= trace->objectCountLimit) {
                                    if (trace->filterExpression) {
                                        if (regexec (&trace->expression, o->name, 1, match, 0) == REG_NOMATCH) {
                                            break;
                                        }
                                    }
                                    printf ("%6d (%8d) %8d %12lld %-15s %-15s ",
                                            ted->ec - ted->ecp,
                                            ted->ec,
                                            (int)c_type(o)->size,
                                            (long long)(c_type(o)->size * (long long)((int)ted->ec - (int)ted->ecp)),
                                            baseKind[c_baseObject(o)->kind],
                                            collectionKind[c_collectionType(o)->kind]);

                                    printScope (o->definedIn);
                                    printf ("::%s\r\n", o->name);
                                    fflush(stdout);

                                    trace->totalCount += c_type(o)->size * (ted->ec - ted->ecp);
                                    trace->totalExtentCount += ted->ec - ted->ecp;
                                    trace->index++;
                                }
                            }
                        } else {
                            result = pa_ld32 (&c_type(o)->objectCount);
                            if (result >= trace->objectCountLimit) {
                                if (trace->filterExpression) {
                                    if (regexec (&trace->expression, o->name, 1, match, 0) == REG_NOMATCH) {
                                        break;
                                    }
                                }
                                printf ("%8d %8d %12lld %-15s %-15s ",
                                        pa_ld32 (&c_type(o)->objectCount),
                                        (int)c_type(o)->size,
                                        (long long)((long long)c_type(o)->size * pa_ld32 (&c_type(o)->objectCount)),
                                        baseKind[c_baseObject(o)->kind],
                                        collectionKind[c_collectionType(o)->kind]);
                                printScope (o->definedIn);
                                printf ("::%s\r\n", o->name);
                                fflush(stdout);
                                trace->totalCount += c_type(o)->size * pa_ld32 (&c_type(o)->objectCount);
                                trace->totalExtentCount += pa_ld32 (&c_type(o)->objectCount);
                                trace->index++;
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
                    ted = monitor_typeExtent (c_type(o), trace);
                    if (ted) {
                        result = ted->ec - ted->ecp;
                        if (result >= trace->objectCountLimit) {
                            if (trace->filterExpression) {
                                if (regexec (&trace->expression, o->name, 1, match, 0) == REG_NOMATCH) {
                                    break;
                                }
                            }
                            printf ("%6d (%8d) %8d %12lld %-15s ",
                                    ted->ec - ted->ecp,
                                    ted->ec,
                                    (int)c_type(o)->size,
                                    (long long)(c_type(o)->size * (long long)((int)ted->ec - (int)ted->ecp)),
                                    baseKind[c_baseObject(o)->kind]);
                            printf ("                ");
                            printScope (o->definedIn);
                            printf ("::%s\r\n", o->name);
                            fflush(stdout);

                            trace->totalCount += c_type(o)->size * (ted->ec - ted->ecp);
                            trace->totalExtentCount += ted->ec - ted->ecp;
                            trace->index++;
                        }
                    }
                } else {
                    result = pa_ld32 (&c_type(o)->objectCount);
                    if (result >= trace->objectCountLimit) {
                        if (trace->filterExpression) {
                            if (regexec (&trace->expression, o->name, 1, match, 0) == REG_NOMATCH) {
                                break;
                            }
                        }
                        printf ("%8d %8d %12lld %-15s ",
                                pa_ld32 (&c_type(o)->objectCount),
                                (int)c_type(o)->size,
                                (long long)((long long)c_type(o)->size * pa_ld32 (&c_type(o)->objectCount)),
                                baseKind[c_baseObject(o)->kind]);
                        printf ("                ");
                        printScope (o->definedIn);
                        printf ("::%s\r\n", o->name);
                        fflush(stdout);
                        trace->totalCount += c_type(o)->size * pa_ld32 (&c_type(o)->objectCount);
                        trace->totalExtentCount += pa_ld32 (&c_type(o)->objectCount);
                        trace->index++;
                    }
                }
                break;
            case M_MODULE:
                c_metaWalk (o, metaobject, trace);
        }
    }
}

static void
orderedWalk (
    c_metaObject o,
    void *trace_voidp
    )
{
    monitor_trc trace = (monitor_trc) trace_voidp;
    switch (c_baseObject(o)->kind) {
        case M_MODULE:
            c_metaWalk (o, orderedWalk, trace);
            break;
        case M_UNDEFINED:
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
            break;
        default:
            (void) ut_tableInsert(trace->orderedList, o, o);
            break;
    }
}

static os_int32
orderedAction(void *o, void *arg)
{
    os_int32 result;
    monitor_trc trace = (monitor_trc) arg;

    if (trace->index < trace->orderCount)
    {
        metaobject(o, trace);
        result = TRUE;
    }
    else
    {
        result = FALSE;
    }
    return result;
}

void
monitor_trcAction (
    v_public entity,
    c_voidp args
    )
{
    c_base base;
    monitor_trc trace = monitor_trc(args);
    time_t t;
    os_address totalSize;

    time (&t);
    base = c_getBase(entity);
    printf ("\r\n################# Start tracing ################## %s\r\n", ctime(&t));
    printf ("Limit             : %d\r\n", trace->objectCountLimit);
    if (trace->filterExpression) {
        printf ("Filter expression : %s\r\n", trace->filterExpression);
    }
    printf ("\r\n");

    if (trace->delta)
    {
        printf ("%6s (%8s) %8s %12s %-15s %-15s %s\r\n",
                "Delta",
                "ObjCount",
                "ObjSize",
                "TotalSize",
                "ObjectKind",
                "CollectionKind",
                "TypeName");
    }
    else
    {
        printf ("%8s %8s %12s %-15s %-15s %s\r\n",
                "ObjCount",
                "ObjSize",
                "TotalSize",
                "ObjectKind",
                "CollectionKind",
                "TypeName");
    }
    printf ("----------------------------------------------------------------------------------------------------------\r\n");
    fflush(stdout);
    trace->totalCount = 0;
    trace->totalExtentCount = 0;
    trace->index = 0;
    if (trace->oKind == NO_ORDERING)
    {
        c_metaWalk (c_metaObject (base), metaobject, trace);
    }
    else
    {
        trace->orderedList = ut_tableNew (orderLeafs, trace, NULL, NULL, NULL, NULL);
        c_metaWalk (c_metaObject (base), orderedWalk, trace);
        ut_walk(ut_collection(trace->orderedList), orderedAction, trace);
        ut_tableFree(trace->orderedList);
        trace->orderedList = NULL;
    }
    printf ("\r\n");
    printf ("        %ld  for %ld object headers ("PA_ADDRFMT") and MM headers (%d)\r\n",
        trace->totalExtentCount * (C_SIZEOF(c_header)  + MM_HEADER_SIZE),
        trace->totalExtentCount,
        (PA_ADDRCAST) C_SIZEOF(c_header), MM_HEADER_SIZE);
    totalSize = trace->totalCount + trace->totalExtentCount * (C_SIZEOF(c_header)  + MM_HEADER_SIZE);
    printf ("Total : "PA_ADDRFMT"  (%.2f KB)\r\n", totalSize, (double)totalSize/1024.0);
    fflush(stdout);
    trace->cycleNr++;
}
