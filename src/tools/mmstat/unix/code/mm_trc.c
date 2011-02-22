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
#include "u_user.h"
#include "c_base.h"
#include "c__base.h"
#include "c_avltree.h"
#include "ut_collection.h"
#include "os_abstract.h"
#include "os_stdlib.h"
#include "c_module.h"

#include "mm_trc.h"

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
    c_extent extent;
};
#endif
#endif

C_STRUCT(monitor_trc) {
    c_long objectCountLimit;
    char *filterExpression;
    regex_t expression;
    ut_collection extTree;
    long totalCount;
    long totalExtentCount;
    c_bool delta;
};

static const c_address HEADERSIZE = ALIGNSIZE(C_SIZEOF(c_header));

C_CLASS(refLeaf);
C_STRUCT(refLeaf) {
    c_type tr;
    unsigned int ec;
    unsigned int ecp;
};

static char *baseKind [] = {
    "M_UNDEFINED",
    "M_ATTRIBUTE",
    "M_CLASS",
    "M_COLLECTION",
    "M_CONSTANT",
    "M_CONSTOPERAND",
    "M_ENUMERATION",
    "M_EXCEPTION",
    "M_EXPRESSION",
    "M_INTERFACE",
    "M_LITERAL",
    "M_MEMBER",
    "M_MODULE",
    "M_OPERATION",
    "M_PARAMETER",
    "M_PRIMITIVE",
    "M_RELATION",
    "M_BASE",
    "M_STRUCTURE",
    "M_TYPEDEF",
    "M_UNION",
    "M_UNIONCASE",
    "M_COUNT"
};

static char *collectionKind [] = {
    "C_UNDEFINED",
    "C_LIST",
    "C_ARRAY",
    "C_BAG",
    "C_SET",
    "C_MAP",
    "C_DICTIONARY",
    "C_SEQUENCE",
    "C_STRING",
    "C_WSTRING",
    "C_QUERY",
    "C_SCOPE",
    "C_COUNT"
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

static c_equality
compareLeafs (
    c_object o1,
    c_object o2,
    c_voidp args
    )
{
    if (o1 < o2) {
	return C_LT;
    } else if (o1 > o2) {
	return C_GT;
    }
    return C_EQ;
}

monitor_trc
monitor_trcNew (
    c_long objectCountLimit,
    const char*filterExpression,
    c_bool delta
    )
{
    char expressionError [1024];
    monitor_trc o = malloc (C_SIZEOF(monitor_trc));

    if (o) {
	o->objectCountLimit = objectCountLimit;
    }
    if (filterExpression) {
	o->filterExpression = os_strdup(filterExpression);
    } else {
	o->filterExpression = NULL;
    }
    if (o->filterExpression) {
	if (regcomp (&o->expression, o->filterExpression, REG_EXTENDED) != 0) {
	    regerror (errno, &o->expression, expressionError, sizeof(expressionError));
	    printf ("Filter expression error: %s\r\n", expressionError);
	    regfree (&o->expression);
	    free (o->filterExpression);
	    o->filterExpression = NULL;
	}
    }
    o->extTree = ut_tableNew (compareLeafs, NULL);
    o->delta = delta;

    return o;
}

void
monitor_trcFree (
    monitor_trc o
    )
{
    if (o->filterExpression) {
	free (o->filterExpression);
	regfree (&o->expression);
    }
    ut_collectionFree (o->extTree, freeNode, NULL);
    free (o);
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

static refLeaf
monitor_typeExtent (
    c_type o,
    monitor_trc trace
    )
{
    refLeaf ted = NULL;

    ted = (refLeaf)ut_get (trace->extTree, o);
    if (ted) {
	ted->ecp = ted->ec;
	ted->ec = o->objectCount;
    } else {
	ted = malloc (C_SIZEOF(refLeaf));
	ted->tr = o;
	ted->ec = o->objectCount;
	ted->ecp = 0;
	ut_tableInsert(ut_table(trace->extTree), o, ted);
    }
    return ted;
}

static void
metaobject (
    c_metaObject o,
    monitor_trc trace
    )
{
    regmatch_t match[1];
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
                                if (regexec (&trace->expression, o->name, 1, match, 0) == REG_NOMATCH) {
                                    break;
                                }
                            }
                            printf ("%6d (%6d) %6d %10d %-15s %-15s ",
		                ted->ec - ted->ecp,
				ted->ec,
		                (int)c_type(o)->size,
		                (int)(c_type(o)->size * (ted->ec - ted->ecp)),
		                baseKind[c_baseObject(o)->kind],
		                collectionKind[c_collectionType(o)->kind]);
		            printScope (o->definedIn);
		            printf ("::%s\r\n", o->name);
		            trace->totalCount += c_type(o)->size * (ted->ec - ted->ecp);
		            trace->totalExtentCount += ted->ec - ted->ecp;
			}
		    }
		} else {
		    if (c_type(o)->objectCount >= trace->objectCountLimit) {
		        if (trace->filterExpression) {
			    if (regexec (&trace->expression, o->name, 1, match, 0) == REG_NOMATCH) {
			        break;
			    }
		        }
                        printf ("%6d %6d %10d %-15s %-15s ",
		            c_type(o)->objectCount,
		            (int)c_type(o)->size,
		            (int)(c_type(o)->size * c_type(o)->objectCount),
		            baseKind[c_baseObject(o)->kind],
		            collectionKind[c_collectionType(o)->kind]);
		        printScope (o->definedIn);
		        printf ("::%s\r\n", o->name);
		        trace->totalCount += c_type(o)->size * c_type(o)->objectCount;
		        trace->totalExtentCount += c_type(o)->objectCount;
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
    case M_EXTENT:
    case M_EXTENTSYNC:
	    if (trace->delta) {
		ted = monitor_typeExtent (c_type(o), trace);
		if (ted) {
		    if ((ted->ec - ted->ecp) >= trace->objectCountLimit) {
		        if (trace->filterExpression) {
                            if (regexec (&trace->expression, o->name, 1, match, 0) == REG_NOMATCH) {
                                break;
                            }
                        }
                        printf ("%6d (%6d) %6d %10d %-15s ",
		            ted->ec - ted->ecp,
			    ted->ec,
		            (int)c_type(o)->size,
		            (int)(c_type(o)->size * (ted->ec - ted->ecp)),
		            baseKind[c_baseObject(o)->kind]);
		        printf ("                ");
		        printScope (o->definedIn);
		        printf ("::%s\r\n", o->name);
		        trace->totalCount += c_type(o)->size * (ted->ec - ted->ecp);
		        trace->totalExtentCount += ted->ec - ted->ecp;
		    }
		}
	    } else {
	        if (c_type(o)->objectCount >= trace->objectCountLimit) {
		    if (trace->filterExpression) {
		        if (regexec (&trace->expression, o->name, 1, match, 0) == REG_NOMATCH) {
			    break;
		        }
		    }
                    printf ("%6d %6d %10d %-15s ",
		        c_type(o)->objectCount,
		        (int)c_type(o)->size,
		        (int)(c_type(o)->size * c_type(o)->objectCount),
		        baseKind[c_baseObject(o)->kind]);
		    printf ("                ");
	            printScope (o->definedIn);
	            printf ("::%s\r\n", o->name);
		    trace->totalCount += c_type(o)->size * c_type(o)->objectCount;
		    trace->totalExtentCount += c_type(o)->objectCount;
	        }
	    }
	    break;
	case M_MODULE:
	    c_metaWalk (o, metaobject, trace);
	}
    }
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
    int totalSize;

    time (&t);
    base = c_getBase(entity);
    printf ("\r\n################# Start tracing ################## %s\r\n", ctime(&t));
    printf ("Limit             : %d\r\n", trace->objectCountLimit);
    if (trace->filterExpression) {
	printf ("Filter expression : %s\r\n", trace->filterExpression);
    }
    printf ("\r\n");
    trace->totalCount = 0;
    trace->totalExtentCount = 0;
    c_metaWalk (c_metaObject (base), metaobject, trace);
    printf ("\r\n");
    printf ("        %ld  for %ld object headers ("PA_ADDRFMT") and MM headers (%d)\r\n",
        trace->totalExtentCount * (C_SIZEOF(c_header)  + MM_HEADER_SIZE),
        trace->totalExtentCount,
        C_SIZEOF(c_header), MM_HEADER_SIZE);
    totalSize = trace->totalCount + trace->totalExtentCount * (C_SIZEOF(c_header)  + MM_HEADER_SIZE);
    printf ("Total : %d  (%.2f KB)\r\n", totalSize, (double)totalSize/1024.0);
}
