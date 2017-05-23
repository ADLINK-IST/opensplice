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

package DDS;


public final class DataReaderQos
{
  public DDS.DurabilityQosPolicy durability = new DDS.DurabilityQosPolicy();
  public DDS.DeadlineQosPolicy deadline = new DDS.DeadlineQosPolicy();
  public DDS.LatencyBudgetQosPolicy latency_budget = new DDS.LatencyBudgetQosPolicy();
  public DDS.LivelinessQosPolicy liveliness = new DDS.LivelinessQosPolicy();
  public DDS.ReliabilityQosPolicy reliability = new DDS.ReliabilityQosPolicy();
  public DDS.DestinationOrderQosPolicy destination_order = new DDS.DestinationOrderQosPolicy();
  public DDS.HistoryQosPolicy history = new DDS.HistoryQosPolicy();
  public DDS.ResourceLimitsQosPolicy resource_limits = new DDS.ResourceLimitsQosPolicy();
  public DDS.UserDataQosPolicy user_data = new DDS.UserDataQosPolicy();
  public DDS.OwnershipQosPolicy ownership = new DDS.OwnershipQosPolicy();
  public DDS.TimeBasedFilterQosPolicy time_based_filter = new DDS.TimeBasedFilterQosPolicy();
  public DDS.ReaderDataLifecycleQosPolicy reader_data_lifecycle = new DDS.ReaderDataLifecycleQosPolicy();
  public DDS.ShareQosPolicy share = new DDS.ShareQosPolicy();
  public DDS.ReaderLifespanQosPolicy reader_lifespan = new DDS.ReaderLifespanQosPolicy();
  public DDS.SubscriptionKeyQosPolicy subscription_keys = new DDS.SubscriptionKeyQosPolicy();

  /**
   * Returns the default DataReaderQos.
   */
  public DataReaderQos ()
  {
  } // ctor

  public DataReaderQos (DDS.DurabilityQosPolicy _durability,
                        DDS.DeadlineQosPolicy _deadline,
                        DDS.LatencyBudgetQosPolicy _latency_budget,
                        DDS.LivelinessQosPolicy _liveliness,
                        DDS.ReliabilityQosPolicy _reliability,
                        DDS.DestinationOrderQosPolicy _destination_order,
                        DDS.HistoryQosPolicy _history,
                        DDS.ResourceLimitsQosPolicy _resource_limits,
                        DDS.UserDataQosPolicy _user_data,
                        DDS.OwnershipQosPolicy _ownership,
                        DDS.TimeBasedFilterQosPolicy _time_based_filter,
                        DDS.ReaderDataLifecycleQosPolicy _reader_data_lifecycle,
                        DDS.SubscriptionKeyQosPolicy _subscription_keys,
                        DDS.ReaderLifespanQosPolicy _reader_lifespan,
                        DDS.ShareQosPolicy _share)
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
    subscription_keys = _subscription_keys;
    share = _share;
    reader_lifespan = _reader_lifespan;
  } // ctor

} // class DataReaderQos
