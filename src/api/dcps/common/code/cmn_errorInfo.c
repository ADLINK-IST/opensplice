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
#include <assert.h>
#include "cmn_errorInfo.h"
#include "v_kernel.h"
#include "ut_result.h"

os_int32
cmn_errorInfo_reportCodeToCode (
    os_int32 reportCode)
{
    os_int32 code;

    switch ((reportCode & OS_RETCODE_ID_MASK)) {
        case OS_RETCODE_ID_OS_RESULT:
            code = os_resultToReturnCode ((enum os_result) reportCode);
            break;
        case OS_RETCODE_ID_UT_RESULT:
            code = ut_resultToReturnCode ((enum ut_result_e) reportCode);
            break;
        case OS_RETCODE_ID_V_RESULT:
            code = v_resultToReturnCode ((v_result) reportCode);
            break;
        case OS_RETCODE_ID_V_WRITE_RESULT:
            code = v_writeResultToReturnCode ((v_writeResult) reportCode);
            break;
        case OS_RETCODE_ID_V_DATAREADER_RESULT:
            code = v_dataReaderResultToReturnCode ((v_dataReaderResult) reportCode);
            break;
        default:
            assert (!(reportCode & OS_RETCODE_ID_MASK));
            code = reportCode & ~OS_RETCODE_ID_MASK;
            break;
    }

    return code;
}
