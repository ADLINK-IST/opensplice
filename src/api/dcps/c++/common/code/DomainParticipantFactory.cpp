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
#include "vortex_os.h"
#include "u_user.h"

#include "Constants.h"
#include "DomainParticipantFactory.h"
#include "DomainParticipant.h"
#include "Domain.h"
#include "Constants.h"
#include "QosUtils.h"
#include "MiscUtils.h"
#include "ReportUtils.h"
#include "os_atomics.h"

namespace DDS
{
    namespace OpenSplice
    {
        class DomainParticipantFactoryHolder {
            public:
                DomainParticipantFactoryHolder() {
                    pa_stvoidp(&_factory, NULL);
                }
                ~DomainParticipantFactoryHolder();

                DomainParticipantFactory *
                get_instance();

            private:
                pa_voidp_t _factory;
        };
    }
}


DDS::OpenSplice::DomainParticipantFactoryHolder::~DomainParticipantFactoryHolder()
{
    DDS::DomainParticipantFactory *theFactory = static_cast<DDS::DomainParticipantFactory *>(pa_ldvoidp(&this->_factory));
    DDS::release(theFactory);
}

DDS::DomainParticipantFactory *
DDS::OpenSplice::DomainParticipantFactoryHolder::get_instance()
{
    DDS::DomainParticipantFactory *theFactory = static_cast<DDS::DomainParticipantFactory *>(pa_ldvoidp(&this->_factory));
    if (!theFactory) {
        theFactory = new DDS::DomainParticipantFactory();
        if (pa_casvoidp(&this->_factory, NULL, theFactory)) {
            os_procAtExit(DDS::DomainParticipantFactory::cleanup);
        } else {
            DDS::release(theFactory);
            theFactory = static_cast<DDS::DomainParticipantFactory *>(pa_ldvoidp(&this->_factory));
        }
    }
    return theFactory;
}



/* Stores the DomainParticipantFactory instance and will release the DomainParticipantFactory
 * instance when the application terminates.
 */
static DDS::OpenSplice::DomainParticipantFactoryHolder domainParticipantFactoryHolder;


/*
 * Local utility functions.
 */

DDS::Boolean
DDS::DomainParticipantFactory::rlReq_fnCountParticipants(
        DDS::Object_ptr object, countParticipantsArg *arg)
{
    DDS::OpenSplice::DomainParticipant *participant = dynamic_cast<DDS::OpenSplice::DomainParticipant *>(object);

    if (participant && participant->get_domain_id() == arg->domainId) {
        arg->nrOfConnectedParticipants++;
    }
    return TRUE;
}

DDS::Boolean
DDS::DomainParticipantFactory::rlReq_fnFindMatchingDomain(
        DDS::Object_ptr object, findMatchingDomainIdArg *arg)
{
    DDS::Boolean result = TRUE;

    DDS::OpenSplice::Domain *domainObj = dynamic_cast<DDS::OpenSplice::Domain *>(object);
    if (domainObj) {
        if (domainObj->get_domain_id () == arg->domainId) {
            arg->match = DDS::Object::_duplicate(domainObj);
            result = FALSE;
        }
    }
    return result;
}

DDS::Boolean
DDS::DomainParticipantFactory::rlReq_fnFindMatchingParticipant(
        DDS::Object_ptr object, findMatchingDomainIdArg *arg)
{
    DDS::Boolean result = TRUE;

    DDS::OpenSplice::DomainParticipant *participant = dynamic_cast<DDS::OpenSplice::DomainParticipant *>(object);
    if (participant) {
        if (participant->get_domain_id () == arg->domainId) {
            arg->match = participant;
            result = FALSE;
        }
    }
    return result;
}

void
DDS::DomainParticipantFactory::cleanup(void)
{
    u_userDetach(U_USER_DELETE_ENTITIES);
}

/*
 * Public operations.
 */
DDS::DomainParticipantFactory::DomainParticipantFactory() :
    DDS::OpenSplice::CppSuperClass(DDS::OpenSplice::DOMAINPARTICIPANTFACTORY),
    participantList(new DDS::OpenSplice::ObjSet(TRUE)),
    domainList(new DDS::OpenSplice::ObjSet(TRUE))
{
    if (u_userInitialise() == U_RESULT_OK) {
        if (init() != DDS::RETCODE_OK) {
            CPP_REPORT(DDS::RETCODE_ERROR,
                "Could not create DomainParticipantFactory.");
            exit(-1);
        }
    } else {
        CPP_REPORT(DDS::RETCODE_ERROR,
            "Could not create DomainParticipantFactory, "
            "user layer failed to initialize.");
        exit(-1);
    }
}

DDS::ReturnCode_t
DDS::DomainParticipantFactory::init()
{
    return nlReq_init();
}

DDS::ReturnCode_t
DDS::DomainParticipantFactory::wlReq_deinit()
{
    // Empty place holder: singleton DomainParticipantFactory will never be deleted.
    return DDS::RETCODE_UNSUPPORTED;
}


DDS::ReturnCode_t
DDS::DomainParticipantFactory::nlReq_init()
{
    DDS::ReturnCode_t result;

    result = DDS::OpenSplice::CppSuperClass::nlReq_init();
    if (result == DDS::RETCODE_OK) {
        result = participantList->init();
        if (result == DDS::RETCODE_OK) {
            result = domainList->init();
        }
    }

    if (result == DDS::RETCODE_OK) {
        myQos = PARTICIPANTFACTORY_QOS_DEFAULT;
        defaultParticipantQos = PARTICIPANT_QOS_DEFAULT;
    }
    return result;
}

DDS::DomainParticipantFactory::~DomainParticipantFactory()
{
    if (this->participantList) delete this->participantList;
    if (this->domainList) delete this->domainList;
    os_osExit();
}

DDS::DomainParticipantFactory_ptr
DDS::DomainParticipantFactory::_nil (void)
{
    return static_cast<DDS::DomainParticipantFactory_ptr>(NULL);
}

DDS::DomainParticipantFactory_ptr
DDS::DomainParticipantFactory::_duplicate (DDS::DomainParticipantFactory_ptr obj)
{
    return DDS::DomainParticipantFactoryInterface::_duplicate(obj);
}

DDS::Boolean
DDS::DomainParticipantFactory::wlReq_insertParticipant(
    DDS::OpenSplice::DomainParticipant *dp)
{
    return participantList->insertElement(dp);
}

DDS::Boolean
DDS::DomainParticipantFactory::wlReq_removeParticipant(
    DDS::OpenSplice::DomainParticipant *dp)
{
    return participantList->removeElement(dp);
}

DDS::Boolean
DDS::DomainParticipantFactory::wlReq_insertDomain(
    DDS::OpenSplice::Domain *domain)
{
    return domainList->insertElement(domain);
}

DDS::Boolean
DDS::DomainParticipantFactory::wlReq_removeDomain(
    DDS::OpenSplice::Domain *domain)
{
    return domainList->removeElement(domain);
}


DDS::DomainParticipantFactory_ptr
DDS::DomainParticipantFactory::get_instance(
)
{
    return DDS::DomainParticipantFactory::_duplicate(domainParticipantFactoryHolder.get_instance());
}

DDS::DomainParticipant_ptr
DDS::DomainParticipantFactory::create_participant (
    DDS::DomainId_t domain_id,
    const DDS::DomainParticipantQos & qos,
    DDS::DomainParticipantListener_ptr a_listener,
    DDS::StatusMask mask
) THROW_ORB_EXCEPTIONS
{
    DDS::ReturnCode_t result = DDS::RETCODE_OK;
    const DDS::DomainParticipantQos *pQos;
    DDS::OpenSplice::DomainParticipant *participant = NULL;

    CPP_REPORT_STACK();

    if (domain_id == DDS::DOMAIN_ID_INVALID) {
        result = DDS::RETCODE_BAD_PARAMETER;
        CPP_REPORT(result, "domain_id '%d' is invalid.", domain_id);
    } else {
        result = DDS::OpenSplice::Utils::qosIsConsistent(qos);
    }

    if (result == DDS::RETCODE_OK) {
        result = this->write_lock();
        if (result == DDS::RETCODE_OK) {
            if (&qos == &PARTICIPANT_QOS_DEFAULT) {
                pQos = &this->defaultParticipantQos;    // Prevent deep-copy.
            } else {
                pQos = &qos;                            // Prevent deep-copy.
            }
            this->unlock();
            participant = new DDS::OpenSplice::DomainParticipant();
            if ( participant ) {
                result = participant->init(this, domain_id, *pQos);
                if ( result == DDS::RETCODE_OK ) {
                    result = participant->nlReq_builtinTopicRegisterTypeSupport();
                }
                if ( result == DDS::RETCODE_OK ) {
                    result = this->write_lock();
                }
                if ( result == DDS::RETCODE_OK ) {
                    DDS::Boolean inserted = wlReq_insertParticipant(participant);
                    assert(inserted);
                    OS_UNUSED_ARG(inserted);
                    result = participant->set_listener(a_listener, mask);
                    if ( result == DDS::RETCODE_OK ) {
                        if (this->myQos.entity_factory.autoenable_created_entities) {
                            result = participant->enable();
                        }
                        if (result != DDS::RETCODE_OK) {
                            (void)participant->set_listener(NULL, 0);
                        }
                    }

                    if (result != DDS::RETCODE_OK) {
                        /* After a failure: remove the created object from the list. */
                        DDS::Boolean removed = wlReq_removeParticipant(participant);
                        assert(removed);
                        OS_UNUSED_ARG(removed);
                        (void) participant->deinit();
                    }

                    this->unlock();
                }
                if (result != DDS::RETCODE_OK) {
                    DDS::release(participant);
                    participant = NULL;
                }
            } else {
                result = DDS::RETCODE_OUT_OF_RESOURCES;
                CPP_REPORT(result, "Could not create DomainParticipant.");
            }
        }
    }

    CPP_REPORT_FLUSH(this, result != DDS::RETCODE_OK);

    return participant;
}

DDS::ReturnCode_t
DDS::DomainParticipantFactory::delete_participant (
    DDS::DomainParticipant_ptr a_participant
) THROW_ORB_EXCEPTIONS
{
    DDS::ReturnCode_t result = DDS::RETCODE_OK;
    DDS::OpenSplice::DomainParticipant *participant;
    DDS::DomainId_t domainId;

    CPP_REPORT_STACK();

    if (a_participant == NULL) {
        result = DDS::RETCODE_BAD_PARAMETER;
        CPP_REPORT(result, "a_participant '<NULL>' is invalid.");
    } else {
        participant = dynamic_cast<DDS::OpenSplice::DomainParticipant *>(a_participant);
        if (participant == NULL) {
            result = DDS::RETCODE_BAD_PARAMETER;
            CPP_REPORT(result, "a_participant is invalid, not of type '%s'.",
                "DDS::OpenSplice::DomainParticipant");
        }
    }

    if (result == DDS::RETCODE_OK) {
        result = this->write_lock ();
    }

    if (result == DDS::RETCODE_OK) {
        /* Be sure that this Participant is currently owned by this Factory. */
        if (wlReq_removeParticipant(participant) == FALSE) {
            /* Check if the participant is still alive. It can't be a participant from another factory,
             * since the DomainParticipantFactory is a singleton. */
            result = DDS::RETCODE_BAD_PARAMETER;
            CPP_REPORT(result, "DomainParticipant not created by DomainParticipantFactory.");
        }

        if (result == DDS::RETCODE_OK) {
            domainId = a_participant->get_domain_id();

            /* Destroy the Participant. */
            (void)participant->set_listener(NULL, 0);
            result = participant->deinit ();
            if (result == DDS::RETCODE_OK) {

                /* Check if participant is last of its domain */
                countParticipantsArg arg;
                arg.domainId = domainId;
                arg.nrOfConnectedParticipants = 0;

                (void) participantList->walk(
                        (DDS::OpenSplice::ObjSet::ObjSetActionFunc) rlReq_fnCountParticipants, &arg);

                /* Delete reference to domain if participant was last */
                if (arg.nrOfConnectedParticipants == 0) {
                    findMatchingDomainIdArg dArg;

                    dArg.domainId = domainId;
                    dArg.match = NULL;
                    (void) domainList->walk(
                            (DDS::OpenSplice::ObjSet::ObjSetActionFunc) rlReq_fnFindMatchingDomain,
                            &dArg);
                    if (dArg.match) {
                        DDS::OpenSplice::Domain *domain = dynamic_cast<DDS::OpenSplice::Domain *>(dArg.match);
                        assert(domain);
                        DDS::Boolean removed = wlReq_removeDomain(domain);
                        assert(removed);
                        OS_UNUSED_ARG(removed);
                        result = domain->deinit ();
                        DDS::release(domain);
                    }
                }
            } else {
                /* Re-insert the participant. */
                wlReq_insertParticipant(participant);
            }
        }
        this->unlock();
    }

    CPP_REPORT_FLUSH(this, result != DDS::RETCODE_OK);

    return result;
}

DDS::DomainParticipant_ptr
DDS::DomainParticipantFactory::lookup_participant (
    DDS::DomainId_t domainId
) THROW_ORB_EXCEPTIONS
{
    DDS::ReturnCode_t result;
    DDS::DomainParticipant_ptr participant = NULL;

    CPP_REPORT_STACK();

    result = this->read_lock();
    if (result == DDS::RETCODE_OK) {
        findMatchingDomainIdArg dArg;

        if (domainId == DOMAIN_ID_DEFAULT) {
            domainId = u_userGetDomainIdFromEnvUri();
        }
        dArg.domainId = domainId;
        dArg.match = NULL;
        (void) participantList->walk(
                (DDS::OpenSplice::ObjSet::ObjSetActionFunc) rlReq_fnFindMatchingParticipant,
                &dArg);
        if (dArg.match) {
            participant = dynamic_cast<DDS::DomainParticipant_ptr>(dArg.match);
            (void) DDS::DomainParticipant::_duplicate(participant);
        }
        this->unlock();
    }

    CPP_REPORT_FLUSH(this, result != DDS::RETCODE_OK);

    return participant;
}

DDS::ReturnCode_t
DDS::DomainParticipantFactory::set_qos (
    const DDS::DomainParticipantFactoryQos & qos
) THROW_ORB_EXCEPTIONS
{
    DDS::ReturnCode_t result;

    CPP_REPORT_STACK();

    result = this->write_lock();
    if (result == DDS::RETCODE_OK) {
        // OK to always adopt qos, since in case PARTICIPANTFACTORY_QOS_DEFAULT is passed we
        // need to adopt the factory default, which is unchangeable for the ParticipantFactory
        // and already initialized to the correct values.
        this->myQos = qos;
        this->unlock();
    }

    CPP_REPORT_FLUSH(this, result != DDS::RETCODE_OK);

    return result;
}

DDS::ReturnCode_t
DDS::DomainParticipantFactory::get_qos (
    DDS::DomainParticipantFactoryQos & qos
) THROW_ORB_EXCEPTIONS
{
    DDS::ReturnCode_t result;

    CPP_REPORT_STACK();

    if (&qos == &PARTICIPANTFACTORY_QOS_DEFAULT) {
        result = DDS::RETCODE_BAD_PARAMETER;
        CPP_REPORT(result, "QoS 'PARTICIPANTFACTORY_QOS_DEFAULT' is read-only.");
    } else {
        result = this->read_lock ();
    }

    if (result == DDS::RETCODE_OK) {
        // OK to always adopt myQos, since participantFactoryQos is not represented
        // in shared memory and therefore not changeable from the outside (e.g. with
        // the Tuner).
        qos = this->myQos;
        this->unlock();
    }

    CPP_REPORT_FLUSH(this, result != DDS::RETCODE_OK);

    return result;
}

DDS::Domain_ptr
DDS::DomainParticipantFactory::lookup_domain (
    DDS::DomainId_t domainId
) THROW_ORB_EXCEPTIONS
{
    DDS::ReturnCode_t result;
    DDS::OpenSplice::Domain *domain = NULL;

    CPP_REPORT_STACK();

    result = this->write_lock();
    if (result == DDS::RETCODE_OK) {
        findMatchingDomainIdArg dArg;

        if (domainId == DOMAIN_ID_DEFAULT) {
            domainId = u_userGetDomainIdFromEnvUri();
        }
        dArg.domainId = domainId;
        dArg.match = NULL;
        (void) domainList->walk(
                (DDS::OpenSplice::ObjSet::ObjSetActionFunc) rlReq_fnFindMatchingDomain,
                &dArg);
        if (dArg.match) {
            domain = dynamic_cast<DDS::OpenSplice::Domain *>(dArg.match);
        } else {
            domain = new DDS::OpenSplice::Domain ();
            if (domain) {
                result = domain->init (domainId);
                if (result == DDS::RETCODE_OK) {
                    /* We need to insert a OpenSplice::Domain pointer because we extract that as well,
                     * like when using compare_domain_id<DDS::OpenSplice::Domain *>() */
                    DDS::Boolean inserted = wlReq_insertDomain(domain);
                    assert(inserted);
                    OS_UNUSED_ARG(inserted);
                } else {
                    DDS::release(domain);
                    domain = NULL;
                }
            }
        }
        this->unlock();
    }

    CPP_REPORT_FLUSH(this, result != DDS::RETCODE_OK);

    return domain;
}

DDS::ReturnCode_t
DDS::DomainParticipantFactory::delete_domain (
    DDS::Domain_ptr a_domain
) THROW_ORB_EXCEPTIONS
{
    DDS::ReturnCode_t result;
    DDS::OpenSplice::Domain *domain = dynamic_cast<DDS::OpenSplice::Domain *>(a_domain);

    CPP_REPORT_STACK();

    if (domain == NULL) {
        result = DDS::RETCODE_BAD_PARAMETER;
        CPP_REPORT(result, "a_domain '<NULL>' is invalid.");
        return result;
    }

    result = this->write_lock ();
    if (result == DDS::RETCODE_OK) {
        if (wlReq_removeDomain(domain)) {
            result = domain->deinit ();
            if (result != DDS::RETCODE_OK) {
                DDS::Boolean inserted = wlReq_insertDomain(domain);
                assert(inserted);
                OS_UNUSED_ARG(inserted);
            }
        } else {
            result = DDS::RETCODE_PRECONDITION_NOT_MET;
            CPP_REPORT(
                result, "Domain not registered to DomainParticipantFactory.");
        }
        unlock ();
    }

    CPP_REPORT_FLUSH(this, result != DDS::RETCODE_OK);

    return result;
}

DDS::ReturnCode_t
DDS::DomainParticipantFactory::delete_contained_entities (
) THROW_ORB_EXCEPTIONS
{
    DDS::ReturnCode_t result;

    result = this->write_lock();
    if (result == DDS::RETCODE_OK) {
        result = wlReq_deleteFactoryList<DDS::OpenSplice::DomainParticipant *>(participantList);
        if(result == DDS::RETCODE_OK)
        {
            result = wlReq_deleteEntityList<DDS::OpenSplice::Domain *>(domainList);
        }
        unlock();
    }
    return result;
}

DDS::ReturnCode_t
DDS::DomainParticipantFactory::set_default_participant_qos (
    const DDS::DomainParticipantQos & qos
) THROW_ORB_EXCEPTIONS
{
    DDS::ReturnCode_t result;

    result = this->write_lock();
    if (result == DDS::RETCODE_OK) {
        // OK to always adopt qos, since in case PARTICIPANTFACTORY_QOS_DEFAULT is passed
        // we are supposed to revert to the factory default, which is exactly what the current
        // value of PARTICIPANTFACTORY_QOS_DEFAULT represents.
        this->defaultParticipantQos = qos;

        this->unlock();
    }

    return result;
}

DDS::ReturnCode_t
DDS::DomainParticipantFactory::get_default_participant_qos (
    DDS::DomainParticipantQos & qos
) THROW_ORB_EXCEPTIONS
{
    DDS::ReturnCode_t result;

    CPP_REPORT_STACK();

    result = this->read_lock();
    if (result == DDS::RETCODE_OK) {
        if (&qos == &PARTICIPANT_QOS_DEFAULT) {
            result = DDS::RETCODE_BAD_PARAMETER;
            CPP_REPORT(result, "QoS 'PARTICIPANT_QOS_DEFAULT' is read-only.");
        } else {
            qos = this->defaultParticipantQos;
        }

        this->unlock();
    }

    CPP_REPORT_FLUSH(this, result != DDS::RETCODE_OK);

    return result;
}

DDS::ReturnCode_t
DDS::DomainParticipantFactory::detach_all_domains (
     DDS::Boolean block_operations,
     DDS::Boolean delete_entities)
{
    os_uint32 flags = 0;
    u_result ures;

    if (block_operations) {
        flags |= U_USER_BLOCK_OPERATIONS;
    }
    if (delete_entities) {
        flags |= U_USER_DELETE_ENTITIES;
    }

    ures = u_userDetach(flags);

    return uResultToReturnCode (ures);
}
