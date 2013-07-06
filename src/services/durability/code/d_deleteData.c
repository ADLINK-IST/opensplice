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
#include "d_deleteData.h"
#include "d_message.h"
#include "os.h"

d_deleteData
d_deleteDataNew(
    d_admin admin,
    d_timestamp actionTime,
    const c_char* partitionExpr,
    const c_char* topicExpr)
{
    d_deleteData deleteData = NULL;
    
    if(admin){
        deleteData = d_deleteData(os_malloc(C_SIZEOF(d_deleteData)));
        d_messageInit(d_message(deleteData), admin);
        deleteData->actionTime.seconds     = actionTime.seconds;
        deleteData->actionTime.nanoseconds = actionTime.nanoseconds;
        
        if(partitionExpr){
            deleteData->partitionExpr = os_strdup(partitionExpr);
        } else {
            deleteData->partitionExpr = NULL;
        }
        if(topicExpr){
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
