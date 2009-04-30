/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2009 PrismTech 
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE 
 *
 *   for full copyright notice and license terms. 
 *
 */
#include <u_user.h>
#include <c_base.h>
#include <c__base.h>

#include "mm_ms.h"

#include <sys/time.h>

C_STRUCT(monitor_ms) {
    c_long header;
    c_bool extendedMode;
    c_bool rawMode;
    c_bool delta;
    struct c_mmStatus_s prevState;
};

monitor_ms
monitor_msNew (
    c_bool extendedMode,
    c_bool rawMode,
    c_bool delta
    )
{
    monitor_ms o = malloc (C_SIZEOF(monitor_ms));

    o->header = 0;
    o->extendedMode = extendedMode;
    o->rawMode = rawMode;
    o->delta = delta;
    memset (&o->prevState, 0, sizeof (o->prevState));
    return o;
}

void
monitor_msFree (
    monitor_ms o
    )
{
    free (o);
}

void
monitor_msAction (
    v_entity entity,
    c_voidp args
    )
{
    time_t ct;
    int cv;
    char timbuf[30];
    char extra[70];
    c_mm mm;
    c_mmStatus s;
    c_base base;
    monitor_ms msData;
    
    msData = monitor_ms(args);
    base = c_getBase(entity);
    mm = c_baseMM(base);
    s = c_mmState(mm);
    
    if (msData->rawMode == FALSE) {
        time (&ct);

        if (msData->header == 0) {
            strftime (timbuf, sizeof(timbuf), "%d/%m/%Y", localtime(&ct));
	    if (msData->delta) {
                printf ("%-12s%11s%11s%11s%11s%11s%11s\r\n",
                    timbuf, "D-size", "D-used", "D-maxUsed", "D-fails", "D-garbage", "D-count");
                msData->header = 1;
	    } else {
                printf ("%-12s%11s%11s%11s%11s%11s%11s\r\n",
                    timbuf, "size", "used", "maxUsed", "fails", "garbage", "count");
                msData->header = 15;
	    }
	}
        strftime (timbuf, sizeof(timbuf), "%H:%M:%S", localtime(&ct));
    } else {
        /* no headers and print time as seconds since start of mmstat */
        os_time now = os_timeGet();
        sprintf(timbuf,"%d.%9.9d ",now.tv_sec,now.tv_nsec);
    }
            
    if (msData->extendedMode) {
        cv = ((s.used * 40)/s.size);
        strncpy (extra, "  |                                        |\r\n", 
                 sizeof(extra));
        extra [cv+3] = '*';
    } else {
        strncpy (extra, "\r\n", sizeof(extra));
    }
    if (msData->delta) {
        printf("%-12s%11d%11d%11d%11d%11d%11d\n\r",
            timbuf,
	    s.size - msData->prevState.size,
	    s.used - msData->prevState.used,
	    s.maxUsed - msData->prevState.maxUsed, 
            s.fails - msData->prevState.fails,
	    s.garbage - msData->prevState.garbage,
	    s.count - msData->prevState.count);
            msData->prevState = s;
    } else {
        printf("%-12s%11d%11d%11d%11d%11d%11d%s",
            timbuf, s.size, s.used, s.maxUsed, 
            s.fails, s.garbage, s.count, extra);
    }
    msData->header--;
}
