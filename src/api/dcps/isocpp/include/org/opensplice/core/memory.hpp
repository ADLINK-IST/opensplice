/*
*                         OpenSplice DDS
*
 *   This software and documentation are Copyright 2006 to TO_YEAR PrismTech
 *   Limited, its affiliated companies and licensors. All rights reserved.
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

#ifndef ORG_OPENSPLICE_CORE_MEMORY_HPP_
#define ORG_OPENSPLICE_CORE_MEMORY_HPP_

#include <dds/core/macros.hpp>
#include <dds/core/ref_traits.hpp>
#include <org/opensplice/core/config.hpp>
#include <dds/core/TEntity.hpp>

namespace org
{
namespace opensplice
{
namespace core
{

typedef dds::core::smart_ptr_traits<DDS::DomainParticipant>::ref_type   DDS_DP_REF;
typedef dds::core::smart_ptr_traits<DDS::Publisher>::ref_type           DDS_PUB_REF;
typedef dds::core::smart_ptr_traits<DDS::Subscriber>::ref_type          DDS_SUB_REF;
typedef dds::core::smart_ptr_traits<DDS::DataWriter>::ref_type          DDS_DW_REF;
typedef dds::core::smart_ptr_traits<DDS::DataReader>::ref_type          DDS_DR_REF;
typedef dds::core::smart_ptr_traits<DDS::Topic>::ref_type               DDS_TOPIC_REF;

class OMG_DDS_API DPDeleter
{
public:
    DPDeleter();
    void  operator()(DDS::DomainParticipant* dp);
    void close(DDS::DomainParticipant* dp);

private:
    bool is_closed_;
};

class OMG_DDS_API PubDeleter
{

public:
    PubDeleter(const DDS_DP_REF& dp);

    ~PubDeleter();

    void operator()(DDS::Publisher* p);
    void close(DDS::Publisher* p);

private:
    DDS_DP_REF dp_;
    bool is_closed_;
};

class OMG_DDS_API DWDeleter
{
public:
    DWDeleter(const DDS_PUB_REF& pub);
    ~DWDeleter();
    void  operator()(DDS::DataWriter* w);
    void close(DDS::DataWriter* w);

private:
    DDS_PUB_REF  pub_;
    bool is_closed_;
};

class OMG_DDS_API SubDeleter
{
public:
    SubDeleter(const DDS_DP_REF& dp);

    ~SubDeleter();

    void operator()(DDS::Subscriber* s);
    void close(DDS::Subscriber* s);
    void set_builtin()
    {
        is_builtin_ = true;
    }
    bool is_builtin()
    {
        return is_builtin_;
    }

private:
    DDS_DP_REF dp_;
    bool is_closed_;
    bool is_builtin_;
};

class OMG_DDS_API DRDeleter
{
public:
    DRDeleter(const DDS_SUB_REF& sub);
    ~DRDeleter();
    void  operator()(DDS::DataReader* r);
    void close(DDS::DataReader* r);
    void set_builtin()
    {
        is_builtin_ = true;
    }
    bool is_builtin()
    {
        return is_builtin_;
    }

private:
    DDS_SUB_REF sub_;
    bool is_closed_;
    bool is_builtin_;
};

class OMG_DDS_API TopicDeleter
{
public:
    TopicDeleter(const DDS_DP_REF& dp_);
    ~TopicDeleter();
    void  operator()(DDS::Topic* t);
    void close(DDS::Topic* t);

private:
    DDS_DP_REF dp_;
    bool is_closed_;
};

}
}
}
#endif /* ORG_OPENSPLICE_CORE_MEMORY_HPP_ */
