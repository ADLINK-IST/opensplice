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


public final class DataWriterQos 
{
  public DDS.DurabilityQosPolicy durability = null;
  public DDS.DeadlineQosPolicy deadline = null;
  public DDS.LatencyBudgetQosPolicy latency_budget = null;
  public DDS.LivelinessQosPolicy liveliness = null;
  public DDS.ReliabilityQosPolicy reliability = null;
  public DDS.DestinationOrderQosPolicy destination_order = null;
  public DDS.HistoryQosPolicy history = null;
  public DDS.ResourceLimitsQosPolicy resource_limits = null;
  public DDS.TransportPriorityQosPolicy transport_priority = null;
  public DDS.LifespanQosPolicy lifespan = null;
  public DDS.UserDataQosPolicy user_data = null;
  public DDS.OwnershipQosPolicy ownership = null;
  public DDS.OwnershipStrengthQosPolicy ownership_strength = null;
  public DDS.WriterDataLifecycleQosPolicy writer_data_lifecycle = null;

  public DataWriterQos ()
  {
  } // ctor

  public DataWriterQos (DDS.DurabilityQosPolicy _durability, DDS.DeadlineQosPolicy _deadline, DDS.LatencyBudgetQosPolicy _latency_budget, DDS.LivelinessQosPolicy _liveliness, DDS.ReliabilityQosPolicy _reliability, DDS.DestinationOrderQosPolicy _destination_order, DDS.HistoryQosPolicy _history, DDS.ResourceLimitsQosPolicy _resource_limits, DDS.TransportPriorityQosPolicy _transport_priority, DDS.LifespanQosPolicy _lifespan, DDS.UserDataQosPolicy _user_data, DDS.OwnershipQosPolicy _ownership, DDS.OwnershipStrengthQosPolicy _ownership_strength, DDS.WriterDataLifecycleQosPolicy _writer_data_lifecycle)
  {
    durability = _durability;
    deadline = _deadline;
    latency_budget = _latency_budget;
    liveliness = _liveliness;
    reliability = _reliability;
    destination_order = _destination_order;
    history = _history;
    resource_limits = _resource_limits;
    transport_priority = _transport_priority;
    lifespan = _lifespan;
    user_data = _user_data;
    ownership = _ownership;
    ownership_strength = _ownership_strength;
    writer_data_lifecycle = _writer_data_lifecycle;
  } // ctor

} // class DataWriterQos
