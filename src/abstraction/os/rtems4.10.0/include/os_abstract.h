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

#ifndef PA_RTEMS_ABSTRACT_H
#define PA_RTEMS_ABSTRACT_H

#if defined (__cplusplus)
extern "C" {
#endif

/* include OS specific PLATFORM definition file */
#include <rtems/endian.h>

#if BYTE_ORDER == LITTLE_ENDIAN
#define PA__LITTLE_ENDIAN
#else
#define PA__BIG_ENDIAN
#endif

#define CONF_PARSER_NOFILESYS

#if defined (__cplusplus)
}
#endif

#endif /* PA_LINUX_ABSTRACT_H */
