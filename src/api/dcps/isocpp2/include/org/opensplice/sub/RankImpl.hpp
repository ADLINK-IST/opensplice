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
