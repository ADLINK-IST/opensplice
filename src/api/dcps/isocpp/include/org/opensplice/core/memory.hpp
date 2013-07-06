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

#ifndef ORG_OPENSPLICE_CORE_MEMORY_HPP_
#define ORG_OPENSPLICE_CORE_MEMORY_HPP_

#include <dds/core/macros.hpp>
#include <dds/core/ref_traits.hpp>
#include <org/opensplice/core/config.hpp>

namespace org { namespace opensplice { namespace core {

typedef dds::core::smart_ptr_traits<DDS::DomainParticipant>::ref_type   DDS_DP_REF;
typedef dds::core::smart_ptr_traits<DDS::Publisher>::ref_type           DDS_PUB_REF;
typedef dds::core::smart_ptr_traits<DDS::Subscriber>::ref_type          DDS_SUB_REF;
typedef dds::core::smart_ptr_traits<DDS::DataWriter>::ref_type          DDS_DW_REF;
typedef dds::core::smart_ptr_traits<DDS::DataReader>::ref_type          DDS_DR_REF;
typedef dds::core::smart_ptr_traits<DDS::Topic>::ref_type               DDS_TOPIC_REF;

class OMG_DDS_API DPDeleter {
public:
    void  operator()(DDS::DomainParticipant* dp);
};


// The following class should be written using boost::bind

class OMG_DDS_API PubDeleter {

public:
    PubDeleter(const DDS_DP_REF& dp);

    ~PubDeleter();

    void operator()(DDS::Publisher* p);

private:
    DDS_DP_REF dp_;
};


class OMG_DDS_API DWDeleter {
public:
    DWDeleter(const DDS_PUB_REF& pub);
    ~DWDeleter();
    void  operator()(DDS::DataWriter* w);

private:
    DDS_PUB_REF  pub_;
};



class OMG_DDS_API SubDeleter {
public:
    SubDeleter(const DDS_DP_REF& dp);

    ~SubDeleter();

    void operator()(DDS::Subscriber* s);

private:
    DDS_DP_REF dp_;
};


class OMG_DDS_API DRDeleter {
public:
    DRDeleter(const DDS_SUB_REF& sub);
    ~DRDeleter();
    void  operator()(DDS::DataReader* r);

private:
    DDS_SUB_REF sub_;
};

class OMG_DDS_API TopicDeleter {
public:
    TopicDeleter(const DDS_DP_REF& dp_);
    ~TopicDeleter();
    void  operator()(DDS::Topic* t);

private:
    DDS_DP_REF dp_;
};

} } }
#endif /* ORG_OPENSPLICE_CORE_MEMORY_HPP_ */
