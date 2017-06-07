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
 *
 */

#ifndef VORTEX_FACE_DATALISTENER_HPP_
#define VORTEX_FACE_DATALISTENER_HPP_

#include "Vortex_FACE.hpp"
#include "Vortex/FACE/ReportSupport.hpp"

namespace Vortex {
namespace FACE {

template <typename TYPE>
class DataListener : public virtual dds::sub::NoOpDataReaderListener<TYPE>
{
public:
    typedef typename Vortex::FACE::smart_ptr_traits< DataListener<TYPE> >::shared_ptr shared_ptr;

    DataListener(const std::string &tn,
                 const std::string &cn,
                 typename ::FACE::Read_Callback<TYPE>::send_event cb) :
                     typeName(tn),
                     connectionName(cn),
                     transactionId(0),
                     callback(cb)
    {
    }

    virtual ~DataListener() {}

protected:
    virtual void on_data_available (dds::sub::DataReader<TYPE>& reader)
    {
        ::FACE::RETURN_CODE_TYPE status = ::FACE::NO_ERROR;
        try {
            typename dds::sub::LoanedSamples<TYPE> samples = reader.take();
            typename dds::sub::LoanedSamples<TYPE>::const_iterator sample;
            for (sample = samples.begin(); sample != samples.end(); ++sample) {
                TYPE data = sample->data();
                const dds::sub::SampleInfo& info = sample->info();
                if (info.valid()) {
                    this->callback(transactionId++,
                                   data,
                                   0,
                                   0,
                                   dummyMask,
                                   status);
                    if (status != ::FACE::NO_ERROR) {
                        FACE_REPORT_ERROR(status, "This was returned by the callback for '%s'<%s> (we can not handle that).", this->connectionName.c_str(), this->typeName.c_str());
                    }
                }
            }
        } catch (const dds::core::Exception& e) {
            status = Vortex::FACE::exceptionToReturnCode(e);
        } catch (...) {
            FACE_REPORT_ERROR(::FACE::NO_ACTION, "Data take for '%s'<%s> callback", this->connectionName.c_str(), this->typeName.c_str());
            assert(false);
        }
    }

private:
    std::string typeName;
    std::string connectionName;
    ::FACE::WAITSET_TYPE dummyMask;
    ::FACE::TRANSACTION_ID_TYPE transactionId;
    typename ::FACE::Read_Callback<TYPE>::send_event callback;
};

}; /* namespace FACE */
}; /* namespace Vortex */


#endif /* VORTEX_FACE_DATALISTENER_HPP_ */
