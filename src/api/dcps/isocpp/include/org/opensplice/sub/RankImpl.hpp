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

#ifndef ORG_OPENSPLICE_SUB_RANK_IMPL_HPP_
#define ORG_OPENSPLICE_SUB_RANK_IMPL_HPP_

namespace org
{
namespace opensplice
{
namespace sub
{
class RankImpl;
}
}
}
class org::opensplice::sub::RankImpl
{
public:
    RankImpl() : s_(0), g_(0), ag_(0) { }
    RankImpl(int32_t s, int32_t g, int32_t ag) : s_(s), g_(g), ag_(ag) { }

public:
    inline int32_t absolute_generation() const
    {
        return ag_;
    }

    inline int32_t generation() const
    {
        return g_;
    }

    inline int32_t sample() const
    {
        return s_;
    }

    bool operator ==(const RankImpl& other) const
    {
        return other.s_ == s_ && other.g_ == g_ && other.ag_;
    }
private:
    int32_t s_;
    int32_t g_;
    int32_t ag_;
};


#endif /* ORG_OPENSPLICE_SUB_RANK_IMPL_HPP_ */
