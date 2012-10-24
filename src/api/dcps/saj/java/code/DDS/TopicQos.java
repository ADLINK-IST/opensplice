/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2011 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE 
 *
 *   for full copyright notice and license terms. 
 *
 */

package DDS;


public final class TopicQos 
{
  public DDS.TopicDataQosPolicy topic_data = null;
  public DDS.DurabilityQosPolicy durability = null;
  public DDS.DurabilityServiceQosPolicy durability_service = null;
  public DDS.DeadlineQosPolicy deadline = null;
  public DDS.LatencyBudgetQosPolicy latency_budget = null;
  public DDS.LivelinessQosPolicy liveliness = null;
  public DDS.ReliabilityQosPolicy reliability = null;
  public DDS.DestinationOrderQosPolicy destination_order = null;
  public DDS.HistoryQosPolicy history = null;
  public DDS.ResourceLimitsQosPolicy resource_limits = null;
  public DDS.TransportPriorityQosPolicy transport_priority = null;
  public DDS.LifespanQosPolicy lifespan = null;
  public DDS.OwnershipQosPolicy ownership = null;

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
