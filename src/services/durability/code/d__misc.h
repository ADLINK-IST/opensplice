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

#ifndef D__MISC_H
#define D__MISC_H

#include "d__types.h"
#include "c_time.h"
#include "c_base.h"
#include "os.h"

#if defined (__cplusplus)
extern "C" {
#endif

struct baseFind {
    c_base base;
};

void                d_printState                (d_durability durability, 
                                                 d_configuration config, 
                                                 const char* threadName,
                                                 char* header);

void                d_findBaseAction            (v_entity entity,
                                                 c_voidp args);

#if defined (__cplusplus)
}
#endif

#endif /* D__MISC_H */
