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

#ifndef D__MISC_H
#define D__MISC_H

#include "d__types.h"
#include "c_time.h"
#include "c_base.h"
#include "vortex_os.h"

#if defined (__cplusplus)
extern "C" {
#endif

#define D_CONTEXT_DURABILITY    "Durability Service"
#define RR_MALLOC_FAILED        "Malloc failed for '%s'\n"

#define D_DDS_COMPLETENESS_UNKNOWN            0U
#define D_DDS_COMPLETENESS_INCOMPLETE         1U
#define D_DDS_COMPLETENESS_COMPLETE           2U

/* The following defines specify the error codes that may
 * be returned to a durability client as respons to a request
 * from that client.
 * Currently this list is defined both here and in the implementation
 * of the durability client. When this list of error codes is
 * changed the change should also be implemented in the durability
 * client.
 */
/* no error */
#define D_DDS_RETCDE_NO_ERROR                            0x0
/* Syntax errors  -- 1 - 8388607 (= 1 .. 2^23-1) */
#define D_DDS_RETCDE_INCOMPATIBLE_VERSION                0x1
#define D_DDS_RETCDE_NO_TOPIC                            0x2
#define D_DDS_RETCDE_INVALID_START_TIME                  0x3
#define D_DDS_RETCDE_INVALID_END_TIME                    0x4
#define D_DDS_RETCDE_INVALID_TIME_RANGE                  0x5
#define D_DDS_RETCDE_INVALID_SERIAZATION                 0x6
#define D_DDS_RETCDE_NO_PARTITIONS                       0x7
#define D_DDS_RETCDE_INVALID_MAX_SAMPLES                 0x8
#define D_DDS_RETCDE_INVALID_MAX_INSTANCES               0x9
#define D_DDS_RETCDE_INVALID_MAX_SAMPLES_PER_INSTANCE    0xA
/* Syntax OK, but server is not able process -- 8388608 - (= 2^23 - 2^24-1) - */
#define D_DDS_RETCDE_SERVER_IS_NOT_ALIGNER               0x80000000
#define D_DDS_RETCDE_SERVER_IS_NOT_RESPONSIBLE           0x80000001
#define D_DDS_RETCDE_READER_NOT_KNOWN                    0x80000002
#define D_DDS_RETCDE_NO_MASTER_SELECTED                  0x80000003
#define D_DDS_RETCDE_GROUP_NOT_FOUND                     0x80000004
#define D_DDS_RETCDE_NO_CLIENTS                          0x80000005
#define D_DDS_RETCDE_INVALID                             0xFFFFFFFF

struct error {
    c_ulong errorCode;
    char *errorMessage;
};

extern const char *d_builtinTopics[];
extern const struct error Error[];

/* Internal quality is mapped as os_timeW */
typedef os_timeW d_quality;
#define D_QUALITY_ZERO          OS_TIMEW_ZERO
#define D_QUALITY_INFINITE      OS_TIMEW_INFINITE
#define D_QUALITY_ISZERO(q)     (OS_TIMEW_ISZERO(q))
#define D_QUALITY_ISINFINITE(q) (OS_TIMEW_ISINFINITE(q))

struct baseFind {
    c_base base;
};


void                d_free                      (c_voidp allocated );

c_voidp             d_malloc                    (size_t size,
                                                 const char * errorText);

void                d_printTimedEvent           (d_durability durability,
                                                 d_level level,
                                                 const char * eventText,
                                                 ...);

void                d_printEvent                (d_durability durability,
                                                 d_level level,
                                                 const char * eventText,
                                                 ...);

void                d_reportLocalGroup          (_In_ d_durability d,
                                                 _In_ _Const_ v_group group);

c_base              d_findBase                  (d_durability durability);

c_bool              d_patternMatch              (const char* str,
                                                 const char* pattern);

c_bool              d_isBuiltinGroup            (_In_z_ _Const_ d_partition partition,
                                                 _In_z_ _Const_ d_topic topic);

c_bool              d_isHeartbeatGroup          (_In_z_ _Const_ d_partition partition,
                                                 _In_z_ _Const_ d_topic topic);

void                d_printState                (d_durability durability,
                                                 d_configuration config,
                                                 const char* threadName,
                                                 char* header);

const char *        d_inBuiltinTopicNames       (const char* str,
                                                 size_t len);

c_bool              d_shmAllocAssert            (c_voidp ptr,
                                                 const char* errorMessage);

c_bool              d_shmAllocAllowed            (void);

_DDS_Completeness_t d_mapCompleteness            (d_completeness completeness);

char *              d_getErrorMessage            (c_ulong errorCode);

unsigned short      d_swap2uToBE                 (unsigned short x);

unsigned            d_swap4uToBE                 (unsigned x);

unsigned long long  d_swap8uToBE                 (unsigned long long x);

#define             d_swap2uFromBE               d_swap2uToBE
#define             d_swap4uFromBE               d_swap4uToBE
#define             d_swap8uFromBE               d_swap8uToBE

c_bool              sequenceOfStringCopyOut      (c_iter *to,
                                                  const c_sequence from);

d_durabilityKind    d_durabilityKindFromKernel   (v_durabilityKind kind);

const char         *d_compressionKVImage         (d_compressionKV compression);

void                d_trace                      (c_ulong mask, const char *fmt, ...);

void                d_tracegroupGenKeystr        (char *keystr, size_t keystr_size, v_groupInstance gi);

void                d_tracegroupGenMsgKeystr     (char *keystr, size_t keystr_size, v_group g, v_message msg);

void                d_tracegroupInstance         (v_groupInstance gi, d_durability dur, const char *prefix);

void                d_tracegroup                 (d_durability dur, v_group g, const char *info);

void                d_timestampFromTimeW         (d_timestamp *t1, os_timeW *t2, c_bool Y2038Ready);

void                d_timestampToTimeW           (os_timeW *t1, d_timestamp *t2, c_bool Y2038Ready);

void                d_qualityExtFromQuality      (d_qualityExt *q1, d_quality *q2, c_bool Y2038Ready);

void                d_qualityExtToQuality        (d_quality *q1, d_qualityExt *q2, c_bool Y2038Ready);

os_compare          d_qualityCompare             (d_quality q1, d_quality q2);

void                d_productionTimestampToSeqnum(os_uint32 *seqnum, d_timestamp *t);

void                d_productionTimestampFromSeqnum(d_timestamp *t, os_uint32 *seqnum);

os_timeE            d_timeWToTimeE               (os_timeW t);

void d_traceMergeAction (d_mergeAction mergeAction, const char *info);

#if defined (__cplusplus)
}
#endif

#endif /* D__MISC_H */
