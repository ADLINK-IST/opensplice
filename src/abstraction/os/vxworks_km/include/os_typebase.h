/*
 *                         Vortex OpenSplice
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
 *
 *   Licensed under the ADLINK Software License Agreement Rev 2.7 2nd October
 *   2014 (the "License"); you may not use this file except in compliance with
 *   the License.
 *   You may obtain a copy of the License at:
 *                      $OSPL_HOME/LICENSE
 *
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */
#ifndef OS_TYPEBASE_H
#define OS_TYPEBASE_H

#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#if defined (__cplusplus)
extern "C" {
#endif

#define OS_STRUCT(name)  struct name##_s
#define OS_EXTENDS(type) OS_STRUCT(type) _parent
#define OS_CLASS(name)   typedef OS_STRUCT(name) *name
#define OS_SIZEOF(name)  sizeof(OS_STRUCT(name))
#define OS_SUPER(obj)    (&((obj)->_parent))

#define OS_ALIGNMENT       (8)

#if defined (__cplusplus)
}
#endif

#endif /* OS_TYPEBASE_H */

