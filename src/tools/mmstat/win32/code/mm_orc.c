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
#include "mm_metakindNames.h"
#include "u_user.h"
#include "c_base.h"
#include "c__base.h"
#include "ut_collection.h"
#include "os_errno.h"
#include "os_stdlib.h"
#include "c_module.h"

#include "mm_orc.h"

#include <sys/types.h>

#include <Windows.h>

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
   ut_table refTree;
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
      o->refTree = ut_tableNew ((const ut_compareElementsFunc)compareLeafs, NULL, NULL, NULL, freeNode, NULL);
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
   ord = (refLeaf)ut_get (ut_collection(trace->refTree), tr);
   if (ord) {
      ord->rc++;
   } else {
      ord = malloc (C_SIZEOF(refLeaf));
      ord->tr = tr;
      ord->rc = 1;
      ord->prc = 0;
      (void) ut_tableInsert(trace->refTree, tr, ord);
   }
}

void
monitor_orcFree (
                 monitor_orc o
                 )
{
   if (o->filterExpression) {
      os_free (o->filterExpression);
   }
   ut_tableFree(o->refTree);
   o->refTree = NULL;

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
         if ((c_ulong)abs(ord->rc - ord->prc) >= trace->extendCountLimit) {
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
               if ((c_ulong)abs(ord->rc - ord->prc) >= trace->extendCountLimit) {
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
         if (trace->delta) {
            if ((c_ulong)abs(ord->rc - ord->prc) >= trace->extendCountLimit) {
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
                   v_public entity,
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

   for (or = base->firstObject; or != base->lastObject; ) {
      or = c_header(or)->nextObject;
      monitor_object (or, trace);
      count++;
   };
   ut_walk (ut_collection(trace->refTree), display_orc, trace);
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
