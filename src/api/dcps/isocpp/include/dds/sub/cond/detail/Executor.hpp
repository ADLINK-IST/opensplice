/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR PrismTech
 *   Limited, its affiliated companies and licensors. All rights reserved.
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
