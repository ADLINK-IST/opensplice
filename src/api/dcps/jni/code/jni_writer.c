/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2011 PrismTech
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
    u_writer uw;
    u_result ur;
    struct jni_writerTypeArg arg;
    
    wri = NULL;
    
    if((pub != NULL) && (top != NULL)){    
        ur = u_entityAction(u_entity(top->utopic), jni_writerTypeAction, &arg);
        
        if ((ur == U_RESULT_OK) && (arg.type != NULL)){
            uw = u_writerNew(pub->upublisher, NULL, top->utopic, jni_writerCopy, qos, TRUE);
    
            if(uw != NULL){
                wri = jni_writer(os_malloc((size_t)(C_SIZEOF(jni_writer))));
                wri->publisher = pub;
                wri->topic = top;
                wri->uwriter = uw;
                wri->deserializer = sd_serializerXMLNewTyped(arg.type);
            }
        }
    }
    return wri;
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
    
    copyArg = (jni_writerCopyArg)data;
    serData = sd_serializerFromString(copyArg->writer->deserializer, copyArg->xmlData);
    sd_serializerDeserializeIntoValidated(copyArg->writer->deserializer, serData, to);
    sd_serializedDataFree(serData);
    
    return TRUE;
}
