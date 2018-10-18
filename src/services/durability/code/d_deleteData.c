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
#include "d_deleteData.h"
#include "d_message.h"
#include "d__misc.h"
#include "vortex_os.h"

d_deleteData
d_deleteDataNew(
    d_admin admin,
    os_timeW actionTime,
    const c_char* partitionExpr,
    const c_char* topicExpr)
{
    d_deleteData deleteData = NULL;

    if (admin) {
        deleteData = d_deleteData(os_malloc(C_SIZEOF(d_deleteData)));
        d_messageInit(d_message(deleteData), admin);
        d_timestampFromTimeW(&deleteData->actionTime, &actionTime, IS_Y2038READY(deleteData));

        if (partitionExpr) {
            deleteData->partitionExpr = os_strdup(partitionExpr);
        } else {
            deleteData->partitionExpr = NULL;
        }
        if (topicExpr) {
            deleteData->topicExpr = os_strdup(topicExpr);
        } else {
            deleteData->topicExpr = NULL;
        }
    }
    return deleteData;
}

void
d_deleteDataFree(
    d_deleteData deleteData)
{
    if(deleteData){
        d_messageDeinit(d_message(deleteData));

        if(deleteData->partitionExpr){
            os_free(deleteData->partitionExpr);
            deleteData->partitionExpr = NULL;
        }
        if(deleteData->topicExpr){
            os_free(deleteData->topicExpr);
            deleteData->topicExpr = NULL;
        }
        os_free(deleteData);
    }
}
