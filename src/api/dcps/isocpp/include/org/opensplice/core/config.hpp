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

#ifndef ORG_OPENSPLICE_CORE_CONFIG_HPP_
#define ORG_OPENSPLICE_CORE_CONFIG_HPP_

#ifndef OPENSPLICE_ISOCXX_PSM
#define OPENSPLICE_ISOCXX_PSM
#endif

/* Includes necessary for pulling in legacy C++ API */
#include <dds_dcps.h>
#include <ccpp_dds_dcps.h>

#include <dds/core/macros.hpp>
#include <dds/core/types.hpp>

/** @internal Using a separate macro for org::opensplice in case we want to separate libs
later */
#define OSPL_ISOCPP_IMPL_API OMG_DDS_API

#endif /* ORG_OPENSPLICE_CORE_CONFIG_HPP_ */
