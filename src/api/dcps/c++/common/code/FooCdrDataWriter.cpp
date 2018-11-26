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
#include "FooCdrDataWriter.h"
#include "ccpp_dds_cdrBlob.h"


DDS::OpenSplice::FooCdrDataWriter::FooCdrDataWriter(
     DDS::DataWriter *wr)
{
    writer = dynamic_cast<DDS::OpenSplice::FooDataWriter_impl *>(wr);
}

DDS::OpenSplice::FooCdrDataWriter::~FooCdrDataWriter()
{

}

::DDS::ReturnCode_t
DDS::OpenSplice::FooCdrDataWriter::write_cdr(
    const DDS::CDRSample & instance_data,
    ::DDS::InstanceHandle_t handle)
{
    if (!writer) {
        return DDS::RETCODE_PRECONDITION_NOT_MET;
    }

    if (instance_data.blob.length() < 4) {
        return DDS::RETCODE_BAD_PARAMETER;
    }

    return writer->write_cdr(instance_data, handle);
}
