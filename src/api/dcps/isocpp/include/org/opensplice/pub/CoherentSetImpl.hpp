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

#ifndef ORG_OPENSPLICE_PUB_COHERENT_SET_IMPL_HPP_
#define ORG_OPENSPLICE_PUB_COHERENT_SET_IMPL_HPP_

#include <iostream>

namespace org {
  namespace opensplice {
    namespace pub {
      class CoherentSetImpl;
    }
  }
}

/** @bug OSPL-1741 This class is not implememted
 * @see http://jira.prismtech.com:8080/browse/OSPL-1741 */

class org::opensplice::pub::CoherentSetImpl {
public:
    CoherentSetImpl() : t_(dds::core::null), ended_(false) { }

    CoherentSetImpl(const dds::pub::Publisher& t) : t_(t), ended_(false) {
        std::cout << "=== begin coherent changes ===" << std::endl;
    }

    void end() {
        if (!ended_) {
            std::cout << "=== end coherent changes ===" << std::endl;
            ended_ = true;
        }
    }

    ~CoherentSetImpl() {
        if (!ended_) {
            this->end();
        }
    }

    bool operator ==(const CoherentSetImpl& other) const
    {
        return t_ == other.t_ && ended_ == other.ended_;
    }

private:
    dds::pub::Publisher t_;
    bool ended_;
};

#endif /* OMG_IDDS_PUB_COHERENT_SET_IMPL_HPP_ */
