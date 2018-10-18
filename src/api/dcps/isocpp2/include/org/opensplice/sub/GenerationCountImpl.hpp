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

    inline void disposed(int32_t d)
    {
        this->d_ = d;
    }

    inline int32_t no_writers() const
    {
        return nw_;
    }

    inline void no_writers(int32_t nw)
    {
        this->nw_ = nw;
    }


    bool operator ==(const GenerationCountImpl& other) const
    {
        return other.d_ == d_ && other.nw_ == nw_;
    }

private:
    int32_t d_, nw_;
};

#endif /* ORG_OPENSPLICE_SUB_GENERATION_COUNT_IMPL_HPP_ */
