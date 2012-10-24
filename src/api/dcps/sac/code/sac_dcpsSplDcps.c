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

#include "dds_dcpsSplDcps.h"
#include "dds_dcps.h"

c_bool
__DDS_Duration_t__copyIn(c_base base, void *_from, void *_to)
{
    DDS_Duration_t *from = (DDS_Duration_t *)_from;
    struct _DDS_Duration_t *to = (struct _DDS_Duration_t *)_to;
    (void)base; /* avoid warning */
    to->sec = (c_long)from->sec;
    to->nanosec = (c_ulong)from->nanosec;
    
    return TRUE;
}

c_bool
__DDS_Time_t__copyIn(c_base base, void *_from, void *_to)
{
    DDS_Time_t *from = (DDS_Time_t *)_from;
    struct _DDS_Time_t *to = (struct _DDS_Time_t *)_to;
    (void)base; /* avoid warning */
    to->sec = (c_long)from->sec;
    to->nanosec = (c_ulong)from->nanosec;
    
    return TRUE;
}

void
__DDS_Duration_t__copyOut(void *_from, void *_to)
{
    struct _DDS_Duration_t *from = (struct _DDS_Duration_t *)_from;
    DDS_Duration_t *to = (DDS_Duration_t *)_to;
    to->sec = (DDS_long)from->sec;
    to->nanosec = (DDS_unsigned_long)from->nanosec;
    
    return;
}

void
__DDS_Time_t__copyOut(void *_from, void *_to)
{
    struct _DDS_Time_t *from = (struct _DDS_Time_t *)_from;
    DDS_Time_t *to = (DDS_Time_t *)_to;
    to->sec = (DDS_long)from->sec;
    to->nanosec = (DDS_unsigned_long)from->nanosec;
    
    return;
}
