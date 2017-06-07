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

/* SAC */
#include <dds_dcps.h>
#include <dds_dcps_private.h>

/* C99 */
#include <dds.h>
#include <dds_report.h>


dds_condition_t
dds_guardcondition_create (void)
{
    DDS_GuardCondition cond;
    DDS_REPORT_STACK();
    cond = DDS_GuardCondition__alloc();
    DDS_REPORT_FLUSH(NULL, cond == NULL);
    return (dds_condition_t)cond;
}

void
dds_condition_delete (dds_condition_t cond)
{
    DDS_ReturnCode_t result = DDS_RETCODE_BAD_PARAMETER;
    DDS_REPORT_STACK();
    if (cond) {
        switch (DDS_Condition_get_kind((DDS_Condition)cond)) {
            case DDS_CONDITION_KIND_STATUS: {
                /* Do nothing. */
                result = DDS_RETCODE_OK;
                break;
            }
            case DDS_CONDITION_KIND_GUARD: {
                DDS_free(cond);
                result = DDS_RETCODE_OK;
                break;
            }
            case DDS_CONDITION_KIND_READ:
            case DDS_CONDITION_KIND_QUERY: {
                DDS_ReadCondition rc = (DDS_ReadCondition)cond;
                DDS_DataReader rdr = DDS_ReadCondition_get_datareader(rc);
                if (rdr != NULL) {
                    result = DDS_DataReader_delete_readcondition(rdr, rc);
                } else {
                    result = dds_report_get_error_code();
                    DDS_REPORT(result, "Could not acquire reader");
                }
                break;
            }
            default: {
                DDS_REPORT(result, "Not a proper condition");
                break;
            }
        }
    } else {
        DDS_REPORT(result, "cond == NULL");
    }
    DDS_REPORT_FLUSH(NULL, result != DDS_RETCODE_OK);
}

void
dds_guard_trigger (dds_condition_t guard)
{
    DDS_ReturnCode_t result;
    DDS_REPORT_STACK();
    result = DDS_GuardCondition_set_trigger_value(guard, TRUE);
    DDS_REPORT_FLUSH(NULL, result != DDS_RETCODE_OK);
}

void
dds_guard_reset (dds_condition_t guard)
{
    DDS_ReturnCode_t result;
    DDS_REPORT_STACK();
    result = DDS_GuardCondition_set_trigger_value(guard, FALSE);
    DDS_REPORT_FLUSH(NULL, result != DDS_RETCODE_OK);
}

bool
dds_condition_triggered (dds_condition_t guard)
{
    return (bool)DDS_Condition_get_trigger_value(guard);
}
