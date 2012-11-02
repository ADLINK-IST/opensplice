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


public interface DomainParticipantFactoryOperations
{
	/**
     * Creates a new DomainParticipant.
     * The newly created {@link DomainParticipant} which will
     * join the domain identified by domainId, with the desired
     * {@link DomainParticipantQos} and attaches the optionally specified
     * {@link DomainParticipantListener} to it.
     * @param domainId the ID of the Domain to which the
     * DomainParticipant is joined.
     * @param qos a DomainParticipantQos for the new DomainParticipant.
     * When this set of QosPolicy settings is inconsistent, no
     * DomainParticipant is created. If no qos is set (e.i. <code>null</code>
     * is used as parameter), the default qos will be used.
     * @param a_listener reference to the DomainParticipantListener for the
     * new DomainParticipant.
     * @return a reference to the newly created DomainParticipant.
     * Returns null in case of an error.
     */
  DDS.DomainParticipant create_participant (String domainId, DDS.DomainParticipantQos qos, DDS.DomainParticipantListener a_listener,int mask);

  /**
   * Deletes a DomainParticipant.
   * This method deletes a DomainParticipant. A DomainParticipant cannot
   * be deleted when it has any attached Entity objects.
   * When the operation is called on a DomainParticipant with existing
   * {@link Entity} objects, the operation returns
   * RETCODE_PRECONDITION_NOT_MET.
   * @param a_participant The DomainParticipant that will be deleted.
   * @return Possible return codes of the operation are: RETCODE_OK,
   * RETCODE_BAD_PARAMETER or RETCODE_PRECONDITION_NOT_MET.
   */
  int delete_participant (DDS.DomainParticipant a_participant);

  /**
   * Retrieves a previously created DomainParticipant belonging to specified
   * domainId.
   * If no such {@link DomainParticipant} exists, null will be returend.
   * If multiple DomainParticipant beloning to that domainId exist, then the
   * method will return one of them. It is <b>not</b> specified which one.
   * @param domainId The domain that is being searched.
   * @return A DomainParticipant.
   */
  DDS.DomainParticipant lookup_participant (String domainId);

  /**
   * Sets the default DomainParticipantQos of the DomainParticipant.
   * This method sets the default {@link DomainParticipantQos} of the
   * DomainParticipant which is used for newly created DomainParticipant
   * objects, in case the no qos is specified. <p>
   * The default DomainParticipantQos is only used when <code>null</code> is
   * supplied as qos parameter in the create_participant method. <p>
   * The DomainParticipant QosPolicy settings are always self consistent,
   * because the they do not depend on each other. This means this operation never returns the
   * RETCODE_INCONSISTENT_POLICY. The values set by this operation are
   * returned by
   * {@link #get_default_participant_qos(DomainParticipantQosHolder)}.
   * @param qos new default qos.
   * @return return code.
   */
  int set_default_participant_qos (DDS.DomainParticipantQos qos);

  /**
   * Returns the default DomainParticipantQos of the DomainParticipant.
   * @param qos a Holder class that will contain the default
   * {@link DDS.DomainParticipantQos}.
   * @see #create_participant(String, DomainParticipantQos,
   * DomainParticipantListener) for more information on the default qos.
   */
  int get_default_participant_qos (DDS.DomainParticipantQosHolder qos);


  int set_qos (DDS.DomainParticipantFactoryQos qos);
  int get_qos (DDS.DomainParticipantFactoryQosHolder qos);

  DDS.Domain lookup_domain (String domain_id);
  int delete_domain (DDS.Domain a_domain);
  int delete_contained_entities ();
} // interface DomainParticipantFactoryOperations
