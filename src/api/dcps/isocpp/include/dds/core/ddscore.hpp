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
