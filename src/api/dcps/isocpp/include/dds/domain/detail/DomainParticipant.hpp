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
#ifndef OSPL_DDS_DOMAIN_DETAIL_DOMAINPARTICIPANT_HPP_
#define OSPL_DDS_DOMAIN_DETAIL_DOMAINPARTICIPANT_HPP_

/**
 * @file
 */

// Implementation

#include <dds/domain/TDomainParticipant.hpp>
#include <org/opensplice/domain/DomainParticipantDelegate.hpp>

namespace dds
{
namespace domain
{
namespace detail
{
typedef dds::domain::TDomainParticipant< org::opensplice::domain::DomainParticipantDelegate>
DomainParticipant;
}
}
}

// End of implementation

#endif /* OSPL_DDS_DOMAIN_DETAIL_DOMAINPARTICIPANT_HPP_ */
