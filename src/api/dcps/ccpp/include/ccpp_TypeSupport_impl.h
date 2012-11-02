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
#ifndef __DCPS_TYPESUPPORT_IMPL_H
#define __DCPS_TYPESUPPORT_IMPL_H

#include "gapi.h"
#include "ccpp.h"
#include "ccpp_Utils.h"
#include "ccpp_Publisher_impl.h"
#include "ccpp_Subscriber_impl.h"
#include "ccpp_DataReader_impl.h"
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

namespace DDS {

    class OS_DCPS_API TypeSupportFactory_impl : 
            public virtual ::DDS::TypeSupportFactory,
            public LOCAL_REFCOUNTED_OBJECT
    {
        friend class ::DDS::Subscriber_impl;
        friend class ::DDS::Publisher_impl;
        friend class ::DDS::DomainParticipant_impl;
        friend class ::DDS::DataReader_impl;

    public:
        TypeSupportFactory_impl() {};
        virtual ~TypeSupportFactory_impl() {};

    private:
        virtual ::DDS::DataReader_ptr
        create_datareader (gapi_dataReader handle) = 0;

        virtual ::DDS::DataWriter_ptr
        create_datawriter (gapi_dataWriter handle) = 0;

        virtual ::DDS::DataReaderView_ptr
        create_view (gapi_dataReaderView handle) = 0;
    };

    typedef TypeSupportFactory_impl *TypeSupportFactory_impl_ptr;

    class OS_DCPS_API TypeSupport_impl : 
            public virtual ::DDS::TypeSupport,
            public LOCAL_REFCOUNTED_OBJECT
    {
    private:
      gapi_typeSupport _gapi_self;

    protected:

    	TypeSupport_impl(
            const gapi_char *type_name,
            const gapi_char *type_keys,
            const gapi_char *type_def,
            gapi_copyIn copy_in,
            gapi_copyOut copy_out,
            gapi_readerCopy reader_copy,
            TypeSupportFactory_impl_ptr factory
        );

    public:

        virtual ~TypeSupport_impl();

        virtual ::DDS::ReturnCode_t register_type(
            ::DDS::DomainParticipant_ptr participant,
            const char * type_name
        ) THROW_ORB_EXCEPTIONS;

        virtual char * get_type_name(
        ) THROW_ORB_EXCEPTIONS;
    };

    typedef TypeSupport_impl *TypeSupport_impl_ptr;
}

#endif // __DCPS_TYPESUPPORT_IMPL_H
