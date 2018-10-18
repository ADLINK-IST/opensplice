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

#ifndef DDS__QOS_H
#define DDS__QOS_H

#include <dds_dcps.h>
#include <dds.h>

void
dds_qos_from_participant_qos(
    dds_qos_t * restrict dst,
    const DDS_DomainParticipantQos * restrict src);

void
dds_qos_from_topic_qos(
    dds_qos_t * restrict dst,
    const DDS_TopicQos * restrict src);

void
dds_qos_from_publisher_qos(
    dds_qos_t * restrict dst,
    const DDS_PublisherQos * restrict src);

void
dds_qos_from_subscriber_qos(
    dds_qos_t * restrict dst,
    const DDS_SubscriberQos * restrict src);

void
dds_qos_from_writer_qos(
    dds_qos_t * restrict dst,
    const DDS_DataWriterQos * restrict src);

void
dds_qos_from_reader_qos(
    dds_qos_t * restrict dst,
    const DDS_DataReaderQos * restrict src);

void
dds_qos_to_participant_qos(
    DDS_DomainParticipantQos * restrict dst,
    const dds_qos_t * restrict src);

void
dds_qos_to_topic_qos(
    DDS_TopicQos * restrict dst,
    const dds_qos_t * restrict src);

void
dds_qos_to_publisher_qos(
    DDS_PublisherQos * restrict dst,
    const dds_qos_t * restrict src);

void
dds_qos_to_subscriber_qos(
    DDS_SubscriberQos * restrict dst,
    const dds_qos_t * restrict src);

void
dds_qos_to_writer_qos(
    DDS_DataWriterQos * restrict dst,
    const dds_qos_t * restrict src);

void
dds_qos_to_reader_qos(
    DDS_DataReaderQos * restrict dst,
    const dds_qos_t * restrict src);

#endif /* DDS__QOS_H */
