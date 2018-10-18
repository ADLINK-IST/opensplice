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

/** \file os/linux/code/os_memMapFile.c
 *  \brief Linux 2.6 memory mapped file management
 *
 * Implements memory mapped file management for Linux
 */

#include <assert.h>
#include "os_memMapFile.h"
#include "../posix/code/os_memMapFile.c"
#include "../common/code/os_memMapFile.c"


void
os_mmfAttrInit (
    os_mmfAttr *mmfAttr)
{
    assert (mmfAttr != NULL);
    mmfAttr->userCred.uid = 0;
    mmfAttr->userCred.gid = 0;
    mmfAttr->map_address = 0x80000000;
}
