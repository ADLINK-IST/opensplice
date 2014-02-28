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

#ifndef ORG_OPENSPLICE_DOMAIN_DOMAIN_HPP_
#define ORG_OPENSPLICE_DOMAIN_DOMAIN_HPP_

#include <org/opensplice/core/config.hpp>

namespace org
{
namespace opensplice
{
namespace domain
{
inline uint32_t any_id()
{
    return 0;
}

inline uint32_t default_id()
{
    return DDS::DOMAIN_ID_DEFAULT;
}

OSPL_ISOCPP_IMPL_API void configure(uint32_t, int, char*[]);
}
}
}

#endif /* ORG_OPENSPLICE_DOMAIN_DOMAIN_HPP_ */
