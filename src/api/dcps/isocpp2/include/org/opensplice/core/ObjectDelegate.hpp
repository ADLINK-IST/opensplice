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

#ifndef ORG_OPENSPLICE_CORE_OBJECT_DELEGATE_HPP_
#define ORG_OPENSPLICE_CORE_OBJECT_DELEGATE_HPP_

#include "dds/core/macros.hpp"
#include "dds/core/refmacros.hpp"
#include "org/opensplice/core/Mutex.hpp"

#include "vortex_os.h"
#include "u_types.h"

namespace org
{
namespace opensplice
{
namespace core
{

class OMG_DDS_API ObjectDelegate
{
public:
    typedef ::dds::core::smart_ptr_traits< ObjectDelegate >::ref_type ref_type;
    typedef ::dds::core::smart_ptr_traits< ObjectDelegate >::weak_ref_type weak_ref_type;

    ObjectDelegate();
    virtual ~ObjectDelegate();

    virtual void close();

    virtual void init(ObjectDelegate::weak_ref_type weak_ref) = 0;

    ObjectDelegate::weak_ref_type get_weak_ref() const;
    ObjectDelegate::ref_type get_strong_ref() const;

    int32_t get_domain_id() const {
        return domainId;
    }

protected:
    Mutex mutex;
    bool closed;
    int32_t domainId;

    void check() const;
    void set_weak_ref(ObjectDelegate::weak_ref_type weak_ref);

    ObjectDelegate::weak_ref_type myself;

    void set_domain_id(int32_t id) {
         domainId = id;
    }

public:
    void lock() const;
    void unlock() const;
};




}
}
}

#endif /* ORG_OPENSPLICE_CORE_OBJECT_DELEGATE_HPP_ */
