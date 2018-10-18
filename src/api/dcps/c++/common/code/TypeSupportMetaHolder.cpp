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
#include "sd_cdr.h"
#include "u_user.h"
#include "DomainParticipant.h"
#include "TypeSupportMetaHolder.h"
#include "FooDataWriter_impl.h"
#include "FooDataReader_impl.h"

static int
ccpp_cdrSerdataInit(void *vsd, char **dst, os_uint32 size_hint)
{
    DDS::octSeq *cdrSampleSeq = (DDS::octSeq *) vsd;
    if (cdrSampleSeq->maximum() == 0) {
        cdrSampleSeq->length(size_hint);
    }
    *dst = (char *) cdrSampleSeq->get_buffer(FALSE);
    return (int) cdrSampleSeq->maximum();
}

static int
ccpp_cdrSerdataGrow(void *vsd, char **dst, os_uint32 size_hint)
{
    DDS::octSeq *cdrSampleSeq = (DDS::octSeq *) vsd;
    DDS::ULong prevMax = cdrSampleSeq->maximum();

    cdrSampleSeq->length(prevMax + size_hint);
    *dst = (char *) cdrSampleSeq->get_buffer(FALSE) + prevMax;
    return (int) cdrSampleSeq->maximum();
}

static void
ccpp_cdrSerdataFinalize(void *vsd, char *dst)
{
    DDS::octSeq *cdrSampleSeq = (DDS::octSeq *) vsd;
    cdrSampleSeq->length(dst - (const char *) cdrSampleSeq->get_buffer());
}

static os_uint32
ccpp_cdrSerdataGetpos(const void *vsd, const char *dst)
{
    return 0;
}

static int
ccpp_cdrTagField_notag (os_uint32 *tag, void *arg, enum sd_cdrTagType type, os_uint32 srcoff)
{
    return 0;
}

DDS::OpenSplice::TypeSupportMetaHolder::TypeSupportMetaHolder(
        const char *typeName,
        const char *internalTypeName,
        const char *keyList) :
    DDS::OpenSplice::CppSuperClass(DDS::OpenSplice::TYPESUPPORTMETAHOLDER), cdrMarshaler(NULL)
{
    DDS::OpenSplice::CppSuperClass::nlReq_init();

    this->typeName = typeName;
    // In case no explicit internalTypeName is set, then it is equal to the normal typeName.
    if (strlen(internalTypeName) == 0) {
        this->internalTypeName = typeName;
    } else {
        this->internalTypeName = internalTypeName;
    }
    this->keyList = keyList;
    this->writerCopy = (u_writerCopy) DDS::OpenSplice::FooDataWriter_impl::rlReq_copyIn;
    this->readerCopy = (cxxReaderCopy) DDS::OpenSplice::FooDataReader_impl::copySampleOut;
}


DDS::OpenSplice::TypeSupportMetaHolder::~TypeSupportMetaHolder()
{
    (void) deinit();
}

DDS::OpenSplice::TypeSupportMetaHolder *
DDS::OpenSplice::TypeSupportMetaHolder::createProxyCDRMetaHolder(
    TypeSupportMetaHolder *originalTSMH,
    c_type topicType)
{
    sd_cdrControl control;
    control.init = ccpp_cdrSerdataInit;
    control.grow = ccpp_cdrSerdataGrow;
    control.finalize = ccpp_cdrSerdataFinalize;
    control.getpos = ccpp_cdrSerdataGetpos;
    control.tag = ccpp_cdrTagField_notag;
    control.tag_arg = NULL;
    control.process = 0;
    control.process_arg = NULL;


    DDS::OpenSplice::TypeSupportMetaHolder *cdrProxyTSMH = this->clone();
    cdrProxyTSMH->keyList = originalTSMH->keyList;
    cdrProxyTSMH->typeName = originalTSMH->typeName;
    cdrProxyTSMH->internalTypeName = originalTSMH->internalTypeName;
    delete[] cdrProxyTSMH->metaDescriptor;
    cdrProxyTSMH->metaDescriptor = new const char*[originalTSMH->metaDescriptorArrLength];
    memcpy(cdrProxyTSMH->metaDescriptor, originalTSMH->metaDescriptor, originalTSMH->metaDescriptorArrLength);
    cdrProxyTSMH->metaDescriptorLength = originalTSMH->metaDescriptorLength;
    cdrProxyTSMH->metaDescriptorArrLength = originalTSMH->metaDescriptorArrLength;
    cdrProxyTSMH->writerCopy = (u_writerCopy) DDS::OpenSplice::FooDataWriter_impl::rlReq_cdrCopyIn;
    cdrProxyTSMH->readerCopy = (cxxReaderCopy) DDS::OpenSplice::FooDataReader_impl::copyCDRSampleOut;
    cdrProxyTSMH->cdrMarshaler = sd_cdrInfoNewControl (topicType, &control);
    if (cdrProxyTSMH->cdrMarshaler) {
        if (sd_cdrCompile((sd_cdrInfo *) cdrProxyTSMH->cdrMarshaler) < 0) {
            DDS::release(cdrProxyTSMH);
            cdrProxyTSMH = NULL;
        }
    } else {
        DDS::release(cdrProxyTSMH);
        cdrProxyTSMH = NULL;
    }

    return cdrProxyTSMH;
}


DDS::ReturnCode_t
DDS::OpenSplice::TypeSupportMetaHolder::wlReq_deinit()
{
    if (cdrMarshaler) {
        sd_cdrInfoFree((struct sd_cdrInfo *) cdrMarshaler);
    }
    delete[] metaDescriptor;
    return DDS::OpenSplice::CppSuperClass::wlReq_deinit();
}

char *
DDS::OpenSplice::TypeSupportMetaHolder::get_meta_descriptor()
{
    ::DDS::ULong i;
    char *descriptorString;

    descriptorString = DDS::string_alloc(metaDescriptorLength);
    descriptorString[0] = '\0';
    for ( i= 0; i < metaDescriptorArrLength; i++) {
        os_strcat(descriptorString, metaDescriptor[i]);
    }
    return descriptorString;
}

const char *
DDS::OpenSplice::TypeSupportMetaHolder::get_key_list()
{
    return keyList;
}

const char *
DDS::OpenSplice::TypeSupportMetaHolder::get_type_name()
{
    return typeName;
}

const char *
DDS::OpenSplice::TypeSupportMetaHolder::get_internal_type_name()
{
    return internalTypeName;
}

DDS::OpenSplice::cxxCopyIn
DDS::OpenSplice::TypeSupportMetaHolder::get_copy_in()
{
    return copyIn;
}

DDS::OpenSplice::cxxCopyOut
DDS::OpenSplice::TypeSupportMetaHolder::get_copy_out()
{
    return copyOut;
}

u_writerCopy
DDS::OpenSplice::TypeSupportMetaHolder::get_writerCopy()
{
    return writerCopy;
}

DDS::OpenSplice::cxxReaderCopy
DDS::OpenSplice::TypeSupportMetaHolder::get_readerCopy()
{
    return readerCopy;
}

void *
DDS::OpenSplice::TypeSupportMetaHolder::get_cdrMarshaler()
{
	return cdrMarshaler;
}
