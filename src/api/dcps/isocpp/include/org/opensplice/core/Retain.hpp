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

