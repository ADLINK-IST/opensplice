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

/** \file os/AIX5.3/code/os_thread.c
 *  \brief AIX thread management
 *
 * Implements thread management for AIX
 * by including the POSIX implementation but with AIX-specific default values
 */

/* AIX doesn't seem to have sys/prctl.h */
#define OS_HAS_NO_SET_NAME_PRCTL

#include "../posix/code/os_thread.c"
#include "os_thread_attr.c"
