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

/****************************************************************
 * Implementation for time management conforming to             *
 * OpenSplice requirements                                      *
 ****************************************************************/

/** \file os-net/qnx/code/os_socket.c
 *  \brief socket management
 */

#include "os_heap.h"
#include <string.h>

#define OS_NEEDS_SO_REUSEPORT

/* include OS specific socket management implementation		*/
#include "../common/code/os_socket.c"
