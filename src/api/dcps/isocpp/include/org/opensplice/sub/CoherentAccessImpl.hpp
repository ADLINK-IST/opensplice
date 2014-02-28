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

#ifndef ORG_OPENSPLICE_SUB_COHERENT_ACCESS_IMPL_HPP_
#define ORG_OPENSPLICE_SUB_COHERENT_ACCESS_IMPL_HPP_

namespace org
{
namespace opensplice
{
namespace sub
{

/** @internal @bug OSPL-2476 No implementation
* @todo Implementation required - see OSPL-2476
* @see http://jira.prismtech.com:8080/browse/OSPL-2476 */
class CoherentAccessImpl
{
public:
    CoherentAccessImpl()
    {
        throw dds::core::UnsupportedError(org::opensplice::core::exception_helper(
                                              OSPL_CONTEXT_LITERAL("dds::core::UnsupportedError : Function not currently supported")));
    }

    bool operator==(const CoherentAccessImpl& other) const
    {
        //Temporary compilation fix
        return false;
    }
};

}
}
}

#endif /* ORG_OPENSPLICE_SUB_COHERENT_ACCESS_IMPL_HPP_ */
