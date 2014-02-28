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
#ifndef OSPL_DDS_CORE_DDSCORE_HPP_
#define OSPL_DDS_CORE_DDSCORE_HPP_

/**
 * @file
 */

/*
 * OMG PSM class declaration
 */
#include <spec/dds/core/ddscore.hpp>

// Implementation

/** @internal
* @todo OSPL-3369 Hack the copy in for strings */
inline char* c_stringNew(c_base base, const std::string& str)
{
    return c_stringNew(base, str.c_str());
}

// End of implementation

#endif /* OSPL_DDS_CORE_DDSCORE_HPP_ */
