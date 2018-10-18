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


public final class DomainParticipantQos
{
  public DDS.UserDataQosPolicy user_data = new DDS.UserDataQosPolicy();
  public DDS.EntityFactoryQosPolicy entity_factory = new DDS.EntityFactoryQosPolicy();
  public DDS.SchedulingQosPolicy watchdog_scheduling = new DDS.SchedulingQosPolicy();
  public DDS.SchedulingQosPolicy listener_scheduling = new DDS.SchedulingQosPolicy();

  public DomainParticipantQos ()
  {
  } // ctor

  public DomainParticipantQos (DDS.UserDataQosPolicy _user_data,
                               DDS.EntityFactoryQosPolicy _entity_factory,
                               DDS.SchedulingQosPolicy _watchdog_scheduling,
                               DDS.SchedulingQosPolicy _listener_scheduling)
  {
    user_data = _user_data;
    entity_factory = _entity_factory;
    watchdog_scheduling = _watchdog_scheduling;
    listener_scheduling = _listener_scheduling;
  } // ctor

} // class DomainParticipantQos

