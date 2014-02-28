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

/****************************************************************
 * Implementation for time management conforming to             *
 * OpenSplice requirements                                      *
 ****************************************************************/

#include "os_heap.h"
#include <string.h>
#include <unistd.h>
#include <stropts.h>

/* include OS specific socket management implementation		*/
#include "../common/code/os_socket.c"
