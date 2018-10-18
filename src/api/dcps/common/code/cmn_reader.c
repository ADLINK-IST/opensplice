/*
 *                         Vortex OpenSplice
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
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
#include "cmn_reader.h"
#include "cmn_samplesList.h"

v_actionResult
cmn_reader_action(
    c_object o,
    void *arg)
{
    v_readerSample sample = v_readerSample(o);
    cmn_samplesList samplesList = cmn_samplesList(arg);
    v_actionResult result = V_PROCEED;

    if (sample != NULL) {
        if (!cmn_samplesList_insert(samplesList, sample)) {
            v_actionResultSet(result, V_SKIP);
            v_actionResultClear(result, V_PROCEED);
        }
        if (cmn_samplesList_full(samplesList) == TRUE) {
            v_actionResultClear(result, V_PROCEED);
        }
    } else {
        cmn_samplesList_finalize(samplesList);
        v_actionResultClear(result, V_PROCEED);
    }
    return result;
}

v_actionResult
cmn_reader_nextInstanceAction(
    c_object o,
    void *arg)
{
    v_readerSample sample = v_readerSample(o);
    cmn_samplesList samples = cmn_samplesList(arg);
    v_actionResult result = V_PROCEED;

    if (sample != NULL) {
        if (!cmn_samplesList_insert(samples, sample)) {
            v_actionResultSet(result, V_SKIP);
            v_actionResultClear(result, V_PROCEED);
        }
        if (cmn_samplesList_full(samples) == TRUE) {
            v_actionResultClear(result, V_PROCEED);
        }
    } else {
        if (cmn_samplesList_empty(samples) != TRUE) {
            cmn_samplesList_finalize(samples);
            v_actionResultClear(result, V_PROCEED);
        }
    }
    return result;
}

/*
 * TODO: when OSPL-3588 is finished, remove this function and replace
 * the calls with cmn_reader_nextInstanceAction.
 */
v_actionResult
cmn_reader_nextInstanceAction_OSPL3588(
    c_object o,
    void *arg)
{
    v_readerSample sample = v_readerSample(o);
    cmn_samplesList samples = cmn_samplesList(arg);
    v_actionResult result = V_PROCEED;

    if (sample != NULL) {
        if (!cmn_samplesList_insert(samples, sample)) {
            v_actionResultSet(result, V_SKIP);
            v_actionResultClear(result, V_PROCEED);
        }
        if (cmn_samplesList_full(samples) == TRUE) {
            v_actionResultClear(result, V_PROCEED);
        }
        if (cmn_samplesList_empty(samples) != TRUE) {
            v_actionResultClear(result, V_PROCEED);
        }
    } else {
        if (cmn_samplesList_empty(samples) != TRUE) {
            cmn_samplesList_finalize(samples);
        }
        v_actionResultClear(result, V_PROCEED);
    }
    return result;
}
