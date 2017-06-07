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

#ifndef CPP_SUPERCLASS_H_
#define CPP_SUPERCLASS_H_

#include "vortex_os.h"
#include "os_report.h"
#include "u_types.h"

#include "ccpp.h"
#include "cpp_dcps_if.h"


/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

#ifdef NDEBUG
#define CPP_CAST(type,object) static_cast<type>(object)
#else
#define CPP_CAST(type,object) dynamic_cast<type>(object)
#endif

namespace DDS
{
    namespace OpenSplice
    {
        class EntityContainer;

        typedef enum {
            UNDEFINED,

            /* Objects: (1) */
            ERRORINFO,
            DOMAINPARTICIPANTFACTORY,
            TYPESUPPORT,
            TYPESUPPORTMETAHOLDER,
            WAITSET,
            __DOMAIN,

            /* Conditions: (7) */
            CONDITION,
            STATUSCONDITION,
            GUARDCONDITION,
            READCONDITION,
            QUERYCONDITION,

            /* Entities: (12) */
            ENTITY,
            DOMAINPARTICIPANT,
            PUBLISHER,
            SUBSCRIBER,
            DATAWRITER,
            DATAREADER,
            DATAREADERVIEW,

            /* Topic Descriptions: */
            TOPICDESCRIPTION,
            TOPIC,
            CONTENTFILTEREDTOPIC,
            MULTITOPIC,

            OBJECT_COUNT
        } ObjectKind;

        class OS_API CppSuperClassInterface :
                public virtual DDS::LocalObject
        {
        public:
            virtual DDS::ReturnCode_t
            deinit() = 0;

            virtual DDS::ReturnCode_t
            wlReq_deinit() = 0;

            virtual ObjectKind
            get_kind() = 0;

            virtual ObjectKind
            rlReq_get_kind() = 0;

            virtual DDS::ReturnCode_t
            read_lock() = 0;

            virtual DDS::ReturnCode_t
            write_lock() = 0;

            virtual void
            force_write_lock() = 0;

            virtual void
            unlock() = 0;

            virtual DDS::ReturnCode_t
            check() = 0;

            virtual os_int32
            getDomainId() = 0;
			
			CppSuperClassInterface() {};
            virtual ~CppSuperClassInterface();
			
		private:
			CppSuperClassInterface(const CppSuperClassInterface&);
			CppSuperClassInterface &operator= (const CppSuperClassInterface&);
        };

        class OS_API CppSuperClass :
                public virtual CppSuperClassInterface,
                public LOCAL_REFCOUNTED_OBJECT
        {
            friend class ::DDS::OpenSplice::EntityContainer;

        private:
            const DDS::ULong    magic;
            const ObjectKind    objKind;
            os_mutex            mutex;
            os_cond             cond;
            DDS::Boolean        deinitialized;
            os_int32            domainId;

        protected:
            CppSuperClass(
                    ObjectKind kind);

            virtual ~CppSuperClass();

            virtual DDS::ReturnCode_t
            deinit();

            // no lock Required.
            DDS::ReturnCode_t
            nlReq_init();

            // write lock Required.
            virtual DDS::ReturnCode_t
            wlReq_deinit();

            // write lock Required.
            DDS::ReturnCode_t
            wlReq_wait();

            // write lock Required.
            void
            wlReq_trigger();

            virtual void
            setDomainId(
                   os_int32 _domainId);

        public:
            ObjectKind
            get_kind();

            ObjectKind
            rlReq_get_kind();

            DDS::ReturnCode_t
            read_lock();

            DDS::ReturnCode_t
            write_lock();

            void
            force_write_lock();

            void
            unlock();

            DDS::ReturnCode_t
            check();

            virtual os_int32
            getDomainId();

            static DDS::ReturnCode_t
            uResultToReturnCode(
                    u_result uResult);

            static DDS::ReturnCode_t
            osResultToReturnCode(
                    os_result osResult);

            static u_result
            ReturnCodeTouResult(
                DDS::ReturnCode_t result);

        }; /* class CppSuperClass */
    } /* namespace OpenSplice */
} /* namespace DDS */

#undef OS_API
#endif /* CPP_SUPERCLASS_H_ */
