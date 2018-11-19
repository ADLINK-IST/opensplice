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
#include "ReportUtils.h"

#include "c_base.h"
#include "c_metabase.h"
#include "sd_serializerXMLTypeinfo.h"
#include "os_atomics.h"

class SerializationBaseHolder {
public:
    SerializationBaseHolder();
    ~SerializationBaseHolder();

    c_base get_base();

    int is_valid()
    {
        return valid;
    }

private:
    pa_voidp_t basePtr;
    int valid;
};

SerializationBaseHolder::SerializationBaseHolder()
{
    pa_stvoidp(&basePtr, NULL);
    valid = 0;
}

SerializationBaseHolder::~SerializationBaseHolder()
{
    c_base base = (c_base)pa_ldvoidp(&basePtr);
    valid = 0;
    if (base) {
        c_destroy(base);
    }

}


c_base
SerializationBaseHolder::get_base()
{
    c_base base = (c_base)pa_ldvoidp(&basePtr);
    if (!base) {
        base = c_create("message_serializer", NULL, 0, 0);
        if (base) {
            if (!pa_casvoidp(&basePtr, NULL, base)) {
                c_destroy(base);
                base = (c_base) pa_ldvoidp(&basePtr);
            }
        }
        valid = base != NULL;
    }
    return base;
}

static SerializationBaseHolder serialization_base_holder;



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
    this->cdrMarshaler = NULL;
    this->cType = NULL;
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
    if (serialization_base_holder.is_valid()) {
        c_free(cType);
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

void *
DDS::OpenSplice::TypeSupportMetaHolder::get_ctype()
{
    return c_keep(cType);
}

DDS::ReturnCode_t
DDS::OpenSplice::TypeSupportMetaHolder::init_cdr()
{
    DDS::ReturnCode_t result = DDS::RETCODE_OK;
    c_base cbase;

    result = write_lock();
    if (result != DDS::RETCODE_OK) {
        return result;
    }

    if (cType) {
        unlock();
        return result;
    }

    cbase = serialization_base_holder.get_base();
    if (cbase)  {
        sd_serializer serializer;
        sd_serializedData serData;
        c_metaObject mt;
        char *metaDescr = get_meta_descriptor();

        serializer = sd_serializerXMLTypeinfoNew(cbase, TRUE);
        serData = sd_serializerFromString(serializer, metaDescr);
        mt = c_metaObject(sd_serializerDeserialize(serializer, serData));
        if (mt) {
            cType = c_resolve(cbase, typeName);
            if (!cType) {
                result = DDS::RETCODE_BAD_PARAMETER;
                CPP_REPORT(result, "Could not inject type: %%s'", typeName);
            }
        } else {
            result = DDS::RETCODE_BAD_PARAMETER;
            CPP_REPORT(result, "Could not inject type: '%s'", typeName);
        }
        sd_serializedDataFree(serData);
        sd_serializerFree(serializer);
        DDS::string_free(metaDescr);
    } else {
        result = DDS::RETCODE_OUT_OF_RESOURCES;
        CPP_REPORT(result, "Could not inject type: '%s'", typeName);
    }

    if (cType) {
        cdrMarshaler = sd_cdrInfoNew((c_type)cType);
        if (cdrMarshaler) {
            if (sd_cdrCompile((struct sd_cdrInfo *)cdrMarshaler) < 0) {
                sd_cdrInfoFree((struct sd_cdrInfo *) cdrMarshaler);
                cdrMarshaler = NULL;
                c_free(cType);
                cType = NULL;
                result = DDS::RETCODE_BAD_PARAMETER;
                CPP_REPORT(result, "Could not create marshaler for type: '%s'", typeName);
            }
        } else {
            c_free(cType);
            cType = NULL;
            result = DDS::RETCODE_BAD_PARAMETER;
            CPP_REPORT(result, "Could not create marshaler for type: '%s'", typeName);
        }
    }

    unlock();

    return result;
}
