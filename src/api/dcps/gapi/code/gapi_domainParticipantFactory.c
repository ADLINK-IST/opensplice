#include "gapi_domainParticipantFactory.h"
#include "gapi_domainParticipant.h"
#include "gapi_entity.h"
#include "gapi_kernel.h"
#include "gapi_qos.h"
#include "gapi_structured.h"
#include "gapi_expression.h"
#include "gapi_error.h"

#include "os.h"
#include "c_iterator.h"

#define U_DOMAIN_GET(t)       u_domain(U_ENTITY_GET(t))


C_STRUCT(_DomainParticipantFactory) {
    C_EXTENDS(_Object);
    _ObjectRegistry registry;
    c_iter DomainParticipantList;         /* List of domain participants */
    gapi_domainParticipantQos defaultQos; /* Default domain participant qos */
    os_mutex mtx;
};

static _DomainParticipantFactory TheFactory = (_DomainParticipantFactory)0;
static os_uint TheFactoryInitialized = 0;

static void factoryCleanup ( void );

static v_domainQos
newParticipantFactoryQos (
    void)
{
    v_domainQos ParticipantFactoryQos;
    ParticipantFactoryQos = u_domainQosNew(NULL);
    return ParticipantFactoryQos;
}

static void
freeParticipantFactoryQos (
    v_domainQos ParticipantFactoryQos)
{
    u_domainQosFree(ParticipantFactoryQos);
}

#ifdef _RP_
static gapi_boolean
copyParticipantFactoryQosIn (
    const gapi_domainParticipantFactoryQos *srcQos,
    v_domainQos dstQos)
{
    dstQos->entityFactory.autoenable_created_entities =
             srcQos->entity_factory.autoenable_created_entities;

    return TRUE;
}
#endif

static gapi_boolean
copyParticipantFactoryQosOut (
    const v_domainQos  srcQos,
    gapi_domainParticipantFactoryQos *dstQos)
{
    assert(srcQos);
    assert(dstQos);

    dstQos->entity_factory.autoenable_created_entities =
             srcQos->entityFactory.autoenable_created_entities;

    return TRUE;
}


static _DomainParticipantFactory
_DomainParticipantFactoryNew(
    void)
{
    os_mutexAttr attr;
    _DomainParticipantFactory _this = _DomainParticipantFactoryAlloc();

    if (_this != NULL) {
        _this->DomainParticipantList = c_iterNew (NULL);
        memset (&_this->defaultQos, 0,
                sizeof(_this->defaultQos));
        gapi_domainParticipantQosCopy (&gapi_domainParticipantQosDefault,
                                       &_this->defaultQos);
        if (u_userInitialise() != U_RESULT_OK) {
            c_iterFree (_this->DomainParticipantList);
            _EntityDelete(_this);
            _this = NULL;
        } else {
            os_mutexAttrInit(&attr);
            attr.scopeAttr = OS_SCOPE_PRIVATE;
            os_mutexInit(&_this->mtx, &attr);
        }
    }

    if ( _this ) {
        _this->registry = _ObjectRegistryNew();
    }

    gapi_expressionInitParser();

    os_procAtExit(factoryCleanup);

    return _this;
}

void
_DomainParticipantFactoryRegister (
    _Object object)
{
    gapi_domainParticipantFactory dpf;

    if ( TheFactory == NULL ) {
        dpf = gapi_domainParticipantFactory_get_instance();

    }
    _ObjectRegistryRegister(TheFactory->registry, object);
}

gapi_domainParticipantFactoryQos *
_DomainParticipantFactoryGetQos (
    _DomainParticipantFactory _this,
    gapi_domainParticipantFactoryQos *qos)
{
#ifdef _RP_
    v_domainQos domainQos;
    u_domain uDomain;

    assert(_this);

    uDomain = U_DOMAIN_GET(_this);

    if ( u_entityQoS(u_entity(uDomain), (v_qos*)&domainQos) == U_RESULT_OK ) {
        copyParticipantFactoryQosOut(domainQos,  qos);
        u_domainQosFree(domainQos);

    }

    return qos;
#else
    if (qos) {
        if (_this) {
            qos->entity_factory.autoenable_created_entities =
                _this->defaultQos.entity_factory.autoenable_created_entities;
        }
    }
    return qos;
#endif
}


/*
 * From Specification
 *
 *     DomainParticipantFactory get_instance (void)
 */
gapi_domainParticipantFactory
gapi_domainParticipantFactory_get_instance(
    void)
{
    os_uint safecount;
    os_time delay = {1,0};
    os_uint count;

    safecount = pa_increment(&TheFactoryInitialized);
    if ((safecount == 1) && (TheFactory == NULL)) {
        TheFactory = _DomainParticipantFactoryNew();
        _EntityRelease(TheFactory);
    } else { /* wait for the factory to become initialized */
        if (TheFactory == NULL) {
            count = 0;
            while ((count < 5) && (TheFactory == NULL)) {
                count++;
                os_nanoSleep(delay);
            }
        }
    }
    pa_decrement(&TheFactoryInitialized);
    if (TheFactory == NULL) {
        return NULL;
    } else {
        return (gapi_domainParticipantFactory)_EntityHandle(TheFactory);
    }
}

/*     DomainParticipant
 *     create_participant(
 *         in DomainId_t domainId,
 *         in DomainParticipantQos qos,
 *         in DomainParticipantListener a_listener);
 */
gapi_domainParticipant
gapi_domainParticipantFactory_create_participant(
    gapi_domainParticipantFactory _this,
    const gapi_domainId_t domainId,
    const gapi_domainParticipantQos *qos,
    const struct gapi_domainParticipantListener *a_listener,
    const gapi_statusMask mask,
    gapi_listenerThreadAction thread_start_action,
    gapi_listenerThreadAction thread_stop_action,
    void *thread_action_arg)
{
    _DomainParticipantFactory factory;
    _DomainParticipant newParticipant = NULL;
    gapi_context context;

    GAPI_CONTEXT_SET(context, _this, GAPI_METHOD_CREATE_PARTICIPANT);

    factory = gapi_domainParticipantFactoryClaim(_this, NULL);

    if( !factory ) {
        return (gapi_domainParticipant)NULL;
    }

    if ( factory != TheFactory ) {
        _EntityRelease(factory);
        return (gapi_domainParticipant)NULL;
    }

    os_mutexLock(&factory->mtx);
    if (qos == GAPI_PARTICIPANT_QOS_DEFAULT) {
        qos = &factory->defaultQos;
    }
    if (gapi_domainParticipantQosIsConsistent(qos, &context) == GAPI_RETCODE_OK) {
        newParticipant = _DomainParticipantNew (domainId, qos,
                                                a_listener, mask, factory,
                                                thread_start_action,
                                                thread_stop_action,
                                                thread_action_arg,
                                                &context);
        if ( newParticipant ) {
            c_iterInsert (factory->DomainParticipantList, newParticipant);
            _ObjectRegistryRegister(factory->registry, (_Object)newParticipant);
        }
    } else {
        newParticipant = NULL;
    }
    os_mutexUnlock(&factory->mtx);
    _EntityRelease(factory);

    return (gapi_domainParticipant)_EntityRelease(newParticipant);
}

/*     ReturnCode_t
 *     delete_participant(
 *         in DomainParticipant a_participant);
 */
gapi_returnCode_t
gapi_domainParticipantFactory_delete_participant(
    gapi_domainParticipantFactory _this,
    const gapi_domainParticipant a_participant)
{
    gapi_returnCode_t result;
    _DomainParticipantFactory factory;
    _DomainParticipant participant;
    gapi_context context;

    GAPI_CONTEXT_SET(context, _this, GAPI_METHOD_DELETE_PARTICIPANT);

    factory = gapi_domainParticipantFactoryClaim(_this, &result);

    if ( factory ) {
        os_mutexLock(&factory->mtx);
        if ( factory != TheFactory ) {
            result = GAPI_RETCODE_BAD_PARAMETER;
        } else {
            participant = gapi_domainParticipantClaimNB(a_participant, NULL);
            if ( participant != NULL ) {
                if (_DomainParticipantPrepareDelete (participant, &context)) {
                    if (c_iterTake (factory->DomainParticipantList, participant) != participant) {
                        result = GAPI_RETCODE_BAD_PARAMETER;
                    } else {
                        result = _DomainParticipantFree (participant);
                        if ( result == GAPI_RETCODE_OK ) {
                            participant = NULL;
                        }
                    }
                } else {
                    result = GAPI_RETCODE_PRECONDITION_NOT_MET;
                }
            } else {
                result = GAPI_RETCODE_BAD_PARAMETER;
            }
            _EntityRelease(participant);
        }
        os_mutexUnlock(&factory->mtx);
    }

    _EntityRelease(factory);

    return result;
}

gapi_returnCode_t
gapi_domainParticipantFactory_delete_participant_w_action(
    gapi_domainParticipantFactory _this,
    const gapi_domainParticipant  a_participant,
    gapi_deleteEntityAction       delete_action,
    void                         *action_arg)
{
    gapi_returnCode_t result;
    _DomainParticipantFactory factory;
    _DomainParticipant participant;
    gapi_context context;

    GAPI_CONTEXT_SET(context, _this, GAPI_METHOD_DELETE_PARTICIPANT);

    factory = gapi_domainParticipantFactoryClaim(_this, &result);

    if ( factory ) {
        os_mutexLock(&factory->mtx);
        if ( factory != TheFactory ) {
           result = GAPI_RETCODE_BAD_PARAMETER;
        } else {
            participant = gapi_domainParticipantClaim(a_participant, NULL);
            if ( participant ) {
                if (_DomainParticipantPrepareDelete (participant, &context)) {
                    if (c_iterTake (factory->DomainParticipantList, participant) != participant) {
                        result = GAPI_RETCODE_BAD_PARAMETER;
                    } else {
                        if ( delete_action ) {
                            _DomainParticipantSetBuiltinDeleteAction(
                                    participant, delete_action, action_arg, FALSE);
                        }
                        result = _DomainParticipantFree (participant);
                        if ( result == GAPI_RETCODE_OK ) {
                            participant = NULL;
                        }
                    }
                } else {
                    result = GAPI_RETCODE_PRECONDITION_NOT_MET;
                }
            } else {
                result = GAPI_RETCODE_BAD_PARAMETER;
            }
            _EntityRelease(participant);
        }
        os_mutexUnlock(&factory->mtx);
    }

    _EntityRelease(factory);

    return result;
}

gapi_returnCode_t
gapi_domainParticipantFactory_delete_participant_w_action_ext(
    gapi_domainParticipantFactory _this,
    const gapi_domainParticipant  a_participant,
    gapi_deleteEntityAction       delete_action,
    void                         *action_arg)
{
    gapi_returnCode_t result;
    _DomainParticipantFactory factory;
    _DomainParticipant participant;
    gapi_context context;

    GAPI_CONTEXT_SET(context, _this, GAPI_METHOD_DELETE_PARTICIPANT);

    factory = gapi_domainParticipantFactoryClaim(_this, &result);

    if ( factory ) {
        os_mutexLock(&factory->mtx);
        if ( factory != TheFactory ) {
            result = GAPI_RETCODE_BAD_PARAMETER;
        } else {
            participant = gapi_domainParticipantClaim(a_participant, NULL);
            if ( participant ) {
                if (_DomainParticipantPrepareDelete (participant, &context)) {
                    if (c_iterTake (factory->DomainParticipantList, participant) != participant) {
                        result = GAPI_RETCODE_BAD_PARAMETER;
                    } else {
                        if ( delete_action ) {
                            _DomainParticipantSetBuiltinDeleteAction(
                                    participant, delete_action, action_arg, TRUE);
                        }
                        result = _DomainParticipantFree (participant);
                        if ( result == GAPI_RETCODE_OK ) {
                            participant = NULL;
                        }
                    }
                } else {
                    result = GAPI_RETCODE_PRECONDITION_NOT_MET;
                }
            } else {
                result = GAPI_RETCODE_BAD_PARAMETER;
            }
            _EntityRelease(participant);
        }
        os_mutexUnlock(&factory->mtx);
    }

    _EntityRelease(factory);

    return result;
}


/*     DomainParticipant
 *     lookup_participant(
 *         in DomainId_t domainId);
 */
static c_equality
gapi_compareDomain(
    _DomainParticipant participant,
    gapi_domainId_t domainId)
{
    gapi_domainId_t id    = _DomainParticipantGetDomainId(participant);
    c_equality      equal = C_NE;

    if ( (domainId == NULL) && (id == NULL) ) {
        equal = C_EQ;
    } else if ( (domainId != NULL) && (id != NULL) ) {
        long len = strlen(id) + 1;
        if ( strncmp(domainId, id, len) == 0 ) {
            equal = C_EQ;
        } else {
            equal = C_NE;
        }
    } else {
        equal = C_NE;
    }

    return equal;
}

gapi_domainParticipant
gapi_domainParticipantFactory_lookup_participant(
    gapi_domainParticipantFactory _this,
    const gapi_domainId_t domainId)
{
    _DomainParticipantFactory factory;
    _DomainParticipant participant = NULL;

    factory = gapi_domainParticipantFactoryClaim(_this, NULL);

    if ( factory ) {
        if ( factory == TheFactory ) {
            os_mutexLock(&factory->mtx);
            participant = c_iterResolve (factory->DomainParticipantList,
                                         gapi_compareDomain,
                                         (void *)domainId);
            os_mutexUnlock(&factory->mtx);
        }
    }

    _EntityRelease(factory);

    return (gapi_domainParticipant)_EntityHandle(participant);
}

/*     ReturnCode_t
 *     set_qos(
 *         in DomainParticipantFactoryQos qos);
 *
 * Function will operate indepenedent of the enable flag
 */
gapi_returnCode_t
gapi_domainParticipantFactory_set_qos (
    gapi_domainParticipantFactory _this,
    const gapi_domainParticipantFactoryQos *qos)
{
    gapi_returnCode_t result = GAPI_RETCODE_OK;
    gapi_domainParticipantFactoryQos *existing_qos;
    _DomainParticipantFactory factory;
    gapi_context context;

    GAPI_CONTEXT_SET(context, _this, GAPI_METHOD_SET_QOS);

    factory = gapi_domainParticipantFactoryClaim(_this, &result);

    if ( factory && qos ) {
        result = gapi_domainParticipantFactoryQosIsConsistent(qos, &context);
    } else {
        result = GAPI_RETCODE_BAD_PARAMETER;
    }

    if ( result == GAPI_RETCODE_OK ) {
        existing_qos = gapi_domainParticipantFactoryQos__alloc();
#ifdef _RP_
        result = gapi_domainParticipantFactoryQosCheckMutability(qos, _DomainParticipantFactoryGetQos(factory, existing_qos), &context);
#else
        _DomainParticipantFactoryGetQos(factory, existing_qos);
        result = gapi_domainParticipantFactoryQosCheckMutability(qos,
                                                                 existing_qos,
                                                                 &context);
#endif
        gapi_free(existing_qos);
    }

    if ( result == GAPI_RETCODE_OK ) {
#ifdef _RP_
        factoryQos = newParticipantFactoryQos();
        if (factoryQos) {
            if ( copyParticipantFactoryQosIn(qos, factoryQos) ) {
                uResult = u_entitySetQoS(_EntityUEntity(factory),
                                         (v_qos)(factoryQos) );
                result = kernelResultToApiResult(uResult);
                freeParticipantFactoryQos(factoryQos);
            } else {
                result = GAPI_RETCODE_OUT_OF_RESOURCES;
            }
        } else {
            result = GAPI_RETCODE_OUT_OF_RESOURCES;
        }
#else
        factory->defaultQos.entity_factory.autoenable_created_entities =
                 qos->entity_factory.autoenable_created_entities;
#endif
    }

    _EntityRelease(factory);

    return result;
}

/*     ReturnCode_t
 *     get_qos(
 *         inout DomainParticipantFactoryQos qos);
 *
 * Function will operate indepenedent of the enable flag
 */
gapi_returnCode_t
gapi_domainParticipantFactory_get_qos (
    gapi_domainParticipantFactory _this,
    gapi_domainParticipantFactoryQos *qos)
{
    _DomainParticipantFactory ParticipantFactory;
    gapi_returnCode_t result;


    ParticipantFactory = gapi_domainParticipantFactoryClaim(_this, &result);
    if ( ParticipantFactory && qos ) {
        _DomainParticipantFactoryGetQos(ParticipantFactory, qos);
    }

    _EntityRelease(ParticipantFactory);
    return result;
}




/*     ReturnCode_t
 *     set_default_participant_qos(
 *         in DomainParticipantQos qos);
 */
gapi_returnCode_t
gapi_domainParticipantFactory_set_default_participant_qos(
    gapi_domainParticipantFactory _this,
    const gapi_domainParticipantQos *qos)
{
    gapi_returnCode_t result;
    _DomainParticipantFactory factory;
    gapi_context context;

    GAPI_CONTEXT_SET(context, _this, GAPI_METHOD_SET_DEFAULT_PARTICIPANT_QOS);

    factory = gapi_domainParticipantFactoryClaim(_this, &result);

    if ( factory ) {
        if ( factory == TheFactory ) {
            os_mutexLock(&factory->mtx);
            result = gapi_domainParticipantQosIsConsistent(qos, &context);
            if ( result == GAPI_RETCODE_OK ) {
                gapi_domainParticipantQosCopy(qos, &factory->defaultQos);
            }
            os_mutexUnlock(&factory->mtx);
        } else {
            result = GAPI_RETCODE_BAD_PARAMETER;
        }
    }

    _EntityRelease(factory);

    return result;
}

/*     void
 *     get_default_participant_qos(
 *         inout DomainParticipantQos qos);
 */
gapi_returnCode_t
gapi_domainParticipantFactory_get_default_participant_qos(
    gapi_domainParticipantFactory _this,
    gapi_domainParticipantQos *qos)
{
    gapi_returnCode_t result;
    _DomainParticipantFactory factory;

    factory = gapi_domainParticipantFactoryClaim(_this, &result);

    if ( factory ) {
        if ( factory == TheFactory ) {
            os_mutexLock(&factory->mtx);
            gapi_domainParticipantQosCopy (&factory->defaultQos,
                                           qos);
            os_mutexUnlock(&factory->mtx);
        }
    }

    _EntityRelease(factory);
    return result;
}



static c_equality
gapi_compareTypesupport(
    _DomainParticipant participant,
    _TypeSupport       typeSupport)
{
    c_equality equal = C_NE;

    _EntityClaim(participant);
    if (_DomainParticipantContainsTypeSupport(participant,typeSupport)) {
        equal = C_EQ;
    }
    _EntityRelease(participant);

    return equal;
}

_DomainParticipant
_DomainParticipantFactoryFindParticipantFromType(
    _TypeSupport typeSupport)
{
    _DomainParticipant participant = NULL;

    if ( TheFactory ) {
        os_mutexLock(&TheFactory->mtx);
        participant = c_iterResolve (TheFactory->DomainParticipantList,
                                     gapi_compareTypesupport,
                                     (void *)typeSupport);
        os_mutexUnlock(&TheFactory->mtx);
    }

    return participant;
}

gapi_boolean
_DomainParticipantFactoryIsContentSubscriptionAvailable(
    void)
{
    gapi_boolean result = TRUE;

    if ( !TheFactory ) 
    {
       result = FALSE;
    } 

    return result;
}

static void
factoryCleanup(void)
{
    _DomainParticipant participant;
    c_iter list;
    _ObjectRegistry registry;

    if ( TheFactory ) {
        list = TheFactory->DomainParticipantList;
        participant = _DomainParticipant(c_iterTakeFirst(list));
        while ( participant ) {
            _DomainParticipantCleanup(participant);
            participant = _DomainParticipant(c_iterTakeFirst(list));
        }
        os_mutexDestroy(&TheFactory->mtx);
        registry = TheFactory->registry;
        TheFactory->registry = NULL;
        _ObjectRegistryFree(registry);
    }
}
