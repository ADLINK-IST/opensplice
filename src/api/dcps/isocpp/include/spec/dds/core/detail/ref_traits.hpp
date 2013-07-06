#ifndef OMG_DDS_CORE_DETAIL_REF_TRAITS_H_
#define OMG_DDS_CORE_DETAIL_REF_TRAITS_H_

/* Copyright 2010, Object Management Group, Inc.
 * Copyright 2010, PrismTech, Corp.
 * Copyright 2010, Real-Time Innovations, Inc.
 * All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */


/*
 * This file is non-normative. The implementation is
 * provided only as an example.
 */
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
#include <boost/static_assert.hpp>
#include <boost/type_traits.hpp>

#include <dds/core/types.hpp>
#include <dds/core/Exception.hpp>

template <typename T1, typename T2>
struct dds::core::is_base_of : public boost::is_base_of<T1, T2> { };

template <typename T1, typename T2>
struct dds::core::is_same : public boost::is_same<T1, T1> { };

template <typename T>
struct dds::core::smart_ptr_traits {
    typedef boost::shared_ptr<T> ref_type;
    typedef boost::weak_ptr<T>   weak_ref_type;
};


template <typename TO, typename FROM>
TO dds::core::polymorphic_cast(FROM& from) {
    typename TO::DELEGATE_REF_T dr =
            boost::dynamic_pointer_cast< typename TO::DELEGATE_T>(from.delegate());
    TO to(dr);

    if (to == dds::core::null)
        throw dds::core::InvalidDowncastError("Attempted invalid downcast.");
    return to;
}
#endif /* OMG_DDS_CORE_DETAIL_REF_TRAITS_H_ */
