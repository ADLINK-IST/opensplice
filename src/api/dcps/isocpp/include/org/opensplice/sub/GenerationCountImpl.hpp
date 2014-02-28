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

#ifndef ORG_OPENSPLICE_SUB_GENERATION_COUNT_IMPL_HPP_
#define ORG_OPENSPLICE_SUB_GENERATION_COUNT_IMPL_HPP_

namespace org
{
namespace opensplice
{
namespace sub
{
class GenerationCountImpl;
}
}
}

class org::opensplice::sub::GenerationCountImpl
{
public:
    GenerationCountImpl() : d_(0), nw_(0) { }
    GenerationCountImpl(int32_t d, int32_t nw) : d_(d), nw_(nw) { }

public:
    inline int32_t disposed() const
    {
        return d_;
    }

    inline int32_t no_writers() const
    {
        return nw_;
    }

    bool operator ==(const GenerationCountImpl& other) const
    {
        return other.d_ == d_ && other.nw_ == nw_;
    }

private:
    int32_t d_, nw_;
};

#endif /* ORG_OPENSPLICE_SUB_GENERATION_COUNT_IMPL_HPP_ */
