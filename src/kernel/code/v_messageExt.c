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
 */
#include "c_base.h"
#include "v_topic.h"
#include "v_messageExt.h"
#include "v_state.h"
#include "sd_cdr.h"
#include "os_heap.h"


static os_uint64
c_timeToUint64(
    c_time time)
{
    os_uint64 v;

    v = (os_uint64)time.seconds;
    v = (v << 32) + time.nanoseconds;

    return v;
}

static c_time
c_timeFromUint64(
    os_uint64 value)
{
    c_time tr;

    tr.seconds = (c_long)(value >> 32);
    tr.nanoseconds = value & 0xFFFFFFFFLU;

    return tr;
}


c_type v_messageExtTypeNew (v_topic topic)
{
    /* closely follows v_topic.c:messageTypeNew */
    static const char baseTypeName[] = "kernelModule::v_messageExt";
    c_type dataType = v_topicDataType (topic);
    c_base base = c_getBase (topic);
    c_type baseType = c_resolve (base, baseTypeName);
    c_type type;
    c_object o;

    assert (dataType);
    assert (baseType);

    type = c_type(c_metaDefine (c_metaObject(base), M_CLASS));
    c_class(type)->extends = c_keep (c_class(baseType));
    o = c_metaDeclare (c_metaObject(type), "userData", M_ATTRIBUTE);
    c_property(o)->type = c_keep (dataType);
    c_metaObject(type)->definedIn = c_keep (base);
    c_metaFinalize (c_metaObject(type));
    c_free (baseType);
    c_free(o);

    /* leave anonymous */
    return type;
}

void v_messageExtTypeFree (c_type xmsgType)
{
    c_free (xmsgType);
}


static void v_messageExtConvertHeaderFromExtCommon (v_message vmsg, const struct v_messageExt_s *xmsg, c_bool y2038Ready)
{
    vmsg->_parent.nodeState = xmsg->_parent.nodeState;
    if (y2038Ready) {
        OS_TIMEW_GET_VALUE(vmsg->writeTime) = c_timeToUint64(xmsg->writeTime);
        v_stateClear(vmsg->_parent.nodeState, L_TIME_Y2038);
    } else {
        vmsg->writeTime = c_timeToTimeW(xmsg->writeTime);
    }
    vmsg->writerGID = xmsg->writerGID;
    vmsg->writerInstanceGID = xmsg->writerInstanceGID;
    vmsg->sequenceNumber = xmsg->sequenceNumber;
    vmsg->transactionId = xmsg->transactionId;
    vmsg->qos = xmsg->qos;
}

static void v_messageExtConvertHeaderToExtCommon (v_messageExt xmsg, const struct v_message_s *vmsg, c_bool y2038Ready)
{
    xmsg->_parent.nodeState = vmsg->_parent.nodeState;
    if (y2038Ready) {
        v_stateSet(xmsg->_parent.nodeState, L_TIME_Y2038);
        xmsg->writeTime = c_timeFromUint64(OS_TIMEW_GET_VALUE(vmsg->writeTime));
    } else {
        xmsg->writeTime = c_timeFromTimeW(vmsg->writeTime);
    }
    xmsg->writerGID = vmsg->writerGID;
    xmsg->writerInstanceGID = vmsg->writerInstanceGID;
    xmsg->sequenceNumber = vmsg->sequenceNumber;
    xmsg->transactionId = vmsg->transactionId;
    xmsg->qos = vmsg->qos;
}

void v_messageExtConvertHeaderFromExt (v_message vmsg, const struct v_messageExt_s *xmsg)
{
    c_bool y2038Ready = v_stateTest(xmsg->_parent.nodeState, L_TIME_Y2038);

    v_messageExtConvertHeaderFromExtCommon(vmsg, xmsg, y2038Ready);
    if (y2038Ready) {
        OS_TIMEE_GET_VALUE(vmsg->allocTime) = c_timeToUint64(xmsg->allocTime);
    } else {
        vmsg->allocTime = c_timeToTimeE(xmsg->allocTime);
    }
}

void v_messageExtConvertHeaderToExt (v_messageExt xmsg, const struct v_message_s *vmsg)
{
    c_bool y2038Ready = c_baseGetY2038Ready(c_getBase(c_object(vmsg)));

    v_messageExtConvertHeaderToExtCommon(xmsg, vmsg, y2038Ready);
    if (y2038Ready) {
        xmsg->allocTime = c_timeFromUint64(OS_TIMEE_GET_VALUE(vmsg->allocTime));
    } else {
        xmsg->allocTime = c_timeFromTimeE(vmsg->allocTime);
    }
}

void v_messageExtConvertHeaderFromExtNoAllocTime (v_message vmsg, const struct v_messageExt_s *xmsg)
{
    c_bool y2038Ready = v_stateTest(xmsg->_parent.nodeState, L_TIME_Y2038);

    v_messageExtConvertHeaderFromExtCommon(vmsg, xmsg, y2038Ready);
    vmsg->allocTime = os_timeEGet();
}

void v_messageExtConvertHeaderToExtNoAllocTime (v_messageExt xmsg, const struct v_message_s *vmsg)
{
    c_bool y2038Ready = c_baseGetY2038Ready(c_getBase(c_object(vmsg)));

    v_messageExtConvertHeaderToExtCommon(xmsg, vmsg, y2038Ready);
    xmsg->allocTime.seconds = 0;
    xmsg->allocTime.nanoseconds = 0;
}

void v_messageEOTExtConvertFromExtNoAllocTime (v_messageEOT vmsgEOT, const struct v_messageEOTExt_s *xmsgEOT)
{
    v_messageExtConvertHeaderFromExtNoAllocTime (&vmsgEOT->_parent, &xmsgEOT->_parent);
    vmsgEOT->publisherId = xmsgEOT->publisherId;
    vmsgEOT->transactionId = xmsgEOT->transactionId;
    /* xmsg lives on stack, no need for c_keep(tidList) */
    vmsgEOT->tidList = xmsgEOT->tidList;
}

void v_messageEOTExtConvertToExtNoAllocTime (v_messageEOTExt xmsgEOT, const struct v_messageEOT_s *vmsgEOT)
{
    v_messageExtConvertHeaderToExtNoAllocTime (&xmsgEOT->_parent, &vmsgEOT->_parent);
    xmsgEOT->publisherId = vmsgEOT->publisherId;
    xmsgEOT->transactionId = vmsgEOT->transactionId;
    /* xmsg lives on stack, no need for c_keep(tidList) */
    xmsgEOT->tidList = vmsgEOT->tidList;
}

v_message v_messageExtConvertFromExtType (c_type msgType, v_messageExt xmsg)
{
    c_type xmsgType = c_getType (xmsg);
    v_message vmsg;
    vmsg = c_new_s (msgType);
    if (vmsg) {
        v_messageExtConvertHeaderFromExt (vmsg, xmsg);
        memcpy (vmsg + 1, xmsg + 1, xmsgType->size - sizeof (*xmsg));
        memset (xmsg, 0, xmsgType->size);
        c_free (xmsg);
    }
    return vmsg;
}

v_messageExt v_messageExtCopyToExtType (c_type xmsgType, const struct v_message_s *vmsg)
{
    v_messageExt xmsg;
    xmsg = c_new (xmsgType);
    v_messageExtConvertHeaderToExt (xmsg, vmsg);
    memcpy (xmsg + 1, vmsg + 1, xmsgType->size - sizeof (*xmsg));
    return xmsg;
}

void v_messageExtFree (v_messageExt xmsg)
{
    c_type xmsgType = c_getType (xmsg);
    memset (xmsg, 0, xmsgType->size);
    c_free (xmsg);
}

v_messageEOTExt v_messageEOTExtCopyToExtType (const struct v_messageEOT_s *vmsg)
{
    c_type xmsgType = c_resolve(c_getBase((c_object)vmsg), "kernelModule::v_messageEOTExt");
    v_messageEOTExt xmsg;
    xmsg = c_new (xmsgType);
    c_free(xmsgType);
    v_messageEOTExtConvertToExtNoAllocTime (xmsg, vmsg);
    return xmsg;
}


v_message v_messageEOTExtConvertFromExtType (v_messageEOTExt xmsg_eot)
{
    c_type msgType = c_resolve(c_getBase((c_object)xmsg_eot), "kernelModuleI::v_messageEOT");
    return v_messageExtConvertFromExtType (msgType, (v_messageExt) xmsg_eot);
}

void v_messageEOTExtFree (v_messageEOTExt xmsg)
{
    c_type xmsgType = c_getType (xmsg);
    memset (xmsg, 0, xmsgType->size);
    c_free (xmsg);
}

struct v_messageExtCdrInfo {
    struct sd_cdrInfo *ci;
    c_type vmsgType;
};

struct v_messageExtCdrTmp {
    struct v_messageExt_s *h;
    void *d;
};

struct v_messageExtCdrInfo *v_messageExtCdrInfoNew(c_type topicMessageType, const struct sd_cdrControl *control)
{
    static const char headerTypeName[] = "kernelModule::v_messageExt";
    c_base base = c_getBase (topicMessageType);
    c_property userDataProperty = c_property(c_metaResolve(c_metaObject(topicMessageType),"userData"));
    c_type topicDataType = userDataProperty->type;
    c_type headerClass = c_resolve (base, headerTypeName);
    c_type dataClass;
    c_type type;
    c_object o;
    c_array members;
    struct v_messageExtCdrInfo *xci;
    struct c_type_s const *tstk[2];
    c_free(userDataProperty);

    /* Wrap user data in a class, so that we can make a struct with a pointer to it */
    dataClass = c_type(c_metaDefine(c_metaObject(base), M_CLASS));
    c_class(dataClass)->extends = NULL;
    o = c_metaDeclare(c_metaObject(dataClass), "userData", M_ATTRIBUTE);
    c_property(o)->type = c_keep(topicDataType);
    c_metaObject(dataClass)->definedIn = c_keep(base);
    c_metaFinalize(c_metaObject(dataClass));
    c_free(o);

    /* Make a struct containing two pointers, one to the v_messageExt class, and one to
     the just-created anonymous dataClass type, corresponding to struct v_messageExtCdrTmp */
    type = c_type(c_metaDefine(c_metaObject(base),M_STRUCTURE));
    members = c_arrayNew(c_member_t(base),2);
    members[0] = (c_voidp)c_metaDefine(c_metaObject(base),M_MEMBER);
    c_specifier(members[0])->name = c_stringNew(base,"h");
    c_specifier(members[0])->type = c_keep(headerClass);
    members[1] = (c_voidp)c_metaDefine(c_metaObject(base),M_MEMBER);
    c_specifier(members[1])->name = c_stringNew(base,"d");
    c_specifier(members[1])->type = c_keep(dataClass);
    c_structure(type)->members = members;
    c_metaObject(type)->definedIn = c_keep(base);
    c_metaFinalize(c_metaObject(type));
    c_free(dataClass);
    c_free(headerClass);

    xci = os_malloc(sizeof(*xci));
    xci->ci = sd_cdrInfoNewControl(type, control);

    /* Note: current simplistic annotation processing requires the annotations
     to be made in the order in which they are encountered when walking the
     type */
    tstk[0] = type;
    tstk[1] = headerClass;
    if (sd_cdrNoteQuietRef(xci->ci, 2, tstk) < 0) {
        goto err_note;
    }
    tstk[1] = dataClass;
    if (sd_cdrNoteQuietRef(xci->ci, 2, tstk) < 0) {
        goto err_note;
    }

    if (sd_cdrCompile(xci->ci) < 0) {
        goto err_compile;
    }
    xci->vmsgType = c_keep(topicMessageType);
    c_free(type);
    return xci;

err_compile:
err_note:
    os_free(xci);
    c_free(type);
    return NULL;
}

void v_messageExtCdrInfoFree(struct v_messageExtCdrInfo *xci)
{
    sd_cdrInfoFree(xci->ci);
    c_free(xci->vmsgType);
}

int v_messageExtCdrSerializeNoAllocTime (int (*f) (const struct sd_cdrInfo *ci, void *serdata, const void *data), const struct v_messageExtCdrInfo *xci, void *serdata, const struct v_message_s *vmsg)
{
    struct v_messageExtCdrTmp tmp;
    struct v_messageExt_s xmsg;
    tmp.h = &xmsg;
    tmp.d = (void *) (vmsg + 1);
    v_messageExtConvertHeaderToExtNoAllocTime(&xmsg, vmsg);
    return f(xci->ci, serdata, &tmp);
}

int v_messageExtCdrDeserializeNoAllocTime (int (*f) (void *dst, const struct sd_cdrInfo *ci, os_uint32 sz, const void *src), v_message *dst, const struct v_messageExtCdrInfo *xci, os_uint32 sz, const void *src)
{
    struct v_messageExtCdrTmp tmp;
    struct v_messageExt_s xmsg;
    v_message vmsg;
    int rc;
    if ((vmsg = c_new (xci->vmsgType)) == NULL) {
        return SD_CDR_OUT_OF_MEMORY;
    }
    tmp.h = &xmsg;
    tmp.d = vmsg + 1;
    if ((rc = f(&tmp, xci->ci, sz, src)) < 0) {
        c_free(vmsg);
        return rc;
    }
    v_messageExtConvertHeaderFromExtNoAllocTime(vmsg, &xmsg);
    *dst = vmsg;
    return 0;
}
