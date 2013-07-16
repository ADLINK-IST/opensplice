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
#include "cmx__reader.h"
#include "cmx__dataReader.h"
#include "cmx__groupqueue.h"
#include "cmx__networkReader.h"
#include "cmx__entity.h"
#include "cmx__factory.h"
#include "sd_serializerXML.h"
#include "sd_serializerXMLMetadata.h"
#include "u_entity.h"
#include "u_reader.h"
#include "v_query.h"
#include "v_dataReader.h"
#include "v_groupQueue.h"
#include "v_reader.h"
#include "v_topic.h"
#include "v_observer.h"
#include "v_kernel.h"
#include "v_state.h"
#include "os_report.h"
#include "os_stdlib.h"
#include <stdio.h>

c_char*
cmx_readerInit(
    v_reader entity)
{
    c_char* result;

    result = NULL;

    switch(v_object(entity)->kind){
    case K_DATAREADER:
        result = cmx_dataReaderInit((v_dataReader)entity);
    break;
    case K_NETWORKREADER:
        result = cmx_networkReaderInit((v_networkReader)entity);
    break;
    case K_GROUPQUEUE:
        result = cmx_groupQueueInit((v_groupQueue)entity);
    break;
    default: break;
    }

    return result;
}

struct cmx_readerArg {
    c_char* result;
};

c_char*
cmx_readerDataType(
    const c_char* reader)
{
    u_entity entity;
    struct cmx_readerArg arg;

    entity = cmx_entityUserEntity(reader);
    arg.result = NULL;

    if(entity != NULL){
        u_entityAction(entity, cmx_readerDataTypeAction, &arg);
    }
    return arg.result;
}

void
cmx_readerDataTypeAction(
    v_entity entity,
    c_voidp args)
{
    sd_serializer ser;
    sd_serializedData data;
    c_type type;
    v_dataReader r;
    v_query query;
    v_topic topic;
    struct cmx_readerArg *arg;
    arg = (struct cmx_readerArg *)args;

    type = NULL;

    switch(v_object(entity)->kind){

    case K_DATAREADER:
        r = v_dataReader(entity);
        v_observerLock(v_observer(r));
        topic = v_dataReaderGetTopic(r);
        type = v_topicDataType(topic);
        c_free(topic);
        v_observerUnlock(v_observer(r));
    break;
    case K_DATAREADERQUERY:
        query = v_query(entity);
        r = v_dataReader(v_querySource(query));
        v_observerLock(v_observer(r));
        topic = v_dataReaderGetTopic(r);
        type = v_topicDataType(topic);
        c_free(topic);
        v_observerUnlock(v_observer(r));
        c_free(r);
    break;
    case K_NETWORKREADER:
        OS_REPORT(OS_ERROR, CM_XML_CONTEXT, 0,
                  "Resolving data type of networkReader unsupported.\n");
        assert(FALSE);
    break;
    case K_GROUPQUEUE:
        OS_REPORT(OS_ERROR, CM_XML_CONTEXT, 0,
                  "Resolving data type of groupQueue unsupported.\n");
        assert(FALSE);
    break;
    default:
        OS_REPORT(OS_ERROR, CM_XML_CONTEXT, 0,
                  "Trying to resolve dataType of unknown reader type.\n");
        assert(FALSE);
    break;
    }

    if(type != NULL){
        ser = sd_serializerXMLMetadataNew(c_getBase(type));
        data = sd_serializerSerialize(ser, type);
        arg->result = sd_serializerToString(ser, data);
        sd_serializedDataFree(data);
        sd_serializerFree(ser);
    }
}

c_char*
cmx_readerRead(
    const c_char* reader)
{
    u_reader ureader;
    struct cmx_readerArg arg;

    arg.result = NULL;

    ureader = u_reader(cmx_entityUserEntity(reader));

    if(ureader != NULL){
        u_readerRead(ureader, cmx_readerReadCopy, &arg);
    }
    return arg.result;
}

c_char*
cmx_readerTake(
    const c_char* reader)
{
    u_reader ureader;
    struct cmx_readerArg arg;

    arg.result = NULL;

    ureader = u_reader(cmx_entityUserEntity(reader));

    if(ureader != NULL){
        u_readerTake(ureader, cmx_readerCopy, &arg);
    }
    return arg.result;
}

c_char*
cmx_readerReadNext(
    const c_char* reader,
    const c_char* localId,
    const c_char* extId)
{
    /*
    u_reader ureader;
    c_bool result;
    struct cmx_readerArg arg;
    cmx_entityKernelArg kernelArg;
    u_instanceHandle handle;
    c_ulong sid;
    c_ulong loid;
    c_ulong liid;

    ureader = u_reader(cmx_entityUserEntity(reader));
    arg.result = NULL;

    if(ureader != NULL){
        kernelArg = cmx_entityKernelArg(os_malloc(C_SIZEOF(cmx_entityKernelArg)));
        u_entityAction(u_entity(ureader), cmx_entityKernelAction, (c_voidp)kernelArg);
        sscanf(localId, "%u", &loid);
        sscanf(extId, "%u", &liid);

        handle.gid.localId = loid;
        handle.gid.extId = liid;
        handle.kernel = kernelArg->kernel;
        result = u_readerReadNextInstance(ureader, handle, cmx_readerCopy, &arg);

        os_free(kernelArg);
    }
    return arg.result;
    */
    return cmx_readerRead(reader);
}

v_actionResult
cmx_readerCopy(
    c_object o,
    c_voidp args)
{
    sd_serializer ser;
    sd_serializedData data;
    struct cmx_readerArg *arg;
    v_dataReaderSample sample, older;
    v_actionResult result = 0;
    v_lifespanSample next;

    arg = (struct cmx_readerArg *)args;

    if(o != NULL){
        sample = v_dataReaderSample(o);
        older = sample->older;
        next = sample->_parent._parent.next;
        sample->older = NULL;
        sample->_parent._parent.next = NULL;

        ser = sd_serializerXMLNewTyped(c_getType(o));
        data = sd_serializerSerialize(ser, o);
        arg->result = sd_serializerToString(ser, data);
        sd_serializedDataFree(data);
        sd_serializerFree(ser);

        sample->older = older;
        sample->_parent._parent.next = next;
    }
    return result;
}

v_actionResult
cmx_readerReadCopy(
    c_object o,
    c_voidp args)
{
    sd_serializer ser;
    sd_serializedData data;
    v_dataReaderSample sample, newer;
    struct cmx_readerArg *arg;
    v_actionResult result;

    result = 0;
    if(o != NULL){
        sample = v_dataReaderSample(o);

        if(v_stateTest(v_readerSampleState(sample), L_READ)){
            v_actionResultSet(result, V_PROCEED);
        } else {
            sample = v_dataReaderSample(o);
            newer = sample->newer;
            sample->newer = NULL;

            arg = (struct cmx_readerArg *)args;
            ser = sd_serializerXMLNewTyped(c_getType(o));
            data = sd_serializerSerialize(ser, o);
            arg->result = sd_serializerToString(ser, data);
            sd_serializedDataFree(data);
            sd_serializerFree(ser);

            sample->newer = newer;
        }
    }
    return result;
}
