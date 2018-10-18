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

#ifndef ORG_OPENSPLICE_CORE_COND_FUNCTOR_HOLDER_HPP_
#define ORG_OPENSPLICE_CORE_COND_FUNCTOR_HOLDER_HPP_

namespace dds
{
namespace core
{
namespace cond
{
template <typename DELEGATE> class TCondition;
}
}
}

namespace org
{
namespace opensplice
{
namespace core
{
namespace cond
{

class ConditionDelegate;

class FunctorHolderBase
{
public:
    FunctorHolderBase() { };

    virtual ~FunctorHolderBase() { };

    virtual void dispatch(dds::core::cond::TCondition<org::opensplice::core::cond::ConditionDelegate> &condition) = 0;
};

template <typename FUN>
class FunctorHolder : public FunctorHolderBase
{
public:
    /* Remove const to be able to call non-const functors. */
    FunctorHolder(FUN &functor) : myFunctor(functor)
    {
    }

    virtual ~FunctorHolder() { };

    void dispatch(dds::core::cond::TCondition<org::opensplice::core::cond::ConditionDelegate> &condition)
    {
        myFunctor(condition);
    }

private:
    FUN &myFunctor;
};

template <typename FUN>
class ConstFunctorHolder : public FunctorHolderBase
{
public:
    /* Remove const to be able to call non-const functors. */
    ConstFunctorHolder(const FUN &functor) : myFunctor(functor)
    {
    }

    virtual ~ConstFunctorHolder() { };

    void dispatch(dds::core::cond::TCondition<org::opensplice::core::cond::ConditionDelegate> &condition)
    {
        myFunctor(condition);
    }

private:
    const FUN &myFunctor;
};

}
}
}
}

#endif  /* ORG_OPENSPLICE_CORE_COND_FUNCTOR_HOLDER_HPP_ */
