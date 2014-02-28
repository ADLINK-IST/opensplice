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
#ifndef OSPL_DDS_PUB_DATAWRITERLISTENER_HPP_
#define OSPL_DDS_PUB_DATAWRITERLISTENER_HPP_

/**
 * @file
 */

/*
 * OMG PSM class declaration
 */
#include <spec/dds/pub/DataWriterListener.hpp>

// Implementation

namespace dds
{
namespace pub
{

template <typename T>
NoOpDataWriterListener<T>::~NoOpDataWriterListener() { }

template <typename T>
void NoOpDataWriterListener<T>::on_offered_deadline_missed(dds::pub::DataWriter<T>& writer,
        const dds::core::status::OfferedDeadlineMissedStatus& status) { }

template <typename T>
void NoOpDataWriterListener<T>::on_offered_incompatible_qos(dds::pub::DataWriter<T>& writer,
        const dds::core::status::OfferedIncompatibleQosStatus&  status) { }

template <typename T>
void NoOpDataWriterListener<T>::on_liveliness_lost(dds::pub::DataWriter<T>& writer,
        const dds::core::status::LivelinessLostStatus& status) { }

template <typename T>
void NoOpDataWriterListener<T>::on_publication_matched(dds::pub::DataWriter<T>& writer,
        const dds::core::status::PublicationMatchedStatus& status) { }
}
}

// End of implementation

#endif /* OSPL_DDS_PUB_DATAWRITERLISTENER_HPP_ */
