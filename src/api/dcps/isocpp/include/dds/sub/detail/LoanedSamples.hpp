/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2012 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE
 *
 *   for full copyright notice and license terms.
 *
 */
#ifndef OSPL_DDS_SUB_DETAIL_LOANEDSAMPLES_HPP_
#define OSPL_DDS_SUB_DETAIL_LOANEDSAMPLES_HPP_

/**
 * @file
 */

// Implementation

#include <org/opensplice/core/config.hpp>
#include <org/opensplice/sub/LoanedSequence.hpp>
#include <org/opensplice/topic/TopicTraits.hpp>
#include <vector>
#include <dds/sub/Sample.hpp>
#include <dds/sub/SampleInfo.hpp>
#include <org/opensplice/core/exception_helper.hpp>
#include <org/opensplice/topic/TopicTraits.hpp>

namespace org
{
namespace opensplice
{
namespace topic
{
template<> struct topic_data_seq<dds::sub::SampleInfo>
{
    typedef DDS::SampleInfoSeq type;
    typedef DDS::SampleInfoSeq_uniq_ utype;
};
}
}
}

namespace dds
{
namespace sub
{
namespace detail
{

template <typename T>
class LoanedSamples
{
public:

    typedef std::vector< dds::sub::Sample<T> > LoanedDataContainer;
    typedef typename std::vector< dds::sub::Sample<T> >::iterator iterator;
    typedef typename std::vector< dds::sub::Sample<T> >::const_iterator const_iterator;

    typedef typename org::opensplice::topic::topic_data_reader<T>::type DR;

public:
    LoanedSamples() { }

    ~LoanedSamples()
    {
        if(reader_)
        {
            DR* raw_reader_t_ = DR::_narrow(reader_.get());
            if(raw_reader_t_)
            {
                DDS::ReturnCode_t result = raw_reader_t_->return_loan(data_, info_);
                org::opensplice::core::check_and_throw(result, OSPL_CONTEXT_LITERAL("Calling ::return_loan"));
                DDS::release(raw_reader_t_);
            }
        }
    }

public:

    iterator mbegin()
    {
        return samples_.begin();
    }

    const_iterator begin() const
    {
        return samples_.begin();
    }

    const_iterator end() const
    {
        return samples_.end();
    }

    uint32_t length() const
    {
        /** @internal @todo Possible RTF size issue ? */
        return static_cast<uint32_t>(samples_.size());
    }

    void resize(uint32_t s)
    {
        samples_.resize(s);
    }

    org::opensplice::core::DDS_DR_REF reader_;
    typename org::opensplice::topic::topic_data_seq<T>::type data_;
    typename org::opensplice::topic::topic_data_seq<dds::sub::SampleInfo>::type info_;
private:
    LoanedDataContainer samples_;
};

}
}
}

// End of implementation

#endif /* OSPL_DDS_SUB_DETAIL_LOANEDSAMPLES_HPP_ */
