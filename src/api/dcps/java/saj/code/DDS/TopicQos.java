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

package DDS;


public final class TopicQos 
{
  public DDS.TopicDataQosPolicy topic_data = new DDS.TopicDataQosPolicy();
  public DDS.DurabilityQosPolicy durability = new DDS.DurabilityQosPolicy();
  public DDS.DurabilityServiceQosPolicy durability_service = new DDS.DurabilityServiceQosPolicy();
  public DDS.DeadlineQosPolicy deadline = new DDS.DeadlineQosPolicy();
  public DDS.LatencyBudgetQosPolicy latency_budget = new DDS.LatencyBudgetQosPolicy();
  public DDS.LivelinessQosPolicy liveliness = new DDS.LivelinessQosPolicy();
  public DDS.ReliabilityQosPolicy reliability = new DDS.ReliabilityQosPolicy();
  public DDS.DestinationOrderQosPolicy destination_order = new DDS.DestinationOrderQosPolicy();
  public DDS.HistoryQosPolicy history = new DDS.HistoryQosPolicy();
  public DDS.ResourceLimitsQosPolicy resource_limits = new DDS.ResourceLimitsQosPolicy();
  public DDS.TransportPriorityQosPolicy transport_priority = new DDS.TransportPriorityQosPolicy();
  public DDS.LifespanQosPolicy lifespan = new DDS.LifespanQosPolicy();
  public DDS.OwnershipQosPolicy ownership = new DDS.OwnershipQosPolicy();

  public TopicQos ()
  {
  } // ctor

  public TopicQos (
          DDS.TopicDataQosPolicy _topic_data,
          DDS.DurabilityQosPolicy _durability,
          DDS.DurabilityServiceQosPolicy _durability_service,
          DDS.DeadlineQosPolicy _deadline,
          DDS.LatencyBudgetQosPolicy _latency_budget,
          DDS.LivelinessQosPolicy _liveliness,
          DDS.ReliabilityQosPolicy _reliability,
          DDS.DestinationOrderQosPolicy _destination_order,
          DDS.HistoryQosPolicy _history,
          DDS.ResourceLimitsQosPolicy _resource_limits,
          DDS.TransportPriorityQosPolicy _transport_priority,
          DDS.LifespanQosPolicy _lifespan,
          DDS.OwnershipQosPolicy _ownership)
  {
    topic_data = _topic_data;
    durability = _durability;
    durability_service = _durability_service;
    deadline = _deadline;
    latency_budget = _latency_budget;
    liveliness = _liveliness;
    reliability = _reliability;
    destination_order = _destination_order;
    history = _history;
    resource_limits = _resource_limits;
    transport_priority = _transport_priority;
    lifespan = _lifespan;
    ownership = _ownership;
  } // ctor

} // class TopicQos
