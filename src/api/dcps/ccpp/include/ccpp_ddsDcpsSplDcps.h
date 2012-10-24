/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2011 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE 
 *
 *   for full copyright notice and license terms. 
 *
 */
#ifndef DDS_DCPSSPLTYPES_H
#define DDS_DCPSSPLTYPES_H

#include "c_base.h"
#include "c_misc.h"
#include "c_sync.h"
#include "c_collection.h"
#include "c_field.h"
#include "ccpp_dcps_if.h"
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

struct _DDS_Duration_t {
    c_long sec;
    c_ulong nanosec;
};

struct _DDS_Time_t {
    c_long sec;
    c_ulong nanosec;
};

#endif
