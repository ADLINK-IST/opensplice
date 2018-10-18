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

#ifndef ORG_OPENSPLICE_CORE_FUNCTOR_HOLDER_HPP
#define ORG_OPENSPLICE_CORE_FUNCTOR_HOLDER_HPP

#include <org/opensplice/core/config.hpp>

namespace org
{
namespace opensplice
{
namespace core
{
class FunctorHolder;

template <typename Functor>
class VoidFunctorHolder;

template <typename Functor, typename ARG1>
class OneArgFunctorHolder;

template <typename Functor, typename ARG1, typename ARG2>
class TwoArgFunctorHolder;
}
}
}

class OSPL_ISOCPP_IMPL_API org::opensplice::core::FunctorHolder
{
public:
    virtual ~FunctorHolder();
    virtual void invoke() = 0;
};

template <typename Functor>
class org::opensplice::core::VoidFunctorHolder :
    public org::opensplice::core::FunctorHolder
{
public:
    VoidFunctorHolder(const Functor& func)
        : func_(func) { }

    virtual ~VoidFunctorHolder() { }

public:
    virtual void invoke()
    {
        func_();
    }
private:
    Functor func_;
};

template <typename Functor, typename ARG1>
class org::opensplice::core::OneArgFunctorHolder :
    public org::opensplice::core::FunctorHolder
{
public:
    OneArgFunctorHolder(const Functor& func, const ARG1& arg1)
        : func_(func), arg1_(arg1) { }

    virtual ~OneArgFunctorHolder() { }

public:
    virtual void invoke()
    {
        func_(arg1_);
    }
private:
    Functor func_;
    ARG1 arg1_;
};

template <typename Functor, typename ARG1, typename ARG2>
class org::opensplice::core::TwoArgFunctorHolder :
    public org::opensplice::core::FunctorHolder
{
public:
    TwoArgFunctorHolder(const Functor& func,
                        const ARG1& arg1,
                        const ARG2& arg2)
        :  func_(func),
           arg1_(arg1),
           arg2_(arg2)
    { }

    virtual ~TwoArgFunctorHolder() { }

public:
    virtual void invoke()
    {
        func_(arg1_, arg2_);
    }
private:
    Functor func_;
    ARG1 arg1_;
    ARG2 arg2_;
};

#endif /* ORG_OPENSPLICE_CORE_FUNCTOR_HOLDER_HPP */
