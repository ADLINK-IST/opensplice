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
#ifndef OSPL_DDS_SUB_DETAIL_EXECUTOR_HPP_
#define OSPL_DDS_SUB_DETAIL_EXECUTOR_HPP_

/**
 * @file
 */

// Implementation

namespace dds
{
namespace sub
{
namespace cond
{
namespace detail
{
class Executor
{
public:
    virtual ~Executor() { }

    virtual void exec() = 0;
};

class TrivialExecutor : public Executor
{
public:
    virtual ~TrivialExecutor() { }
    virtual void exec() { }
};

template <typename FUN, typename ARG>
class ParametrizedExecutor : public Executor
{
public:
    ParametrizedExecutor(const FUN& fun, const ARG& arg) : fun_(fun), arg_(arg) { }
    virtual ~ParametrizedExecutor() { }
    virtual void exec()
    {
        fun_(arg_);
    }

private:
    FUN fun_;
    ARG arg_;
};
}
}
}
}

// End of implementation

#endif /* OSPL_DDS_SUB_DETAIL_EXECUTOR_HPP_ */
