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

#ifndef PA_PIKEOS_ABSTRACT_H
#define PA_PIKEOS_ABSTRACT_H

#if _BYTE_ORDER == _LITTLE_ENDIAN
#define PA__LITTLE_ENDIAN
#else
#define PA__BIG_ENDIAN
#endif

#ifdef __x86_64__
#define PA__64BIT
#endif

#define CONF_PARSER_NOFILESYS

#endif /* PA_PIKEOS_ABSTRACT_H */
