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
#ifndef CCPP_DOMAINPARTICIPANTFACTORY_H
#define CCPP_DOMAINPARTICIPANTFACTORY_H

#include "CppSuperClass.h"
#include "ObjSet.h"
#include "EntityContainer.h"

#include "cpp_dcps_if.h"
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

namespace DDS
{

    namespace OpenSplice
    {
        class DomainParticipant;
        class Domain;
        class DomainParticipantFactoryHolder;
    }

    typedef DomainParticipantFactoryInterface_ptr DomainParticipantFactory_ptr;
    typedef DomainParticipantFactoryInterface_var DomainParticipantFactory_var;

    class OS_API DomainParticipantFactory
        : public virtual DomainParticipantFactoryInterface,
          public ::DDS::OpenSplice::EntityContainer,
          public DDS::OpenSplice::CppSuperClass
    {
        friend class DDS::OpenSplice::DomainParticipantFactoryHolder;

    private:
        // Instance attributes.
        DDS::DomainParticipantFactoryQos    myQos;
        DDS::DomainParticipantQos           defaultParticipantQos;
        DDS::OpenSplice::ObjSet             *participantList;
        DDS::OpenSplice::ObjSet             *domainList;

        DDS::Boolean
        wlReq_insertParticipant(DDS::OpenSplice::DomainParticipant *dp);

        DDS::Boolean
        wlReq_removeParticipant(DDS::OpenSplice::DomainParticipant *dp);

        DDS::Boolean
        wlReq_insertDomain(DDS::OpenSplice::Domain *domain);

        DDS::Boolean
        wlReq_removeDomain(DDS::OpenSplice::Domain *domain);

        typedef struct {
            DDS::DomainId_t domainId;
            int nrOfConnectedParticipants;
        } countParticipantsArg;

        static DDS::Boolean
        rlReq_fnCountParticipants(DDS::Object_ptr object, countParticipantsArg *arg);

        typedef struct {
            DDS::DomainId_t domainId;
            DDS::Object_ptr match;
        } findMatchingDomainIdArg;

        static DDS::Boolean
        rlReq_fnFindMatchingDomain(DDS::Object_ptr object, findMatchingDomainIdArg *arg);

        static DDS::Boolean
        rlReq_fnFindMatchingParticipant(DDS::Object_ptr object, findMatchingDomainIdArg *arg);

        static void
        cleanup();

    protected:
        // Constructor.
        DomainParticipantFactory();

        virtual DDS::ReturnCode_t
        init();

        DDS::ReturnCode_t
        nlReq_init();

        virtual DDS::ReturnCode_t
        wlReq_deinit();

    public:
        ~DomainParticipantFactory( );
        static DDS::DomainParticipantFactory_ptr
        get_instance();

        static DomainParticipantFactory_ptr
        _nil (void);

        static DomainParticipantFactory_ptr
        _duplicate(DomainParticipantFactory_ptr obj);

        virtual DDS::DomainParticipant_ptr
        create_participant (
                DDS::DomainId_t domainId,
                const DDS::DomainParticipantQos & qos,
                DDS::DomainParticipantListener_ptr a_listener,
                DDS::StatusMask mask
        ) THROW_ORB_EXCEPTIONS;

        virtual DDS::ReturnCode_t
        delete_participant (
                DDS::DomainParticipant_ptr a_participant
        ) THROW_ORB_EXCEPTIONS;

        virtual DDS::DomainParticipant_ptr
        lookup_participant (
                DDS::DomainId_t domainId
        ) THROW_ORB_EXCEPTIONS;

        virtual DDS::Domain_ptr
        lookup_domain (
                DDS::DomainId_t domainId
        ) THROW_ORB_EXCEPTIONS;

        virtual DDS::ReturnCode_t
        delete_domain (
                DDS::Domain_ptr a_domain
        ) THROW_ORB_EXCEPTIONS;

        virtual DDS::ReturnCode_t
        delete_contained_entities (
        ) THROW_ORB_EXCEPTIONS;

        virtual DDS::ReturnCode_t
        set_qos (
                const DDS::DomainParticipantFactoryQos & qos
        ) THROW_ORB_EXCEPTIONS;

        virtual DDS::ReturnCode_t
        get_qos (
                DDS::DomainParticipantFactoryQos & qos
        ) THROW_ORB_EXCEPTIONS;


        virtual DDS::ReturnCode_t
        set_default_participant_qos (
                const DDS::DomainParticipantQos & qos
        ) THROW_ORB_EXCEPTIONS;

        virtual DDS::ReturnCode_t
        get_default_participant_qos (
                DDS::DomainParticipantQos & qos
        ) THROW_ORB_EXCEPTIONS;

        virtual DDS::ReturnCode_t
        detach_all_domains (
                DDS::Boolean block_operations,
                DDS::Boolean delete_entities);

    };
}

#undef OS_API
#endif /* DOMAINPARTICIPANTFACTORY */
