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
#include <dds__time.h>

dds_waitset_t dds_waitset_create (void)
{
    DDS_WaitSet ws;
    DDS_REPORT_STACK();
    ws = DDS_WaitSet__alloc();
    DDS_REPORT_FLUSH(NULL, ws == NULL);
    return (dds_waitset_t)ws;
}

void dds_waitset_get_conditions (dds_waitset_t ws, dds_condition_seq * seq)
{
    DDS_ReturnCode_t result = DDS_RETCODE_BAD_PARAMETER;
    DDS_REPORT_STACK();
    if (seq) {
        DDS_ConditionSeq cs = {0,0,NULL,FALSE};
        result = DDS_WaitSet_get_conditions((DDS_WaitSet)ws, &cs);
        if (result == DDS_RETCODE_OK) {
            seq->_buffer  = (dds_condition_t*)cs._buffer;
            seq->_length  = (uint32_t)cs._length;
            seq->_release = (bool)cs._release;
        } else {
            seq->_buffer  = NULL;
            seq->_length  = 0;
            seq->_release = false;
        }
    } else {
        DDS_REPORT(result, "seq == NULL");
    }
    DDS_REPORT_FLUSH(ws, result != DDS_RETCODE_OK);
}

int dds_waitset_delete (dds_waitset_t ws)
{
    DDS_ReturnCode_t result = DDS_RETCODE_BAD_PARAMETER;
    DDS_REPORT_STACK();
    if (ws) {
        DDS_free((DDS_WaitSet)ws);
        result = DDS_RETCODE_OK;
    } else {
        DDS_REPORT(result, "ws == NULL");
    }
    DDS_REPORT_FLUSH(NULL, result != DDS_RETCODE_OK);
    return DDS_ERRNO(result, DDS_MOD_WAITSET, DDS_ERR_Mx);
}

int dds_waitset_attach (dds_waitset_t ws, dds_condition_t e, dds_attach_t x)
{
    DDS_ReturnCode_t result;
    DDS_REPORT_STACK();
    result = DDS_WaitSet_attach_condition_alternative((DDS_WaitSet)ws,
                                                      (DDS_Condition)e,
                                                      (void*)x);
    DDS_REPORT_FLUSH(ws, result != DDS_RETCODE_OK);
    return DDS_ERRNO(result, DDS_MOD_WAITSET, DDS_ERR_Mx);
}

int dds_waitset_detach (dds_waitset_t ws, dds_condition_t e)
{
    DDS_ReturnCode_t result;
    DDS_REPORT_STACK();
    result = DDS_WaitSet_detach_condition((DDS_WaitSet)ws, (DDS_Condition)e);
    DDS_REPORT_FLUSH(ws, result != DDS_RETCODE_OK);
    return DDS_ERRNO(result, DDS_MOD_WAITSET, DDS_ERR_Mx);
}

int dds_waitset_wait (dds_waitset_t ws, dds_attach_t *xs, size_t nxs, dds_duration_t reltimeout)
{
    DDS_ReturnCode_t result = DDS_RETCODE_BAD_PARAMETER;
    int idx = DDS_ERRNO(result, DDS_MOD_WAITSET, DDS_ERR_Mx);

    DDS_REPORT_STACK();

    if (reltimeout >= 0) {
        DDS_ConditionSeq seq = {0,0,NULL,FALSE};
        DDS_Duration_t timeout = dds_duration_to_sac(reltimeout);
        result = DDS_WaitSet_wait((DDS_WaitSet)ws, &seq, &timeout);
        if (result == DDS_RETCODE_OK) {
            for (idx = 0; (idx < (int)seq._length) && (idx < (int)nxs); idx++) {
                xs[idx] = (dds_attach_t)seq._buffer[idx];
            }
            if ((nxs < (size_t)seq._length) && (nxs != 0)) {
                DDS_REPORT_WARNING("%d conditions triggered but only space for %d dds_attach_t elements",
                                   (int)seq._length, (int)nxs);
            }
        } else if (result == DDS_RETCODE_TIMEOUT) {
            /* A timeout is apparently not an error. */
            result = DDS_RETCODE_OK;
            idx = 0;
        }
        DDS_free(seq._buffer);
    } else {
        DDS_REPORT(result, "reltimeout < 0");
    }

    DDS_REPORT_FLUSH(ws, result != DDS_RETCODE_OK);

    return idx;
}

int dds_waitset_wait_until (dds_waitset_t ws, dds_attach_t *xs, size_t nxs, dds_time_t abstimeout)
{
    int idx;
    DDS_REPORT_STACK();
    dds_duration_t reltimeout = dds_delta_from_now(abstimeout);
    if (reltimeout >= 0) {
        idx = (DDS_ReturnCode_t)dds_waitset_wait(ws, xs, nxs, reltimeout);
    } else {
        idx = DDS_ERRNO(DDS_RETCODE_BAD_PARAMETER, DDS_MOD_WAITSET, DDS_ERR_Mx);
        DDS_REPORT(DDS_RETCODE_BAD_PARAMETER, "abstimeout in the past");

    }
    DDS_REPORT_FLUSH(ws, idx < 0);
    return idx;
}
