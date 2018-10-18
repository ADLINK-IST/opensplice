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


/**
 * @file
 */

#ifndef ORG_OPENSPLICE_CORE_RETAIN_HPP_
#define ORG_OPENSPLICE_CORE_RETAIN_HPP_

namespace org
{
namespace opensplice
{
namespace core
{

/**
 *  @internal Adds an Entity to the retain vector, preventing it from being deleted when it goes out of scope
 * @param e The Entity to add
 */
template <typename T>
void retain_add(T& e);

/**
 *  @internal Removes an entity from the retain vector, allowing it's ref count to fall and be deleted
 * @param e The Entity to remove
 */
template <typename T>
void retain_remove(T& e);

}
}
}

#endif /* ORG_OPENSPLICE_CORE_RETAIN_HPP_ */

