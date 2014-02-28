/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2013 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE
 *
 *   for full copyright notice and license terms.
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


os_result
os_mmfAttrInit (
    os_mmfAttr *mmfAttr)
{
    assert (mmfAttr != NULL);
    mmfAttr->userCred.uid = 0;
    mmfAttr->userCred.gid = 0;
    mmfAttr->map_address = (void*)0x80000000;
    return os_resultSuccess;
}

