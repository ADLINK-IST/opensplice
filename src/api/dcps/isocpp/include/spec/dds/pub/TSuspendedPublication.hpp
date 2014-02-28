#ifndef OMG_TDDS_PUB_SUSPENDED_PUBLICATION_HPP_
#define OMG_TDDS_PUB_SUSPENDED_PUBLICATION_HPP_

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

#include <dds/pub/Publisher.hpp>

namespace dds
{
namespace pub
{
template <typename DELEGATE>
class TSuspendedPublication;
}
}

template <typename DELEGATE>
class dds::pub::TSuspendedPublication : public dds::core::Value<DELEGATE>
{
public:
    /**
     * This operation indicates to the Service that the application is about
     * to make multiple modifications using DataWriter objects belonging to
     * the Publisher.
     * It is a hint to the Service so it can optimize its performance by,
     * e.g., holding the dissemination of the modifications and then batching
     * them. It is not required that the Service use this hint in any way.
     * The use of this operation must be matched by a corresponding call to
     * resume_publications indicating that the set of modifications has
     * completed. If the Publisher is deleted before resume_publications
     * is called, any suspended updates yet to be published will be discarded.
     *
     * @return true if publications were suspended, false if the function
     *         was called on an already-suspended publisher
     */
    explicit TSuspendedPublication(const dds::pub::Publisher& pub);

public:
    /**
     * This operation indicates to the Service that the application has
     * completed the multiple changes initiated by the previous
     * suspend_publications. This is a hint to the Service that can be used
     * by a Service implementation to, e.g., batch all the modifications made
     * since the suspend_publications.
     * The call to resume_publications must match a previous call to
     * suspend_publications. Otherwise the operation will return the
     * error PRECONDITION_NOT_MET.
     */
    void resume();           // resumes publications explicitly

public:
    /**
     * This operation indicates to the Service that the application has
     * completed the multiple changes initiated by the previous
     * suspend_publications. This is a hint to the Service that can be used
     * by a Service implementation to, e.g., batch all the modifications made
     * since the suspend_publications.
     * The call to resume_publications must match a previous call to
     * suspend_publications. Otherwise the operation will return the
     * error PRECONDITION_NOT_MET.
     */
    ~TSuspendedPublication();    // resumes publications implicitly
};


#endif /* OMG_TDDS_PUB_SUSPENDED_PUBLICATION_HPP_ */
