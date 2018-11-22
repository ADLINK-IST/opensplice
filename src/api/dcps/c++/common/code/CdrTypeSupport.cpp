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
#include "DomainParticipant.h"
#include "CdrTypeSupport.h"
#include "ReportUtils.h"

#include "c_base.h"
#include "c_metabase.h"
#include "sd_cdr.h"
#include "os_abstract.h"

#ifdef PA_LITTLE_ENDIAN
#define PLATFORM_IS_LITTLE_ENDIAN 1
#else
#define PLATFORM_IS_LITTLE_ENDIAN 0
#endif

namespace DDS
{
    namespace OpenSplice
    {
        class CdrSerializedDataImpl : public DDS::OpenSplice::CdrSerializedData
        {
        public:
            CdrSerializedDataImpl(
               struct sd_cdrSerdata *serdata)
            {
                _serdata = serdata;
                if (serdata) {
                    const void *blob;
                    _size = sd_cdrSerdataBlob(&blob, _serdata);

                } else {
                    _size = 0;
                }
            }

            virtual ~CdrSerializedDataImpl()
            {
                if (_serdata) {
                    sd_cdrSerdataFree(_serdata);
                }
            }

            virtual unsigned int get_size()
            {
                return _size + 4;
            }

            virtual void get_data(void *buffer)
            {
                unsigned char *data = (unsigned char *)buffer;
                const void *blob;
                unsigned int sz;

                sz = sd_cdrSerdataBlob(&blob, _serdata);

                memcpy(&data[4], blob, sz);
                data[0] = 0;
                data[1] = PLATFORM_IS_LITTLE_ENDIAN;
                data[2] = 0;
                data[3] = 0;
            }

        private:
            unsigned int _size;
            struct sd_cdrSerdata *_serdata;
        };
    }
}

DDS::ReturnCode_t
DDS::OpenSplice::CdrTypeSupport::serialize(
    const void *message,
    CdrSerializedData **cdrdata)
{
    ReturnCode_t result = RETCODE_OK;
    struct sd_cdrSerdata *serdata;
    struct sd_cdrInfo *marshaler;
    c_type ctype;
    c_base cbase;
    c_object obj;

    CPP_REPORT_STACK();

    if (!message || !cdrdata) {
        result =  DDS::RETCODE_BAD_PARAMETER;
        CPP_REPORT(result, "message or serdata incorrect");
    } else {
        result = tsMetaHolder->init_cdr();
        if (result == DDS::RETCODE_OK) {
            marshaler = static_cast<struct sd_cdrInfo *>(tsMetaHolder->get_cdrMarshaler());
            ctype = static_cast<c_type>(tsMetaHolder->get_ctype());

            /* copy message in */
            cxxCopyIn copyIn = tsMetaHolder->get_copy_in();

            obj = c_new_s(ctype);
            cbase = c_getBase(c_object(ctype));
            copyIn(cbase, message, obj);

            c_free(ctype);

            serdata = sd_cdrSerialize(marshaler, obj);
            if (serdata) {
                *cdrdata = new CdrSerializedDataImpl(serdata);
            } else {
                result = DDS::RETCODE_BAD_PARAMETER;
                CPP_REPORT(result, "could not serialize data");
            }
            c_free(obj);
        }
    }

    CPP_REPORT_FLUSH_NO_ID(result != DDS::RETCODE_OK);

    return result;
}


DDS::ReturnCode_t
DDS::OpenSplice::CdrTypeSupport::deserialize(
    const void *serialized_message,
    unsigned int message_size,
    void *message)
{
    DDS::ReturnCode_t result;
    c_object obj;
    struct sd_cdrInfo *marshaler;
    int swap = 0;

    CPP_REPORT_STACK();

    const unsigned char *header = (const unsigned char *)serialized_message;
    if (message_size < 4) {
        return DDS::RETCODE_BAD_PARAMETER;
    }

    if ((header[1] & 0x01) == 0) {
        swap = PLATFORM_IS_LITTLE_ENDIAN;
    } else {
        swap = !PLATFORM_IS_LITTLE_ENDIAN;
    }

    result = tsMetaHolder->init_cdr();
    if (result == DDS::RETCODE_OK) {
        int cdrResult;

        marshaler = static_cast<struct sd_cdrInfo *>(tsMetaHolder->get_cdrMarshaler());

        if (!swap) {
            cdrResult = sd_cdrDeserializeObject(&obj, marshaler, message_size - 4, &header[4]);
        } else {
            cdrResult = sd_cdrDeserializeObjectBSwap(&obj, marshaler, message_size - 4, &header[4]);
        }
        if (cdrResult == SD_CDR_OK) {
            cxxCopyOut copyOut = tsMetaHolder->get_copy_out();
            copyOut(obj, message);
            result = DDS::RETCODE_OK;
        } else if (cdrResult == SD_CDR_OUT_OF_MEMORY) {
            CPP_REPORT(result, "could allocate enough resources");
            result = DDS::RETCODE_OUT_OF_RESOURCES;
        } else {
            result = DDS::RETCODE_BAD_PARAMETER;
            CPP_REPORT(result, "could not deserialize data");
        }
    }

    CPP_REPORT_FLUSH_NO_ID(result != DDS::RETCODE_OK);

    return result;
}

DDS::OpenSplice::CdrTypeSupport::CdrTypeSupport(DDS::TypeSupport& ts)
{
   DDS::OpenSplice::TypeSupport *tss = dynamic_cast<DDS::OpenSplice::TypeSupport *>(&ts);
   tsMetaHolder = tss->get_metaHolder();
}

DDS::OpenSplice::CdrTypeSupport::~CdrTypeSupport()
{
}
