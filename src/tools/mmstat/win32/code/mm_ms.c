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

#define MM_MS_TIME_BUF_HDR_FMT          "%-12s"
#define MM_MS_TIME_BUF_FMT              "%-12s"
#define MM_MS_AVAILABLE_HDR             "available"
#define MM_MS_AVAILABLE_HDR_FMT         "%14s"
#define MM_MS_AVAILABLE_FMT             "%14s"
#define MM_MS_COUNT_HDR                 "count"
#define MM_MS_COUNT_HDR_FMT             "%11s"
#define MM_MS_COUNT_FMT                 "%11d"
#define MM_MS_USED_HDR                  "used"
#define MM_MS_USED_HDR_FMT              "%14s"
#define MM_MS_USED_FMT                  "%14s"
#define MM_MS_PREALLOCATED_HDR          "preallocated"
#define MM_MS_PREALLOCATED_HDR_FMT      "%14s"
#define MM_MS_PREALLOCATED_FMT          "%14s"
#define MM_MS_MAXUSED_HDR               "maxUsed"
#define MM_MS_MAXUSED_HDR_FMT           "%14s"
#define MM_MS_MAXUSED_FMT               "%14s"
#define MM_MS_REUSABLE_HDR              "reusable"
#define MM_MS_REUSABLE_HDR_FMT          "%14s"
#define MM_MS_REUSABLE_FMT              "%14s"
#define MM_MS_FAILS_HDR                 "fails"
#define MM_MS_FAILS_HDR_FMT             "%11s"
#define MM_MS_FAILS_FMT                 "%11d"
#define MM_MS_EXTRA_FMT                 "%s"

#define MM_MS_D_FMT_WIDTH               "%11"
#define MM_MS_D_AVAILABLE_HDR           "D-avail"
#define MM_MS_D_AVAILABLE_HDR_FMT       MM_MS_D_FMT_WIDTH "s"
#define MM_MS_D_AVAILABLE_FMT           MM_MS_D_FMT_WIDTH "d"
#define MM_MS_D_USED_HDR                "D-used"
#define MM_MS_D_USED_HDR_FMT            MM_MS_D_FMT_WIDTH "s"
#define MM_MS_D_USED_FMT                MM_MS_D_FMT_WIDTH "d"
#define MM_MS_D_PREALLOCATED_HDR        "D-prealloc"
#define MM_MS_D_PREALLOCATED_HDR_FMT    MM_MS_D_FMT_WIDTH "s"
#define MM_MS_D_PREALLOCATED_FMT        MM_MS_D_FMT_WIDTH "d"
#define MM_MS_D_MAXUSED_HDR             "D-maxUsed"
#define MM_MS_D_MAXUSED_HDR_FMT         MM_MS_D_FMT_WIDTH "s"
#define MM_MS_D_MAXUSED_FMT             MM_MS_D_FMT_WIDTH "d"
#define MM_MS_D_FAILS_HDR               "D-fails"
#define MM_MS_D_FAILS_HDR_FMT           MM_MS_D_FMT_WIDTH "s"
#define MM_MS_D_FAILS_FMT               MM_MS_D_FMT_WIDTH "d"
#define MM_MS_D_REUSABLE_HDR            "D-reusable"
#define MM_MS_D_REUSABLE_HDR_FMT        MM_MS_D_FMT_WIDTH "s"
#define MM_MS_D_REUSABLE_FMT            MM_MS_D_FMT_WIDTH "d"
#define MM_MS_D_COUNT_HDR               "D-count"
#define MM_MS_D_COUNT_HDR_FMT           MM_MS_D_FMT_WIDTH "s"
#define MM_MS_D_COUNT_FMT               MM_MS_D_FMT_WIDTH "d"

#define MM_MS_NEWLINE                   "\r\n"

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
                printf (MM_MS_TIME_BUF_FMT
                        MM_MS_D_AVAILABLE_HDR_FMT
                        MM_MS_D_COUNT_HDR_FMT
                        MM_MS_D_USED_HDR_FMT
                        MM_MS_D_PREALLOCATED_HDR_FMT
                        MM_MS_D_MAXUSED_HDR_FMT
                        MM_MS_D_REUSABLE_HDR_FMT
                        MM_MS_D_FAILS_HDR_FMT
                        MM_MS_NEWLINE
                        ,
                        timbuf,
                        MM_MS_D_AVAILABLE_HDR,
                        MM_MS_D_COUNT_HDR,
                        MM_MS_D_USED_HDR,
                        MM_MS_D_PREALLOCATED_HDR,
                        MM_MS_D_MAXUSED_HDR,
                        MM_MS_D_REUSABLE_HDR,
                        MM_MS_D_FAILS_HDR);

                msData->header = 1;
	    } else {
                printf (MM_MS_TIME_BUF_HDR_FMT
                        MM_MS_AVAILABLE_HDR_FMT
                        MM_MS_COUNT_HDR_FMT
                        MM_MS_USED_HDR_FMT
                        MM_MS_PREALLOCATED_HDR_FMT
                        MM_MS_MAXUSED_HDR_FMT
                        MM_MS_REUSABLE_HDR_FMT
                        MM_MS_FAILS_HDR_FMT
                        MM_MS_NEWLINE
                        ,
                        timbuf,
                        MM_MS_AVAILABLE_HDR,
                        MM_MS_COUNT_HDR,
                        MM_MS_USED_HDR,
                        MM_MS_PREALLOCATED_HDR,
                        MM_MS_MAXUSED_HDR,
                        MM_MS_REUSABLE_HDR,
                        MM_MS_FAILS_HDR);
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
        printf (MM_MS_TIME_BUF_FMT
                MM_MS_D_AVAILABLE_FMT
                MM_MS_D_COUNT_FMT
                MM_MS_D_USED_FMT
                MM_MS_D_PREALLOCATED_FMT
                MM_MS_D_MAXUSED_FMT
                MM_MS_D_REUSABLE_FMT
                MM_MS_D_FAILS_FMT
                MM_MS_NEWLINE
                ,
                timbuf,
                s.size - msData->prevState.size,
                s.count - msData->prevState.count,
                s.used - msData->prevState.used,
                s.preallocated - msData->prevState.preallocated,
                s.maxUsed - msData->prevState.maxUsed,
                s.garbage - msData->prevState.garbage,
                s.fails - msData->prevState.fails);

        msData->prevState = s;
    } else {
        char _size[32], _used[32], _preallocated[32], _maxUsed[32], _reusable[32];

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
        to_string(s.preallocated, _preallocated);
        to_string(ls.maxUsed + ms.maxUsed, _maxUsed);
        to_string(ls.garbage + ms.garbage, _reusable);

        printf(MM_MS_TIME_BUF_FMT
               MM_MS_AVAILABLE_FMT
               MM_MS_COUNT_FMT
               MM_MS_USED_FMT
               MM_MS_PREALLOCATED_FMT
               MM_MS_MAXUSED_FMT
               MM_MS_REUSABLE_FMT
               MM_MS_FAILS_FMT
               MM_MS_EXTRA_FMT
               ,
               timbuf,
               _size,
               ls.count + ms.count,
               _used,
               _preallocated,
               _maxUsed,
               _reusable,
               ls.fails + ms.fails,
               extra);
#endif
    }
    msData->header--;
}
