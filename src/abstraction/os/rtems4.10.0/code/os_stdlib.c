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
#define OS_HAS_STRTOK_R 1
#define MAXHOSTNAMELEN 256
#include "../common/code/os_gethostname.c"
#include "../common/code/os_stdlib.c"
#include "../common/code/os_stdlib_strtod.c"
#include "../common/code/os_stdlib_strtol.c"
#include "../common/code/os_stdlib_strtok_r.c"
