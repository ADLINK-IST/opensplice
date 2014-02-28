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


/**
 * @file
 */

#ifndef ORG_OPENSPLICE_SUB_LOANED_CONTAINER_HPP_
#define ORG_OPENSPLICE_SUB_LOANED_CONTAINER_HPP_

#if 0
#include <dds/sub/SampleInfo.hpp>
#include <org/opensplice/topic/TopicTraits.hpp>

namespace org
{
namespace opensplice
{
namespace sub
{
template <typename T>
class LoanedSequence;
}
}
}

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
template <typename T>
class org::opensplice::sub::LoanedSequence
{
public:
    typedef typename org::opensplice::topic::topic_data_seq<T>::type SequenceType;
    typedef const T* iterator;

public:
    LoanedSequence() { }

public:
    // NOTE: The reinterpret cast below might looks superflous at the
    // first sight, however it is needed to make a DDS::SampleInfo look
    // like a dds::sub::SampleInfo.
    const T* begin() const
    {
        const T* retVal = 0;
        if(s_.length() > 0)
        {
            retVal = reinterpret_cast<const T*>(&s_[0]);
        }
        else
        {
            retVal = reinterpret_cast<const T*>(&s_);
        }

        return retVal;
    }

    const T* end() const
    {
        const T* retVal = 0;
        if(s_.length() > 0)
        {
            retVal = reinterpret_cast<const T*>(&(s_[s_.length() - 1]) + 1);
        }
        else
        {
            retVal = this->begin();
        }

        return retVal;
    }

    SequenceType& sequence()
    {
        return s_;
    }

    uint32_t length() const
    {
        return s_.length();
    }

private:
    LoanedSequence(const LoanedSequence&);
    LoanedSequence& operator=(const LoanedSequence&);

private:
    SequenceType s_;
};

#endif
#endif /* ORG_OPENSPLICE_SUB_LOANED_CONTAINER_HPP_ */
