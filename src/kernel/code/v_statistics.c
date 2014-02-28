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

#include "v_statistics.h"
#include "os_report.h"
#include "v_kernelStatistics.h"
#include "v_writerStatistics.h"
#include "v_readerStatistics.h"
#include "v_queryStatistics.h"
#include "v_networkReaderStatistics.h"
#include "v_networkingStatistics.h"
#include "v_durabilityStatistics.h"
#include "v_cmsoapStatistics.h"
#include "v_rnrStatistics.h"
#include "v_groupQueueStatistics.h"
#include "v_time.h"
#include "os.h"

void
v_statisticsInit(
    v_statistics s)
{
    assert(C_TYPECHECK(s, v_statistics));

    s->lastReset.seconds     = 0;
    s->lastReset.nanoseconds = 0;

    return;
}
c_bool
v_statisticsResetField(
    v_statistics s,
    const c_char *fieldName)
{
    c_type type;
    c_value value;
    c_field field;
    c_bool result = TRUE;
    c_char * buf;
    c_char * subFieldName;
    c_char * fieldNameCopy;
    c_valueKind kind;
    c_bool isMin = FALSE;

    type = c_getType(c_object(s));
    fieldNameCopy = os_strdup(fieldName);

    subFieldName = strstr(fieldNameCopy, ".");
    if (subFieldName != NULL) {
        *subFieldName = '\0';
        subFieldName++; /* now points to subfield */
    } else {
        subFieldName = fieldNameCopy;
    }

    if (strcmp(subFieldName, "avg") == 0) {
        buf = os_malloc(strlen(fieldNameCopy)+1+5+1);
        if (buf) {
            os_sprintf(buf, "%s.count", fieldNameCopy);
            result = v_statisticsResetField(s, buf);
            os_free(buf);
        } else {
            result = FALSE;
        }
    } else {
        if (strcmp(subFieldName, "min") == 0) {
            isMin = TRUE;
        }
    }

    if (result == TRUE) {
        field = c_fieldNew(type , fieldName);
        kind = c_fieldValueKind(field);

        if(field != NULL){
            result = TRUE;

            switch(kind){
                case V_LONG:
                    if (isMin) {
                        value = c_longValue(0x7FFFFFFF);
                    } else {
                        value = c_longValue(0);
                    }
                    c_fieldAssign(field, s, value);
                    break;
                case V_ULONG:
                    value = c_ulongValue(0);
                    c_fieldAssign(field, s, value);
                    break;
                case V_LONGLONG:
                    value = c_longlongValue(0);
                    c_fieldAssign(field, s, value);
                    break;
                case V_ULONGLONG:
                    value = c_ulonglongValue(0);
                    c_fieldAssign(field, s, value);
                    break;
                case V_FLOAT:
                    value = c_floatValue(0.0);
                    c_fieldAssign(field, s, value);
                    break;

                default:
                    if(fieldName){
                        OS_REPORT_2(OS_ERROR,"Kernel", 0,
                                    "Value kind %d unsupported "
                                    "(fieldName: '%s')",
                                    kind, fieldName);
                    } else {
                        OS_REPORT_1(OS_ERROR,
                                    "Kernel", 0,"Value kind %d unsupported",
                                    kind);
                    }
                    result = FALSE;
                    break;
            }
            c_free(field);
        } else {
            result = FALSE;
        }
    }

    return result;
}

c_bool
v_statisticsResetAllFields(
    v_statistics s)
{
    OS_UNUSED_ARG(s);
    OS_REPORT(OS_INFO, "v_statisticsReset", 0,
              "Resetting of fields in unknown statistics class not supported");
    return FALSE;
}


c_bool
v_statisticsReset(
    v_statistics s,
    const c_char *fieldName)
{
    c_bool result = FALSE;
    c_type type;

    type = c_getType(c_object(s));

    if (fieldName != NULL) {
        result = v_statisticsResetField(s, fieldName);
    } else {
        c_char * typename = ((c_metaObject)type)->name;
        s->lastReset = v_timeGet();

        if (strcmp(typename, "v_kernelStatistics")==0 ) {
            result = v_kernelStatisticsReset(v_kernelStatistics(s), NULL);
        } else if (strcmp(typename, "v_writerStatistics")==0 ) {
            result = v_writerStatisticsReset(v_writerStatistics(s), NULL);
        } else if (strcmp(typename, "v_readerStatistics")==0 ) {
            result = v_readerStatisticsReset(v_readerStatistics(s), NULL);
        } else if (strcmp(typename, "v_queryStatistics")==0 ) {
            result = v_queryStatisticsReset(v_queryStatistics(s), NULL);
        } else if (strcmp(typename, "v_networkReaderStatistics")==0 ) {
            result = v_networkReaderStatisticsReset(v_networkReaderStatistics(s), NULL);
        } else if (strcmp(typename, "v_durabilityStatistics")==0 ) {
            result = v_durabilityStatisticsReset(v_durabilityStatistics(s), NULL);
        } else if (strcmp(typename, "v_cmsoapStatistics")==0 ) {
            result = v_cmsoapStatisticsReset(v_cmsoapStatistics(s), NULL);
        } else if (strcmp(typename, "v_networkingStatistics")==0 ) {
            result = v_networkingStatisticsReset(v_networkingStatistics(s), NULL);
        } else if (strcmp(typename, "v_rnrStatistics")==0 ) {
            result = v_rnrStatisticsReset(v_rnrStatistics(s), NULL);
        } else if (strcmp(typename, "v_groupQueueStatistics") ==0 ) {
            result = v_groupQueueStatisticsReset(v_groupQueueStatistics(s), NULL);
        } else {
            result = v_statisticsResetAllFields(s);
        }
    }
    return result;
}

