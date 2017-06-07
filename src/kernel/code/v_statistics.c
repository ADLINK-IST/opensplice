/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR PrismTech
 *   Limited, its affiliated companies and licensors. All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */

#include "v_statistics.h"
#include "os_report.h"
#include "vortex_os.h"

void
v_statisticsInit(
    v_statistics s)
{
    assert(C_TYPECHECK(s, v_statistics));

    s->lastReset = OS_TIMEW_ZERO;

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

    assert(s);
    assert(fieldName);
    assert(C_TYPECHECK(s, v_statistics));

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
        os_sprintf(buf, "%s.count", fieldNameCopy);
        result = v_statisticsResetField(s, buf);
        os_free(buf);
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
                        OS_REPORT(OS_ERROR,"Kernel", V_RESULT_ILL_PARAM,
                                    "Value kind %d unsupported "
                                    "(fieldName: '%s')",
                                    kind, fieldName);
                    } else {
                        OS_REPORT(OS_ERROR,"Kernel", V_RESULT_ILL_PARAM,
                                    "Value kind %d unsupported",
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
