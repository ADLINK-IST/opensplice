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

#include "dds_dcps.h"
#include "sac_common.h"
#include "sac_object.h"
#include "sac_objManag.h"
#include "sac_entity.h"
#include "sac_domain.h"
#include "sac_domainParticipant.h"
#include "sac_typeSupport.h"
#include "sac_report.h"

#include "os_atomics.h"

static C_STRUCT(_DomainParticipantFactory) *TheFactory = NULL;
static pa_uint32_t TheFactoryInitializingCount = PA_UINT32_INIT (0);
static DDS_boolean TheFactoryOperational = FALSE;

#define DDS_DomainParticipantFactoryClaim(_this, factory) \
        DDS_Object_claim(DDS_Object(_this), DDS_DOMAINFACTORY, (_Object *)factory)

#define DDS_DomainParticipantFactoryClaimRead(_this, factory) \
        DDS_Object_claim(DDS_Object(_this), DDS_DOMAINFACTORY, (_Object *)factory)

#define DDS_DomainParticipantFactoryRelease(_this) \
        DDS_Object_release(DDS_Object(_this))

#define DDS_DomainParticipantFactoryCheck(_this, factory) \
        DDS_Object_check_and_assign(DDS_Object(_this), DDS_DOMAINFACTORY, (_Object *)factory)

static void
factoryCleanup(void);

static c_equality
compareDomainId(
    c_voidp o,
    c_iterResolveCompareArg arg)
{
    DDS_DomainId_t domainId;
    c_equality equality = C_NE;

    if((o) && (arg != NULL)) {
        domainId = DDS_Domain_get_domain_id(o);
        if (domainId != DDS_DOMAIN_ID_INVALID) {
            if (domainId == *(DDS_DomainId_t *)arg) {
                equality = C_EQ;
            }
        }
    }
    return equality;
}

static c_equality
compareParticipantDomainId(
    c_voidp o,
    c_iterResolveCompareArg arg)
{
    DDS_DomainId_t domainId;
    c_equality equality = C_NE;

    if((o) && (arg != NULL)) {
        domainId = DDS_DomainParticipant_get_domain_id(o);
        if (domainId != DDS_DOMAIN_ID_INVALID) {
            if (domainId == *(DDS_DomainId_t *)arg) {
                equality = C_EQ;
            }
        }
    }
    return equality;
}

C_CLASS(_TypeSupportBinding);

C_STRUCT(_TypeSupportBinding) {
    DDS_char *type_name;
    DDS_TypeSupport *typeSupport;
};

/*
 * From Specification
 *
 *     DomainParticipantFactory get_instance (void)
 */
DDS_DomainParticipantFactory
DDS_DomainParticipantFactory_get_instance(void)
{
    DDS_ReturnCode_t result;
    os_uint safecount;
    os_time delay = {1,0};
    os_uint count;


    safecount = pa_inc32_nv(&TheFactoryInitializingCount);
    if ((safecount == 1) && (TheFactoryOperational == FALSE)) {
        if (u_userInitialise() == U_RESULT_OK) {
            SAC_REPORT_STACK();
            result = DDS_Object_new(DDS_DOMAINFACTORY, NULL, (_Object *)&TheFactory);
            if (result == DDS_RETCODE_OK) {
                TheFactory->participantList = NULL;
                TheFactory->domainList = NULL;

                /* Can't fail with defaults */
                (void)DDS_DomainParticipantFactoryQos_init(&TheFactory->qos, DDS_PARTICIPANTFACTORY_QOS_DEFAULT);
                (void)DDS_DomainParticipantQos_init(&TheFactory->defaultQos, DDS_PARTICIPANT_QOS_DEFAULT);

                os_procAtExit(factoryCleanup);
                TheFactoryOperational = TRUE;
            }
            SAC_REPORT_FLUSH(NULL, result != DDS_RETCODE_OK);
        }
    } else { /* wait for the factory to become initialized */
        count = 0;
        while ((count < 5) && (TheFactoryOperational == FALSE)) {
            count++;
            os_nanoSleep(delay);
        }
        if (TheFactoryOperational == FALSE) {
            result = DDS_RETCODE_ERROR;
            SAC_REPORT(result, "The factory initialization failed.");
        } else {
            result = DDS_RETCODE_OK;
        }
    }
    (void) pa_dec32_nv(&TheFactoryInitializingCount);
    return (DDS_DomainParticipantFactory)TheFactory;
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
    const DDS_StatusMask mask)
{
    DDS_ReturnCode_t result = DDS_RETCODE_OK;
    _DomainParticipantFactory factory;
    _DomainParticipant participant = NULL;
    DDS_char *participantName;

    SAC_REPORT_STACK();

    if (domain_id == DDS_DOMAIN_ID_INVALID) {
        result = DDS_RETCODE_BAD_PARAMETER;
        SAC_REPORT(result, "DomainParticipant is using an invalid domain identifier (%d).", domain_id);
    } else {
        result = DDS_DomainParticipantQos_is_consistent(qos);
    }

    if (result == DDS_RETCODE_OK) {
        result = DDS_DomainParticipantFactoryClaim(_this, &factory);
        if (result == DDS_RETCODE_OK) {
            if (qos == DDS_PARTICIPANT_QOS_DEFAULT) {
                qos = &factory->defaultQos;
            }
            result = DDS_DomainParticipantFactoryRelease(_this);
            participantName = u_userGetProcessName();
            participant = DDS_DomainParticipantNew(_this, participantName, domain_id, qos);
            os_free(participantName);
            if ( participant ) {
                result = DDS_DomainParticipant_set_listener(participant, a_listener, mask);
                if ( result == DDS_RETCODE_OK ) {
                    result = DDS_DomainParticipantFactoryClaim(_this, &factory);
                    if ( result == DDS_RETCODE_OK ) {
                        if (factory->qos.entity_factory.autoenable_created_entities) {
                            result = DDS_Entity_enable(participant);
                        }
                        if ( result == DDS_RETCODE_OK ) {
                            if (factory->participantList == NULL) {
                                factory->participantList = c_iterNew(NULL);
                            }
                            factory->participantList = c_iterInsert (factory->participantList, participant);
                            result = factory->participantList ? DDS_RETCODE_OK : DDS_RETCODE_OUT_OF_RESOURCES;
                        }
                        DDS_DomainParticipantFactoryRelease(_this);
                        if (result != DDS_RETCODE_OK) {
                            DDS_DomainParticipantFree(participant);
                            participant = NULL;
                        }
                    }
                }
            } else {
                result = DDS_RETCODE_ERROR;
            }
        }
    }
    SAC_REPORT_FLUSH(_this, result != DDS_RETCODE_OK);
    return (DDS_DomainParticipant) participant;
}

typedef struct {
    DDS_DomainId_t domainId;
    int nrOfConnectedParticipants;
} countParticipantsArg;

static void
countParticipants(
        void* o,
        c_iterActionArg arg)
{
    countParticipantsArg *ccArg = (countParticipantsArg *)arg;

    if (DDS_DomainParticipant_get_domain_id(o) == ccArg->domainId) {
        ccArg->nrOfConnectedParticipants++;
    }
}

/*     ReturnCode_t
 *     delete_participant(
 *         in DomainParticipant a_participant);
 */
DDS_ReturnCode_t
DDS_DomainParticipantFactory_delete_participant (
    DDS_DomainParticipantFactory _this,
    const DDS_DomainParticipant participant)
{
    DDS_ReturnCode_t result;
    DDS_DomainParticipant found;
    DDS_DomainId_t domainId;
    _DomainParticipantFactory factory;

    SAC_REPORT_STACK();

    result = DDS_DomainParticipantFactoryClaim(_this, &factory);
    if (result == DDS_RETCODE_OK) {
        domainId = DDS_DomainParticipant_get_domain_id(participant);
        if( domainId != DDS_DOMAIN_ID_INVALID ) {
            if (DDS_DomainParticipant_has_contained_entities(participant)) {
                result = DDS_RETCODE_PRECONDITION_NOT_MET;
                SAC_REPORT(result, "Participant contains entities.");
            } else {
                found = c_iterTake(factory->participantList, participant);
                if (found != participant) {
                    /* The following call is expensive so only use it in case of exceptions. */
                    if (DDS_Object_get_kind(DDS_Object(participant)) == DDS_DOMAINPARTICIPANT) {
                        result = DDS_RETCODE_PRECONDITION_NOT_MET;
                        SAC_REPORT(result, "DomainParticipant does not belong to this DomainParticipantFactory.");
                    } else {
                        result = DDS_RETCODE_BAD_PARAMETER;
                        SAC_REPORT(result, "DomainParticipant parameter 'participant' is of type %s",
                                    DDS_ObjectKind_image(DDS_Object_get_kind(DDS_Object(participant))));
                    }
                } else {
                    result = DDS__free(participant);
                }
            }
            if( result == DDS_RETCODE_OK ) {
                countParticipantsArg arg;
                arg.domainId = domainId;
                arg.nrOfConnectedParticipants = 0;
                /* Check if this participant is the last participant of
                 * its domain.
                 */
                c_iterWalk(factory->participantList,
                           countParticipants,
                           &arg);
                if (c_iterLength(factory->participantList) == 0) {
                    c_iterFree(factory->participantList);
                    factory->participantList = NULL;
                }
            }
        } else {
            result = DDS_RETCODE_BAD_PARAMETER;
            SAC_REPORT(result, "DomainParticipant is invalid.");
        }
        DDS_DomainParticipantFactoryRelease(_this);
    }
    SAC_REPORT_FLUSH(_this, result != DDS_RETCODE_OK);
    return result;
}


/*     DomainParticipant
 *     lookup_participant(
 *         in DDS_DomainId_t domain_id);
 */
DDS_DomainParticipant
DDS_DomainParticipantFactory_lookup_participant(
    DDS_DomainParticipantFactory _this,
    const DDS_DomainId_t domain_id)
{
    DDS_ReturnCode_t result;
    DDS_DomainParticipant participant = NULL;
    _DomainParticipantFactory factory;
    DDS_DomainId_t id;

    SAC_REPORT_STACK();

    result = DDS_DomainParticipantFactoryClaimRead(_this, &factory);
    if (result == DDS_RETCODE_OK) {
        if (domain_id == DDS_DOMAIN_ID_DEFAULT) {
            id = u_userGetDomainIdFromEnvUri();
        } else {
            id = domain_id;
        }
        participant = c_iterResolve(factory->participantList,
                                    compareParticipantDomainId,
                                    (void *)&id);
        DDS_DomainParticipantFactoryRelease(_this);
    }
    SAC_REPORT_FLUSH(_this, result != DDS_RETCODE_OK);
    return participant;
}

/*     ReturnCode_t
 *     set_default_participant_qos(
 *         in DomainParticipantQos qos);
 */
DDS_ReturnCode_t
DDS_DomainParticipantFactory_set_default_participant_qos (
    DDS_DomainParticipantFactory _this,
    const DDS_DomainParticipantQos *qos)
{
    DDS_ReturnCode_t result;
    _DomainParticipantFactory factory;

    SAC_REPORT_STACK();

    result = DDS_DomainParticipantQos_is_consistent(qos);
    if (result == DDS_RETCODE_OK) {
        result = DDS_DomainParticipantFactoryClaim(_this, &factory);
        if (result == DDS_RETCODE_OK) {
            result = DDS_DomainParticipantQos_init(&factory->defaultQos, qos);
            DDS_DomainParticipantFactoryRelease(_this);
        }
    }
    SAC_REPORT_FLUSH(_this, result != DDS_RETCODE_OK);
    return result;
}

/*     ReturnCode_t
 *     get_default_participant_qos(
 *         inout DomainParticipantQos qos);
 */
DDS_ReturnCode_t
DDS_DomainParticipantFactory_get_default_participant_qos (
    DDS_DomainParticipantFactory _this,
    DDS_DomainParticipantQos *qos)
{
    DDS_ReturnCode_t result;
    _DomainParticipantFactory factory;

    SAC_REPORT_STACK();

    result = DDS_DomainParticipantFactoryClaimRead(_this, &factory);
    if (result == DDS_RETCODE_OK) {
        result = DDS_DomainParticipantQos_init(qos, &factory->defaultQos);
        DDS_DomainParticipantFactoryRelease(_this);
    }

    SAC_REPORT_FLUSH(_this, result != DDS_RETCODE_OK);
    return result;
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
    DDS_ReturnCode_t result;
    _DomainParticipantFactory factory;

    SAC_REPORT_STACK();

    result = DDS_DomainParticipantFactoryQos_is_consistent(qos);
    if (result == DDS_RETCODE_OK) {
        result = DDS_DomainParticipantFactoryClaim(_this, &factory);
        if (result == DDS_RETCODE_OK) {
            result = DDS_DomainParticipantFactoryQos_init(&factory->qos, qos);
            DDS_DomainParticipantFactoryRelease(_this);
        }
    }
    SAC_REPORT_FLUSH(_this, result != DDS_RETCODE_OK);
    return result;
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
    DDS_ReturnCode_t result;
    _DomainParticipantFactory factory;

    SAC_REPORT_STACK();

    result = DDS_DomainParticipantFactoryClaimRead(_this, &factory);
    if (result == DDS_RETCODE_OK) {
        result = DDS_DomainParticipantFactoryQos_init(qos, &factory->qos);
        DDS_DomainParticipantFactoryRelease(_this);
    }

    SAC_REPORT_FLUSH(_this, result != DDS_RETCODE_OK);
    return result;
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
    DDS_ReturnCode_t result;
    DDS_Domain domain = NULL;
    DDS_DomainId_t id = domain_id;
    _DomainParticipantFactory factory;

    SAC_REPORT_STACK();

    result = DDS_DomainParticipantFactoryClaim(_this, &factory);
    if (result == DDS_RETCODE_OK) {
        if (domain_id == DDS_DOMAIN_ID_DEFAULT) {
            id = u_userGetDomainIdFromEnvUri();
        } else {
            id = domain_id;
        }
        domain = c_iterResolve(factory->domainList, compareDomainId, &id);
        if (domain == NULL) {
            domain = DDS_DomainNew(id);
            if (domain == NULL) {
                result = DDS_RETCODE_PRECONDITION_NOT_MET;
                SAC_REPORT(result, "Could not locate a domain with domainId = %d.", id);
            } else {
                if (factory->domainList == NULL) {
                    factory->domainList = c_iterNew(NULL);
                }
                factory->domainList = c_iterInsert(factory->domainList, domain);
            }
        }
        DDS_DomainParticipantFactoryRelease(_this);
    }
    SAC_REPORT_FLUSH(_this, result != DDS_RETCODE_OK);
    return domain;
}

/*     ReturnCode_t
 *     delete_domain(
 *          in Domain a_domain);
 */
DDS_ReturnCode_t
DDS_DomainParticipantFactory_delete_domain (
    DDS_DomainParticipantFactory _this,
    DDS_Domain domain)
{
    DDS_ReturnCode_t result;
    DDS_Domain found;
    _DomainParticipantFactory factory;

    SAC_REPORT_STACK();

    result = DDS_DomainParticipantFactoryClaim(_this, &factory);
    if (result == DDS_RETCODE_OK) {
        found = c_iterTake(factory->domainList, domain);
        if (found == domain) {
            result = DDS_DomainFree(domain);
        } else {
            /* The following call is expensive so only use it in case of exceptions. */
            if (DDS_Object_get_kind(DDS_Object(domain)) == DDS_DOMAIN) {
                result = DDS_RETCODE_PRECONDITION_NOT_MET;
                SAC_REPORT(result, "Domain does not belong to this DomainParticipantFactory");
            } else {
                result = DDS_RETCODE_BAD_PARAMETER;
                SAC_REPORT(result, "Domain parameter 'domain' is of type %s",
                            DDS_ObjectKind_image(DDS_Object_get_kind(DDS_Object(domain))));
            }
        }
        if (c_iterLength(factory->domainList) == 0) {
            c_iterFree(factory->domainList);
            factory->domainList = NULL;
        }
        DDS_DomainParticipantFactoryRelease(_this);
    }
    SAC_REPORT_FLUSH(_this, result != DDS_RETCODE_OK);
    return result;
}

/*     ReturnCode_t
 *     delete_contained_entities(
 *          );
 */
DDS_ReturnCode_t
DDS_DomainParticipantFactory_delete_contained_entities (
    DDS_DomainParticipantFactory _this)
{
    DDS_ReturnCode_t result, endResult = DDS_RETCODE_OK;
    DDS_DomainParticipant dp;
    _DomainParticipantFactory factory;
    c_iter list;
    c_ulong i, nrParticipants;

    SAC_REPORT_STACK();

    result = DDS_DomainParticipantFactoryClaim(_this, &factory);
    if (result == DDS_RETCODE_OK) {
        /* Make sure we attempt to delete each entity only once:
         * entities that are not ready to be deleted should be
         * inserted back into the list, but should not be encountered
         * again during this iteration. So iterate by the number
         * of elements instead of by taking until the list is empty.
         */
        list = factory->participantList;
        nrParticipants = c_iterLength(list);
        for (i = 0; i < nrParticipants; i++) {
            dp = DDS_DomainParticipant(c_iterTakeFirst(list));
            result = DDS_DomainParticipantFree (dp);
            if (result != DDS_RETCODE_OK) {
                c_iterInsert(factory->participantList, dp);
                endResult = result;
            }
        }
        if (endResult == DDS_RETCODE_OK) {
            c_iterFree(factory->participantList);
            factory->participantList = NULL;
        }
        DDS_DomainParticipantFactoryRelease(_this);
    }
    SAC_REPORT_FLUSH(_this, endResult != DDS_RETCODE_OK);
    return endResult;
}


DDS_ReturnCode_t
DDS_DomainParticipantFactory_detach_all_domains (
    DDS_DomainParticipantFactory _this,
    DDS_boolean block_operations,
    DDS_boolean delete_entities)
{
    DDS_ReturnCode_t result;
    u_result ures;
    _DomainParticipantFactory factory;
    os_uint32 flags = 0;

    if (block_operations) {
        flags |= U_USER_BLOCK_OPERATIONS;
    }
    if (delete_entities) {
        flags |= U_USER_DELETE_ENTITIES;
    }

    result = DDS_DomainParticipantFactoryCheck(_this, &factory);
    if (result == DDS_RETCODE_OK) {
        ures = u_userDetach(flags);
        result = DDS_ReturnCode_get(ures);
    }

    return result;
}


static void
factoryCleanup(void)
{
    u_userDetach(U_USER_DELETE_ENTITIES);
}
