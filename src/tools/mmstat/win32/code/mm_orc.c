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
#include "c__extent.h"
#include "c_avltree.h"
#include "ut_collection.h"
#include "os_stdlib.h"
#include "c_module.h"

#include "mm_orc.h"

#include <sys/types.h>
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
   ut_collection refTree;
   int totalObjectCount;
   int totalSizeCount;
   c_bool delta;
};

static const c_long HEADERSIZE = ALIGNSIZE(C_SIZEOF(c_header));

C_CLASS(refLeaf);
C_STRUCT(refLeaf) {
   c_type tr;
   unsigned int rc;
   unsigned int prc;
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

monitor_orc
monitor_orcNew (
                c_long extendCountLimit,
                const char*filterExpression,
                c_bool delta
                )
{
   monitor_orc o = malloc (C_SIZEOF(monitor_orc));

   if (o) {
      o->extendCountLimit = extendCountLimit;
   }
   if (filterExpression) {
      o->filterExpression = os_strdup(filterExpression);
   } else {
      o->filterExpression = NULL;
   }
   o->refTree = ut_tableNew (compareLeafs, NULL);
   o->totalSizeCount = 0;
   o->totalObjectCount = 0;
   o->delta = delta;

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
                     if (strstr (c_metaObject(ord->tr)->name, trace->filterExpression) == NULL) {
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
                  trace->totalSizeCount += c_type(ord->tr)->size * (ord->rc - ord->prc);
                  trace->totalObjectCount += ord->rc - ord->prc;
	            }
            } else {
               if (ord->rc >= trace->extendCountLimit) {
                  if (trace->filterExpression) {
                     if (strstr (c_metaObject(ord->tr)->name, trace->filterExpression) == NULL) {
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
      case M_EXTENT:
      case M_EXTENTSYNC:
         if (trace->delta) {
            if (abs(ord->rc - ord->prc) >= trace->extendCountLimit) {
               if (trace->filterExpression) {
                  if (strstr (c_metaObject(ord->tr)->name, trace->filterExpression) == NULL) {
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
               trace->totalSizeCount += c_type(ord->tr)->size * (ord->rc - ord->prc);
               trace->totalObjectCount += ord->rc - ord->prc;
            }
         } else {
            if (ord->rc >= trace->extendCountLimit) {
               if (trace->filterExpression) {
                  if (strstr (c_metaObject(ord->tr)->name, trace->filterExpression) == NULL) {
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
               trace->totalSizeCount += c_type(ord->tr)->size * ord->rc;
               trace->totalObjectCount += ord->rc;
            }
         }
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
   int totalSize;

   time (&t);
   base = c_getBase(entity);
   printf ("\r\n################# Start tracing ################## %s\r\n", ctime(&t));
   printf ("Limit             : %d\r\n", trace->extendCountLimit);
   if (trace->filterExpression) {
      printf ("Filter expression : %s\r\n", trace->filterExpression);
   }
   printf ("\r\n");

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
   trace->totalSizeCount = 0;
   trace->totalObjectCount = 0;
#endif
}
