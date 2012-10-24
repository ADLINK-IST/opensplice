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


public final class DataReaderQos 
{
  public DDS.DurabilityQosPolicy durability = null;
  public DDS.DeadlineQosPolicy deadline = null;
  public DDS.LatencyBudgetQosPolicy latency_budget = null;
  public DDS.LivelinessQosPolicy liveliness = null;
  public DDS.ReliabilityQosPolicy reliability = null;
  public DDS.DestinationOrderQosPolicy destination_order = null;
  public DDS.HistoryQosPolicy history = null;
  public DDS.ResourceLimitsQosPolicy resource_limits = null;
  public DDS.UserDataQosPolicy user_data = null;
  public DDS.OwnershipQosPolicy ownership = null;
  public DDS.TimeBasedFilterQosPolicy time_based_filter = null;
  public DDS.ReaderDataLifecycleQosPolicy reader_data_lifecycle = null;
  public DDS.ShareQosPolicy share = null;
  public DDS.ReaderLifespanQosPolicy reader_lifespan = null;
  public DDS.SubscriptionKeyQosPolicy subscription_keys = null;

  /**
   * Returns the default DataReaderQos.
   */
  public DataReaderQos ()
  {
  } // ctor

  public DataReaderQos (DDS.DurabilityQosPolicy _durability, DDS.DeadlineQosPolicy _deadline, DDS.LatencyBudgetQosPolicy _latency_budget, DDS.LivelinessQosPolicy _liveliness, DDS.ReliabilityQosPolicy _reliability, DDS.DestinationOrderQosPolicy _destination_order, DDS.HistoryQosPolicy _history, DDS.ResourceLimitsQosPolicy _resource_limits, DDS.UserDataQosPolicy _user_data, DDS.OwnershipQosPolicy _ownership, DDS.TimeBasedFilterQosPolicy _time_based_filter, DDS.ReaderDataLifecycleQosPolicy _reader_data_lifecycle, DDS.ShareQosPolicy _share, DDS.ReaderLifespanQosPolicy _reader_lifespan, DDS.SubscriptionKeyQosPolicy _subscription_keys)
  {
    durability = _durability;
    deadline = _deadline;
    latency_budget = _latency_budget;
    liveliness = _liveliness;
    reliability = _reliability;
    destination_order = _destination_order;
    history = _history;
    resource_limits = _resource_limits;
    user_data = _user_data;
    ownership = _ownership;
    time_based_filter = _time_based_filter;
    reader_data_lifecycle = _reader_data_lifecycle;
    share = _share;
    reader_lifespan = _reader_lifespan;
    subscription_keys = _subscription_keys;
  } // ctor

} // class DataReaderQos
