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
#ifndef OSPL_DDS_SUB_DETAIL_FIND_HPP_
#define OSPL_DDS_SUB_DETAIL_FIND_HPP_

/**
 * @file
 */

// Implementation
#include <string>
#include <vector>

#include <dds/sub/Subscriber.hpp>
#include <dds/sub/status/DataState.hpp>
#include <dds/topic/TopicDescription.hpp>

#include <org/opensplice/core/EntityRegistry.hpp>
#include <org/opensplice/core/RegisterBuiltinTopics.hpp>
#include <org/opensplice/topic/qos/QosConverter.hpp>

namespace dds
{
namespace sub
{
namespace detail
{

template <typename READER, typename FwdIterator>
uint32_t
find(const dds::sub::Subscriber& sub,
     const std::string topic_name,
     FwdIterator begin, uint32_t max_size)
{
    DDS::DataReader_ptr ddsdr = sub->sub_.get()->lookup_datareader(topic_name.c_str());
    if(ddsdr)
    {
        READER dr = org::opensplice::core::EntityRegistry<DDS::DataReader_ptr, READER >::get(ddsdr);
        if(max_size > 0)
        {
            if(dr != dds::core::null)
            {
                *begin = dr;
            }
            else if(sub->is_builtin())
            {
                DDS::DataReaderQos drqos;
                ddsdr->get_qos(drqos);
                dds::topic::Topic<typename READER::DataType> topic(dds::core::null);
                dr = READER(sub, topic, org::opensplice::sub::qos::convertQos(drqos));
                org::opensplice::core::EntityRegistry<DDS::DataReader_ptr, READER >::remove(dr->get_raw_reader());
                dr->init_builtin(ddsdr, ddsdr->get_topicdescription());
                dr.retain();
                org::opensplice::core::EntityRegistry<DDS::DataReader_ptr, READER >::insert(ddsdr, dr);

                *begin = dr;
            }
            return 1;
        }
    }
    return 0;
}

template <typename READER, typename BinIterator>
uint32_t
find(const dds::sub::Subscriber& sub,
     const std::string topic_name,
     BinIterator begin)
{
    DDS::DataReader_ptr ddsdr = sub->sub_.get()->lookup_datareader(topic_name.c_str());
    if(ddsdr)
    {
        READER dr = org::opensplice::core::EntityRegistry<DDS::DataReader_ptr, READER >::get(ddsdr);
        if(dr != dds::core::null)
        {
            *begin = dr;
        }
        else if(sub->is_builtin())
        {
            DDS::DataReaderQos drqos;
            ddsdr->get_qos(drqos);
            dds::topic::Topic<typename READER::DataType> topic(dds::core::null);
            dr = READER(sub, topic, org::opensplice::sub::qos::convertQos(drqos));
            org::opensplice::core::EntityRegistry<DDS::DataReader_ptr, READER >::remove(dr->get_raw_reader());
            dr->init_builtin(ddsdr, ddsdr->get_topicdescription());
            dr.retain();
            org::opensplice::core::EntityRegistry<DDS::DataReader_ptr, READER >::insert(ddsdr, dr);

            *begin = dr;
        }
        return 1;
    }
    return 0;
}

template <typename READER, typename T, typename FwdIterator>
uint32_t
find(const dds::sub::Subscriber& sub,
     const dds::topic::TopicDescription<T>& topic_description,
     FwdIterator begin, uint32_t max_size)
{
    DDS::DataReader_ptr ddsdr = sub->sub_.get()->lookup_datareader(topic_description.name().c_str());
    if(ddsdr)
    {
        READER dr = org::opensplice::core::EntityRegistry<DDS::DataReader_ptr, READER>::get(ddsdr);
        if(dr != dds::core::null && max_size > 0)
        {
            *begin = dr;
        }
        else if(sub->is_builtin())
        {
            DDS::DataReaderQos drqos;
            ddsdr->get_qos(drqos);
            dds::topic::Topic<typename READER::DataType> topic(dds::core::null);
            dr = READER(sub, topic, org::opensplice::sub::qos::convertQos(drqos));
            org::opensplice::core::EntityRegistry<DDS::DataReader_ptr, READER >::remove(dr->get_raw_reader());
            dr->init_builtin(ddsdr, ddsdr->get_topicdescription());
            dr.retain();
            org::opensplice::core::EntityRegistry<DDS::DataReader_ptr, READER >::insert(ddsdr, dr);

            *begin = dr;
        }
        return 1;
    }
    return 0;
}

template <typename READER, typename T, typename BinIterator>
uint32_t
find(const dds::sub::Subscriber& sub,
     const dds::topic::TopicDescription<T>& topic_description,
     BinIterator begin)
{
    DDS::DataReader_ptr ddsdr = sub->sub_.get()->lookup_datareader(topic_description.name().c_str());
    if(ddsdr)
    {
        READER dr = org::opensplice::core::EntityRegistry<DDS::DataReader_ptr, READER>::get(ddsdr);
        if(dr != dds::core::null)
        {
            *begin = dr;
        }
        else if(sub->is_builtin())
        {
            DDS::DataReaderQos drqos;
            ddsdr->get_qos(drqos);
            dds::topic::Topic<typename READER::DataType> topic(dds::core::null);
            dr = READER(sub, topic, org::opensplice::sub::qos::convertQos(drqos));
            org::opensplice::core::EntityRegistry<DDS::DataReader_ptr, READER >::remove(dr->get_raw_reader());
            dr->init_builtin(ddsdr, ddsdr->get_topicdescription());
            dr.retain();
            org::opensplice::core::EntityRegistry<DDS::DataReader_ptr, READER >::insert(ddsdr, dr);

            *begin = dr;
        }
        return 1;
    }
    return 0;
}

template <typename READER, typename FwdIterator>
uint32_t
find(const dds::sub::Subscriber& sub,
     const dds::sub::status::DataState& rs,
     FwdIterator begin, uint32_t max_size)
{
    DDS::DataReaderSeq ddsdrseq;
    sub->sub_.get()->get_datareaders(
        ddsdrseq, rs.sample_state().to_ulong(), rs.view_state().to_ulong(), rs.instance_state().to_ulong());
    for(int i = 0; i < ddsdrseq.length() && i < max_size; i++)
    {
        READER dr = org::opensplice::core::EntityRegistry<DDS::DataReader_ptr, READER>::get(ddsdrseq[i]);
        if(dr != dds::core::null)
        {
            *begin = dr;
            begin++;
        }
        else if(sub->is_builtin())
        {
            DDS::DataReaderQos drqos;
            ddsdrseq[i]->get_qos(drqos);
            dds::topic::Topic<typename READER::DataType> topic(dds::core::null);
            dr = READER(sub, topic, org::opensplice::sub::qos::convertQos(drqos));
            org::opensplice::core::EntityRegistry<DDS::DataReader_ptr, READER >::remove(dr->get_raw_reader());
            dr->init_builtin(ddsdrseq[i], ddsdrseq[i]->get_topicdescription());
            dr.retain();
            org::opensplice::core::EntityRegistry<DDS::DataReader_ptr, READER >::insert(ddsdrseq[i], dr);

            *begin = dr;
            begin++;
        }
    }
    return ddsdrseq.length();
}

template <typename READER, typename BinIterator>
uint32_t
find(const dds::sub::Subscriber& sub,
     const dds::sub::status::DataState& rs,
     BinIterator begin)
{
    DDS::DataReaderSeq ddsdrseq;
    sub->sub_.get()->get_datareaders(
        ddsdrseq, rs.sample_state().to_ulong(), rs.view_state().to_ulong(), rs.instance_state().to_ulong());
    for(int i = 0; i < ddsdrseq.length(); i++)
    {
        READER dr = org::opensplice::core::EntityRegistry<DDS::DataReader_ptr, READER>::get(ddsdrseq[i]);
        if(dr != dds::core::null)
        {
            *begin = dr;
            begin++;
        }
        else if(sub->is_builtin())
        {
            DDS::DataReaderQos drqos;
            ddsdrseq[i]->get_qos(drqos);
            dds::topic::Topic<typename READER::DataType> topic(dds::core::null);
            dr = READER(sub, topic, org::opensplice::sub::qos::convertQos(drqos));
            org::opensplice::core::EntityRegistry<DDS::DataReader_ptr, READER >::remove(dr->get_raw_reader());
            dr->init_builtin(ddsdrseq[i], ddsdrseq[i]->get_topicdescription());
            dr.retain();
            org::opensplice::core::EntityRegistry<DDS::DataReader_ptr, READER >::insert(ddsdrseq[i], dr);

            *begin = dr;
            begin++;
        }
    }
    return ddsdrseq.length();
}

}
}
}

#endif /* OSPL_DDS_SUB_DETAIL_FIND_HPP_ */

