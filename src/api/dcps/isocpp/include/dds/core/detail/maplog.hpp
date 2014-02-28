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
#ifndef OSPL_DDS_CORE_DETAIL_MAPLOG_HPP_
#define OSPL_DDS_CORE_DETAIL_MAPLOG_HPP_

#include <dds/core/detail/macros.hpp>
#include <string>
#include <os_report.h>

namespace dds
{
namespace core
{
namespace detail
{

os_reportType OMG_DDS_API_DETAIL maplog(const std::string& kind);

}
}
}

#endif
