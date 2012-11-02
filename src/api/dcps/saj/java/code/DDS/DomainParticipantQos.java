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


public final class DomainParticipantQos
{
  public DDS.UserDataQosPolicy user_data = null;
  public DDS.EntityFactoryQosPolicy entity_factory = null;
  public DDS.SchedulingQosPolicy watchdog_scheduling = null;
  public DDS.SchedulingQosPolicy listener_scheduling = null;

  public DomainParticipantQos ()
  {
  } // ctor

  public DomainParticipantQos (DDS.UserDataQosPolicy _user_data, DDS.EntityFactoryQosPolicy _entity_factory, DDS.SchedulingQosPolicy _watchdog_scheduling, DDS.SchedulingQosPolicy _listener_scheduling)
  {
    user_data = _user_data;
    entity_factory = _entity_factory;
    watchdog_scheduling = _watchdog_scheduling;
    listener_scheduling = _listener_scheduling;
  } // ctor

} // class DomainParticipantQos

