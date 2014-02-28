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

#include "c_base.h"
#include "v_topic.h"
#include "u_user.h"
#include "u_entity.h"
#include "u_instanceHandle.h"
#include "sd_serializer.h"
#include "sd_serializerXML.h"
#include "jni_writer.h"
#include "jni_publisher.h"
#include "jni_misc.h"
#include "jni_topic.h"
#include "os.h"
#include "os_report.h"
#include "os_heap.h"

static c_bool jni_writerCopy(c_type type, void *data, void *to);

C_CLASS(jni_writerCopyArg);

C_STRUCT(jni_writerCopyArg){
    jni_writer writer;
    const c_char* xmlData;
};

struct jni_writerTypeArg{
    c_type type;
};

static void jni_writerTypeAction(v_entity entity, c_voidp args);

static void
jni_writerTypeAction(
    v_entity entity, 
    c_voidp args)
{
    struct jni_writerTypeArg *arg;
    arg = (struct jni_writerTypeArg *)args;
    
    arg->type = NULL;
    
    switch(v_objectKind(entity)){
    case K_TOPIC:
        arg->type = v_topicDataType(entity);      
    break;
    default:
        OS_REPORT(OS_ERROR, "dcpsjni", 0, "Trying to resolve dataType of writer that is not a writer.\n");
        assert(FALSE);
    break;
    }
}

jni_writer
jni_writerNew(
    jni_publisher pub,
    jni_topic top,
    v_writerQos qos)
{
    jni_writer wri;
    u_result ur;
    struct jni_writerTypeArg arg;
    
    if((pub == NULL) || (top == NULL)){
        OS_REPORT_2(OS_ERROR, "jni_writerNew", 0,
                "Bad parameter; jni_publisher (%p) and jni_topic (%p) may not be NULL.",
                pub,
                top);
        goto err_badParam;
    }

    assert(pub->upublisher);
    assert(top->utopic);

    if((ur = u_entityAction(u_entity(top->utopic), jni_writerTypeAction, &arg)) != U_RESULT_OK){
        OS_REPORT_1(OS_ERROR, "jni_writerNew", ur,
                "Failed to invoke jni_writerTypeAction(...) on top->utopic; u_entityAction(...) returned %s.",
                u_resultImage(ur));
        goto err_getWriterType;
    }

    if(arg.type == NULL){
        /* Error reported by jni_writerTypeAction */
        goto err_getWriterType;
    }

    if((wri = os_malloc(sizeof *wri)) == NULL){
        OS_REPORT_1(OS_ERROR, "jni_writerNew", 0,
                "Memory claim of %" PA_PRIuSIZE " denied.",
                sizeof *wri);
        goto err_malloc;
    }

    if((wri->deserializer = sd_serializerXMLNewTyped(arg.type)) == NULL){
        /* Error reported by sd_serializerXMLNewTyped */
        goto err_sd_serializerXMLNewTyped;
    }

    if((wri->uwriter = u_writerNew(pub->upublisher, NULL, top->utopic, jni_writerCopy, qos, TRUE)) == NULL){
        /* Error reported by u_writerNew */
        goto err_uwriterNew;
    }

    wri->publisher = pub;
    wri->topic = top;

    return wri;

/* Error handling */
err_uwriterNew:
    sd_serializerFree(wri->deserializer);
err_sd_serializerXMLNewTyped:
    os_free(wri);
err_malloc:
/* No undo for jni_writerTypeAction */
err_getWriterType:
err_badParam:
    return NULL;
}

jni_result
jni_writerFree( 
    jni_writer wri)
{
    jni_result r;
    
    if(wri != NULL){
        r = jni_convertResult(u_writerFree(wri->uwriter));
        sd_serializerFree(wri->deserializer);
        os_free(wri);
    }
    else{
        r = JNI_RESULT_BAD_PARAMETER;
    }
    return r;
}

jni_result
jni_writerWrite(
    jni_writer wri,
    const c_char* xmlUserData)
{

    jni_result r;
    jni_writerCopyArg copyArg;
    sd_validationResult valResult;
    
    if( (wri == NULL) || (xmlUserData == NULL) || (wri->uwriter == NULL) || (xmlUserData == NULL)){
        r = JNI_RESULT_BAD_PARAMETER;
    } 
    else{
        copyArg = os_malloc(C_SIZEOF(jni_writerCopyArg));
        copyArg->writer = wri;
        copyArg->xmlData = xmlUserData;
        
        r = jni_convertResult(u_writerWrite(wri->uwriter,
                                            copyArg,
                                            u_timeGet(),
                                            U_INSTANCEHANDLE_NIL));
        valResult = sd_serializerLastValidationResult(wri->deserializer);
        
        if(valResult != SD_VAL_SUCCESS){
            OS_REPORT_2(OS_ERROR, CONT_DCPSJNI, 0, 
                        "Write of userdata failed.\nReason: %s\nError: %s\n",
                        sd_serializerLastValidationMessage(wri->deserializer),
                        sd_serializerLastValidationLocation(wri->deserializer));           
            r = JNI_RESULT_ERROR;
        }
        os_free(copyArg);
    }
    return r;
}

static c_bool
jni_writerCopy(
    c_type type, 
    void *data, 
    void *to)
{
    jni_writerCopyArg copyArg;
    sd_serializedData serData;
    
    OS_UNUSED_ARG(type);

    copyArg = (jni_writerCopyArg)data;
    serData = sd_serializerFromString(copyArg->writer->deserializer, copyArg->xmlData);
    sd_serializerDeserializeIntoValidated(copyArg->writer->deserializer, serData, to);
    sd_serializedDataFree(serData);
    
    return TRUE;
}
