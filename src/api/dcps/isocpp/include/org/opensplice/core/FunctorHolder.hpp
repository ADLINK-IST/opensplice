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
