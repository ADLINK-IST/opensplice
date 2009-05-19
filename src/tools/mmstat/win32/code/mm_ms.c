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
#include <os_time.h>

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

static void
to_string(
    c_long val,
    char *str) 
{
    c_ulong unit = 1000000000;
    c_bool first = TRUE;
    c_bool proceed = TRUE;
    c_long x;
    c_char _x[32];

    str[0] = 0;
    while (proceed) {
        x = val / unit;
        if (x) {
            if (!first) {
                if (x>99) {
                    sprintf(_x,".%d",x);
                } else if (x>9) {
                    sprintf(_x,".0%d",x);
                } else {
                    sprintf(_x,".00%d",x);
                }
            } else {
                sprintf(_x,"%d",x);
            }
            strcat(str,_x);
            first = FALSE;
        } else if (!first) {
            sprintf(_x,".000");
            strcat(str,_x);
        }
        val = (val - x * unit);
        if (unit > 1) {
            unit = unit / 1000;
        } else {
            if (first) {
                strcat(str,"0");
            }
            proceed = FALSE;
        }
    }
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
    c_mmStatus ms,ls;
    c_base base;
    monitor_ms msData;
    
    msData = monitor_ms(args);
    base = c_getBase(entity);
    mm = c_baseMM(base);
    s = c_mmState(mm);
    ms = c_mmMapState(mm);
    ls = c_mmListState(mm);
    
    if (msData->rawMode == FALSE) {
        time (&ct);

        if (msData->header == 0) {
            strftime (timbuf, sizeof(timbuf), "%d/%m/%Y", localtime(&ct));
	    if (msData->delta) {
                printf ("%-12s%11s%11s%11s%11s%11s%11s\r\n",
                    timbuf, "D-size", "D-used", "D-maxUsed", "D-fails", "D-garbage", "D-count");
                msData->header = 1;
	    } else {
                printf ("%-12s%14s%11s%14s%14s%14s%11s\r\n",
                    timbuf, "available", "count", "used", "maxUsed", "reusable", "fails");
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
        char _size[32], _used[32], _maxUsed[32], _reusable[32];

#if 0
        to_string(ms.size,_size);
        to_string(ms.used,_used);
        to_string(ms.maxUsed,_maxUsed);
        to_string(ms.garbage,_reusable);

        printf("MAP  %-12s%14s%11d%14s%14s%14s%11d%s",
            timbuf, _size, ms.count, _used, 
            _maxUsed, _reusable, ms.fails, extra);

        to_string(ls.size,_size);
        to_string(ls.used,_used);
        to_string(ls.maxUsed,_maxUsed);
        to_string(ls.garbage,_reusable);

        printf("LIST %-12s%14s%11d%14s%14s%14s%11d%s",
            timbuf, _size, ls.count, _used, 
            _maxUsed, _reusable, ls.fails, extra);
#else
        to_string(ls.size + ls.garbage + ms.garbage, _size);
        to_string(ls.used + ms.used, _used);
        to_string(ls.maxUsed + ms.maxUsed, _maxUsed);
        to_string(ls.garbage + ms.garbage, _reusable);

        printf("%-12s%14s%11d%14s%14s%14s%11d%s",
            timbuf, _size, ls.count + ms.count, _used, 
            _maxUsed, _reusable, ls.fails + ms.fails, extra);
#endif
    }
    msData->header--;
}
