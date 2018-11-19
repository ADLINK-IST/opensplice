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
#include "ReportUtils.h"
#include "FooCdrDataReader.h"
#include "ccpp_dds_cdrBlob.h"

#ifdef PA_LITTLE_ENDIAN
#define PLATFORM_IS_LITTLE_ENDIAN 1
#else
#define PLATFORM_IS_LITTLE_ENDIAN 0
#endif

DDS::OpenSplice::FooCdrDataReader::FooCdrDataReader(
     DDS::DataReader *rd)
{
    reader = dynamic_cast<DDS::OpenSplice::FooDataReader_impl *>(rd);
}

DDS::OpenSplice::FooCdrDataReader::~FooCdrDataReader()
{

}

DDS::ReturnCode_t
DDS::OpenSplice::FooCdrDataReader::read_cdr(
     ::DDS::CDRSample & received_data,
     ::DDS::SampleInfo & info,
     ::DDS::SampleStateMask sample_states,
     ::DDS::ViewStateMask view_states,
     ::DDS::InstanceStateMask instance_states)
{
    if (!reader) {
        return DDS::RETCODE_PRECONDITION_NOT_MET;
    }

    return reader->read_cdr(&received_data, info, sample_states, view_states, instance_states);
}

DDS::ReturnCode_t
DDS::OpenSplice::FooCdrDataReader::take_cdr(
     ::DDS::CDRSample & received_data,
     ::DDS::SampleInfo & info,
     ::DDS::SampleStateMask sample_states,
     ::DDS::ViewStateMask view_states,
     ::DDS::InstanceStateMask instance_states)
{
    unsigned char header[4] = { 0, PLATFORM_IS_LITTLE_ENDIAN, 0, 0 };

    if (!reader) {
        return DDS::RETCODE_PRECONDITION_NOT_MET;
    }

    DDS::CDRSample sample;

    DDS::ReturnCode_t result = reader->take_cdr(&sample, info, sample_states, view_states, instance_states);
    if (result != DDS::RETCODE_OK) {
        return result;
    }

    unsigned int sz = sample.blob.length();
    unsigned char *from_payload = sample.blob.get_buffer();

    received_data.blob.length(sz + sizeof(header));

    unsigned char *to_buffer = received_data.blob.get_buffer();
    memcpy(to_buffer, header, sizeof(header));
    memcpy(&to_buffer[sizeof(header)], from_payload, sz);

    return result;
}
