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

#include "mm_trc.h"

#include <sys/types.h>
#include <errno.h>
#include <math.h>

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

C_STRUCT(monitor_trc) {
    c_long objectCountLimit;
    char *filterExpression;
    ut_collection extTree;
    os_address totalCount;
    long totalExtentCount;
    orderKind oKind;
    int orderCount;
    int index;
    ut_collection orderedList;
    unsigned int cycleNr;
    c_bool delta;
};

static const c_long HEADERSIZE = ALIGNSIZE(C_SIZEOF(c_header));

C_CLASS(refLeaf);
C_STRUCT(refLeaf) {
    c_type tr;
    unsigned int ec;
    unsigned int ecp;
    unsigned int cycleNr;
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

static refLeaf
monitor_typeExtent (
    c_type o,
    monitor_trc trace
    )
{
    refLeaf ted = NULL;

    ted = (refLeaf)ut_get (trace->extTree, o);
    if (ted) {
        if (ted->cycleNr != trace->cycleNr) {
            ted->ecp = ted->ec;
            ted->ec = o->objectCount;
            ted->cycleNr = trace->cycleNr;
        }
    } else {
        ted = malloc (C_SIZEOF(refLeaf));
        ted->tr = o;
        ted->ec = o->objectCount;
        ted->ecp = 0;
        ted->cycleNr = trace->cycleNr;
        ut_tableInsert(ut_table(trace->extTree), o, ted);
    }
    return ted;
}


static c_equality
compareLeafs (
    c_object o1,
    c_object o2,
    c_voidp args
    )
{
    if (o1 > o2) {
        return C_LT;
    } else if (o1 < o2) {
        return C_GT;
    }
    return C_EQ;
}

/*
 * Tree sorting algorithm: first sort by indicated sortKind, then by Object pointer.
 * The second sorting algorithm is required to prevent different types sharing the
 * same sizes from overwriting eachother.
 */
static c_equality
orderLeafs (
    c_object o1,
    c_object o2,
    c_voidp args
    )
{
    monitor_trc trace = (monitor_trc) args;
    long long n1, n2;

    /* First determine whether to sort deltas or current state. */
    if (trace->delta)
    {
        refLeaf ted1 = monitor_typeExtent (c_type(o1), trace);
        refLeaf ted2 = monitor_typeExtent (c_type(o2), trace);

        /* Sort deltas: now determine orderingKind. */
        switch (trace->oKind)
        {
            case ORDER_BY_COUNT:
                n1 = (long long) abs(ted1->ec - ted1->ecp);
                n2 = (long long) abs(ted2->ec - ted2->ecp);
                break;
            case ORDER_BY_SIZE:
                n1 = (long long) c_type(o1)->size;
                n2 = (long long) c_type(o2)->size;
                break;
            case ORDER_BY_TOTAL:
                n1 = (long long) abs((ted1->ec - ted1->ecp) * (long long) c_type(o1)->size);
                n2 = (long long) abs((ted2->ec - ted2->ecp) * (long long) c_type(o2)->size);
                break;
            default:
                assert(FALSE);
        }
    }
    else
    {
        /* Sort current state: now determine orderingKind. */
        switch (trace->oKind)
        {
            case ORDER_BY_COUNT:
                n1 = (long long) c_type(o1)->objectCount;
                n2 = (long long) c_type(o2)->objectCount;
                break;
            case ORDER_BY_SIZE:
                n1 = (long long) c_type(o1)->size;
                n2 = (long long) c_type(o2)->size;
                break;
            case ORDER_BY_TOTAL:
                n1 = (long long) c_type(o1)->objectCount * (long long) c_type(o1)->size;
                n2 = (long long) c_type(o2)->objectCount * (long long) c_type(o2)->size;
                break;
            default:
                assert(FALSE);
        }
    }

    if (n1 > n2) {
        return C_LT;
    } else if (n1 < n2) {
        return C_GT;
    }
    /* In case of equal size, then sort by object pointer to prevent different types from overwriting eachother. */
    if (o1 > o2) {
        return C_LT;
    } else if (o1 < o2) {
        return C_GT;
    }

    return C_EQ;
}

monitor_trc
monitor_trcNew (
    c_long objectCountLimit,
    const char*filterExpression,
    orderKind oKind,
    int orderCount,
    c_bool delta
    )
{
    monitor_trc o = malloc (C_SIZEOF(monitor_trc));

    if (o)
    {
       o->objectCountLimit = objectCountLimit;
       if (filterExpression)
       {
          o->filterExpression = os_strdup(filterExpression);
       }
       else
       {
          o->filterExpression = NULL;
       }
       o->extTree = ut_tableNew (compareLeafs, NULL);
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
            free (o->filterExpression);
        }
        ut_collectionFree (o->extTree, freeNode, NULL);
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
    } else {
        if (scope->name) {
            printf ("::%s", scope->name);
        }
    }
}

static void
metaobject (
    c_metaObject o,
    monitor_trc trace
    )
{
    refLeaf ted;

    if (c_baseObject(o)->kind < M_COUNT) {
        switch (c_baseObject(o)->kind) {
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
            case M_COLLECTION:
                switch (c_collectionType(o)->kind) {
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
                            ted = monitor_typeExtent (c_type(o), trace);
                            if (ted) {
                                if ((ted->ec - ted->ecp) >= trace->objectCountLimit) {
                                    if (trace->filterExpression) {
                                        if (strstr (o->name, trace->filterExpression) == NULL) {
                                            break;
                                        }
                                    }
                                    printf ("%6d (%8d) %8d %12lld %-15s %-15s ",
                                            ted->ec - ted->ecp,
                                            ted->ec,
                                            c_type(o)->size,
                                            (long long)(c_type(o)->size * (long long)((int)ted->ec - (int)ted->ecp)),
                                            baseKind[c_baseObject(o)->kind],
                                            collectionKind[c_collectionType(o)->kind]);
                                    printScope (o->definedIn);
                                    printf ("::%s\r\n", o->name);
                                    trace->totalCount += c_type(o)->size * (ted->ec - ted->ecp);
                                    trace->totalExtentCount += ted->ec - ted->ecp;
                                    trace->index++;
                                }
                            }
                        } else {
                            if (c_type(o)->objectCount >= trace->objectCountLimit) {
                                if (trace->filterExpression) {
                                    if (strstr (o->name, trace->filterExpression) == NULL) {
                                        break;
                                    }
                                }
                                printf ("%8d %8d %12lld %-15s %-15s ",
                                        c_type(o)->objectCount,
                                        c_type(o)->size,
                                        (long long)((long long)c_type(o)->size * c_type(o)->objectCount),
                                        baseKind[c_baseObject(o)->kind],
                                        collectionKind[c_collectionType(o)->kind]);
                                printScope (o->definedIn);
                                printf ("::%s\r\n", o->name);
                                trace->totalCount += c_type(o)->size * c_type(o)->objectCount;
                                trace->totalExtentCount += c_type(o)->objectCount;
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
                        if ((ted->ec - ted->ecp) >= trace->objectCountLimit) {
                            if (trace->filterExpression) {
                                if (strstr (o->name, trace->filterExpression) == NULL) {
                                    break;
                                }
                            }
                            printf ("%6d (%8d) %8d %12lld %-15s ",
                                    ted->ec - ted->ecp,
                                    ted->ec,
                                    c_type(o)->size,
                                    (long long)(c_type(o)->size * (long long)((int)ted->ec - (int)ted->ecp)),
                                    baseKind[c_baseObject(o)->kind]);
                            printf ("                ");
                            printScope (o->definedIn);
                            printf ("::%s\r\n", o->name);
                            trace->totalCount += c_type(o)->size * (ted->ec - ted->ecp);
                            trace->totalExtentCount += ted->ec - ted->ecp;
                            trace->index++;
                        }
                    }
                } else {
                    if (c_type(o)->objectCount >= trace->objectCountLimit) {
                        if (trace->filterExpression) {
                            if (strstr (o->name, trace->filterExpression) == NULL) {
                                break;
                            }
                        }
                        printf ("%8d %8d %12lld %-15s ",
                                c_type(o)->objectCount,
                                c_type(o)->size,
                                (long long)((long long)c_type(o)->size * c_type(o)->objectCount),
                                baseKind[c_baseObject(o)->kind]);
                        printf ("                ");
                        printScope (o->definedIn);
                        printf ("::%s\r\n", o->name);
                        trace->totalCount += c_type(o)->size * c_type(o)->objectCount;
                        trace->totalExtentCount += c_type(o)->objectCount;
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
    monitor_trc trace
    )
{
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
            ut_tableInsert(ut_table(trace->orderedList), o, o);
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
    v_entity entity,
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
    trace->totalCount = 0;
    trace->totalExtentCount = 0;
    trace->index = 0;
    if (trace->oKind == NO_ORDERING)
    {
        c_metaWalk (c_metaObject (base), metaobject, trace);
    }
    else
    {
        trace->orderedList = ut_tableNew (orderLeafs, trace);
        c_metaWalk (c_metaObject (base), orderedWalk, trace);
        ut_walk(trace->orderedList, orderedAction, trace);
        ut_tableFree(trace->orderedList, NULL, NULL, NULL, NULL);
        trace->orderedList = NULL;
    }
    printf ("\r\n");
    printf ("        %ld  for %ld object headers (%d) and MM headers (%d)\r\n",
        trace->totalExtentCount * (C_SIZEOF(c_header)  + MM_HEADER_SIZE),
        trace->totalExtentCount,
        C_SIZEOF(c_header), MM_HEADER_SIZE);
    totalSize = trace->totalCount + trace->totalExtentCount * (C_SIZEOF(c_header)  + MM_HEADER_SIZE);
    printf ("Total : %d  (%.2f KB)\r\n", totalSize, (double)totalSize/1024.0);
    trace->cycleNr++;
}
