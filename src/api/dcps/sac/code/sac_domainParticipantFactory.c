
#include <gapi.h>
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
 *         in DomainId_t domainId,
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
            (gapi_domainId_t)domain_id,
            (const gapi_domainParticipantQos *)qos,
            (const struct gapi_domainParticipantListener *)pListener,
            (gapi_statusMask) mask,
            NULL, NULL, NULL);
        if ( participant ) {
            if ( sac_builtinTopicRegisterTypeSupport(participant) != DDS_RETCODE_OK ) {
                gapi_domainParticipantFactory_delete_participant(
                    (gapi_domainParticipantFactory)_this,
                    (gapi_domainParticipant) participant);
                participant = NULL;
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
 *         in DomainId_t domainId);
 */
DDS_DomainParticipant
DDS_DomainParticipantFactory_lookup_participant (
    DDS_DomainParticipantFactory _this,
    const DDS_DomainId_t domainId
    )
{
    return (DDS_DomainParticipant)
	gapi_domainParticipantFactory_lookup_participant (
	    (gapi_domainParticipantFactory)_this,
	    (gapi_domainId_t)domainId
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
