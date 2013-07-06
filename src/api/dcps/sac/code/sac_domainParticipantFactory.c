/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2013 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE
 *
 *   for full copyright notice and license terms.
 *
 */

#include "gapi.h"
#include "dds_dcps.h"
#include "sac_builtinTopics.h"
#include "sac_structured.h"


/*
 * From Specification
 *
 *     DomainParticipantFactory get_instance (void)
 */
DDS_DomainParticipantFactory
DDS_DomainParticipantFactory_get_instance (
    void
    )
{
    return (DDS_DomainParticipantFactory)
	gapi_domainParticipantFactory_get_instance (
	);
}

/*     DomainParticipant
*     create_participant(
*         in DomainId_t domain_id,
*         in DomainParticipantQos qos,
*         in DomainParticipantListener a_listener);
*/
DDS_DomainParticipant
DDS_DomainParticipantFactory_create_participant (
      DDS_DomainParticipantFactory _this,
      const DDS_DomainId_t domain_id,
      const DDS_DomainParticipantQos *qos,
      const struct DDS_DomainParticipantListener *a_listener,
      const DDS_StatusMask mask
      )
{
     struct gapi_domainParticipantListener gListener;
     struct gapi_domainParticipantListener *pListener = NULL;
     DDS_DomainParticipant participant;

     if ( a_listener ) {
         sac_copySacDomainParticipantListener(a_listener, &gListener);
         pListener = &gListener;
     }



     participant = (DDS_DomainParticipant) gapi_domainParticipantFactory_create_participant (
         (gapi_domainParticipantFactory)_this,
         (DDS_DomainId_t) domain_id,
         (const gapi_domainParticipantQos *)qos,
         (const struct gapi_domainParticipantListener *)pListener,
         (gapi_statusMask) mask,
         NULL, NULL, NULL, NULL);
     if ( participant ) {
         if ( sac_builtinTopicRegisterTypeSupport(participant) != DDS_RETCODE_OK ) {
             gapi_domainParticipantFactory_delete_participant(
                 (gapi_domainParticipantFactory)_this,
                 (gapi_domainParticipant) participant);
             participant = NULL;
         }
         else {
             gapi_domainParticipantFactoryQos *dpfqos = gapi_domainParticipantFactoryQos__alloc();
             if(dpfqos){
                 if(gapi_domainParticipantFactory_get_qos(_this, dpfqos) == GAPI_RETCODE_OK){
                     if(dpfqos->entity_factory.autoenable_created_entities) {
                         gapi_entity_enable(participant);
                     }
                 }

                 gapi_free(dpfqos);
             }
         }
     }

     return (DDS_DomainParticipant) participant;
}

/*     ReturnCode_t
 *     delete_participant(
 *         in DomainParticipant a_participant);
 */
DDS_ReturnCode_t
DDS_DomainParticipantFactory_delete_participant (
    DDS_DomainParticipantFactory _this,
    const DDS_DomainParticipant a_participant
    )
{
    return (DDS_ReturnCode_t)
	gapi_domainParticipantFactory_delete_participant (
	    (gapi_domainParticipantFactory)_this,
	    (gapi_domainParticipant)a_participant
	);
}

/*     DomainParticipant
 *     lookup_participant(
 *         in DDS_DomainId_t domain_id);
 */
DDS_DomainParticipant
DDS_DomainParticipantFactory_lookup_participant(
    DDS_DomainParticipantFactory _this,
    const DDS_DomainId_t domain_id
    )
{
    return (DDS_DomainParticipant)
    gapi_domainParticipantFactory_lookup_participant (
        (gapi_domainParticipantFactory)_this,
        (gapi_domainId_int_t)domain_id
    );
}

/*     ReturnCode_t
 *     set_default_participant_qos(
 *         in DomainParticipantQos qos);
 */
DDS_ReturnCode_t
DDS_DomainParticipantFactory_set_default_participant_qos (
    DDS_DomainParticipantFactory _this,
    const DDS_DomainParticipantQos *qos
    )
{
    return (DDS_ReturnCode_t)
	gapi_domainParticipantFactory_set_default_participant_qos (
	    (gapi_domainParticipantFactory)_this,
	    (const gapi_domainParticipantQos *)qos
	);
}

/*     ReturnCode_t
 *     get_default_participant_qos(
 *         inout DomainParticipantQos qos);
 */
DDS_ReturnCode_t
DDS_DomainParticipantFactory_get_default_participant_qos (
    DDS_DomainParticipantFactory _this,
    DDS_DomainParticipantQos *qos
    )
{
    return (DDS_ReturnCode_t)
    gapi_domainParticipantFactory_get_default_participant_qos (
        (gapi_domainParticipantFactory)_this,
        (gapi_domainParticipantQos *)qos
    );
}

/*     ReturnCode_t
 *     set_qos(
 *         in DomainParticipantFactoryQos qos);
 */
DDS_ReturnCode_t
DDS_DomainParticipantFactory_set_qos (
    DDS_DomainParticipantFactory _this,
    const DDS_DomainParticipantFactoryQos *qos)
{
    return (DDS_ReturnCode_t)
	gapi_domainParticipantFactory_set_qos (
	    (gapi_domainParticipantFactory)_this,
	    (const gapi_domainParticipantFactoryQos *)qos
	);
}

/*     ReturnCode_t
 *     get_qos(
 *         inout DomainParticipantFactoryQos qos);
 */
DDS_ReturnCode_t
DDS_DomainParticipantFactory_get_qos (
    DDS_DomainParticipantFactory _this,
    DDS_DomainParticipantFactoryQos *qos)
{
    return (DDS_ReturnCode_t)
    gapi_domainParticipantFactory_get_qos (
        (gapi_domainParticipantFactory)_this,
        (gapi_domainParticipantFactoryQos *)qos
    );
}


/*     Domain
 *     lookup_domain(
 *         in DDS_DomainId_t domain_id);
 */
DDS_Domain
DDS_DomainParticipantFactory_lookup_domain(
    DDS_DomainParticipantFactory _this,
    const DDS_DomainId_t domain_id)
{
    return (DDS_Domain)gapi_domainParticipantFactory_lookup_domain (
        (gapi_domainParticipantFactory)_this,
        (gapi_domainId_int_t)domain_id
    );
}

/*     ReturnCode_t
 *     delete_domain(
 *          in Domain a_domain);
 */
DDS_ReturnCode_t
DDS_DomainParticipantFactory_delete_domain (
    DDS_DomainParticipantFactory _this,
    DDS_Domain a_domain)
{
    return (DDS_ReturnCode_t)gapi_domainParticipantFactory_delete_domain (
        (gapi_domainParticipantFactory)_this,
        (gapi_domain)a_domain
    );
}

/*     ReturnCode_t
 *     delete_contained_entities(
 *          );
 */
DDS_ReturnCode_t
DDS_DomainParticipantFactory_delete_contained_entities (
    DDS_DomainParticipantFactory _this)
{
    gapi_returnCode_t result;

    result = gapi_domainParticipantFactory_delete_contained_entities (
        (gapi_domainParticipantFactory)_this);
    return (DDS_ReturnCode_t)result;
}
