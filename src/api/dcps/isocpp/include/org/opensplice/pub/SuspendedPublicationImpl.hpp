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

#ifndef ORG_OPENSPLICE_PUB_SUSPENDED_PUBBLICATIONS_IMPL_HPP_
#define ORG_OPENSPLICE_PUB_SUSPENDED_PUBBLICATIONS_IMPL_HPP_

#include <iostream>
#include <org/opensplice/core/exception_helper.hpp>

namespace org
{
namespace opensplice
{
namespace pub
{
class SuspendedPublicationImpl;
}
}
}

/** @internal @bug OSPL-1741 This class is not implememted
 * @see http://jira.prismtech.com:8080/browse/OSPL-1741 */

class org::opensplice::pub::SuspendedPublicationImpl
{
public:
    SuspendedPublicationImpl() : t_(dds::core::null), ended_(false)
    {
        throw dds::core::UnsupportedError(org::opensplice::core::exception_helper(
                                              OSPL_CONTEXT_LITERAL("dds::core::UnsupportedError : Function not currently supported")));
    }

    SuspendedPublicationImpl(const dds::pub::Publisher& t) : t_(t), ended_(false)
    {
#ifdef _WIN32
#pragma warning( push )
#pragma warning( disable : 4702 ) //disable warning caused by temporary exception, remove later
#endif
        throw dds::core::UnsupportedError(org::opensplice::core::exception_helper(
                                              OSPL_CONTEXT_LITERAL("dds::core::UnsupportedError : Function not currently supported")));
#ifdef _WIN32
#pragma warning ( pop ) //re-enable warning to prevent leaking to user code, remove later
#endif

        std::cout << "=== suspend publication" << std::endl;
    }

    void end()
    {
#ifdef _WIN32
#pragma warning( push )
#pragma warning( disable : 4702 ) //disable warning caused by temporary exception, remove later
#endif
        throw dds::core::UnsupportedError(org::opensplice::core::exception_helper(
                                              OSPL_CONTEXT_LITERAL("dds::core::UnsupportedError : Function not currently supported")));
#ifdef _WIN32
#pragma warning ( pop ) //re-enable warning to prevent leaking to user code, remove later
#endif

        if(!ended_)
        {
            std::cout << "=== resume publication" << std::endl;
            ended_ = true;
        }
    }

    ~SuspendedPublicationImpl()
    {
        if(!ended_)
        {
            this->end();
        }
    }

    bool operator ==(const SuspendedPublicationImpl& other) const
    {
        return t_ == other.t_ && ended_ == other.ended_;
    }

private:
    dds::pub::Publisher t_;
    bool ended_;
};

#endif /* ORG_OPENSPLICE_PUB_SUSPENDED_PUBBLICATIONS_IMPL_HPP_ */
