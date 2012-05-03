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
#include "ccpp_TypeSupport_impl.h"
#include "ccpp_DomainParticipant_impl.h"
#include "os_report.h"

DDS::TypeSupport_impl::TypeSupport_impl(
    const gapi_char *type_name,
    const gapi_char *type_keys,
    const gapi_char *type_def,
    gapi_copyIn copy_in,
    gapi_copyOut copy_out,
    gapi_readerCopy reader_copy,
    TypeSupportFactory_impl_ptr factory
)
{
    // Only proceed if all required information is available.
    if (type_name != NULL && type_keys != NULL && type_def != NULL && 
                                        copy_in != NULL && copy_out != NULL)
    {
        // If so, allocate a GAPI FooTypeSupport.
        _gapi_self = gapi_fooTypeSupport__alloc (
            type_name,
            type_keys,
            type_def,
            NULL, /* type_load */
            copy_in, /* copyIn: copy in C++ types */
            copy_out, /* copyOut: copy out C++ types */
            0,    /* alloc_size */
            NULL, /* alloc buffer */
            NULL, /* writer copy */
            reader_copy//, /* reader copy */
//            NULL, /* create datawriter */
//            NULL  /* create datareader */
        );
        
        // When successful, store handle in the user-data field of the handle..
        if (_gapi_self)
        {
            CORBA::Object_ptr anObject = dynamic_cast<CORBA::Object_ptr>(factory);
            gapi_object_set_user_data(_gapi_self, static_cast<void *>(anObject),
                                      DDS::ccpp_CallBack_DeleteUserData,NULL );
        }
        else
        {
            OS_REPORT(OS_ERROR, "CCPP", 0, "Unable to allocate TypeSupport.");
        }
    };
}
        
DDS::TypeSupport_impl::~TypeSupport_impl()
{
    // Only proceed if TypeSupport has been allocated successfully.
    if (_gapi_self)
    {
        CORBA::Object_ptr anObject;
        DDS::TypeSupportFactory_ptr factory;

        // Obtain the TypeSupportFactory object from the user_data field.
        anObject = static_cast<CORBA::Object_ptr>( gapi_object_get_user_data(_gapi_self) );
        factory = dynamic_cast<DDS::TypeSupportFactory_ptr>(anObject);
        gapi_object_set_user_data(_gapi_self,static_cast<void *>(anObject),NULL,NULL);

        // If applicable, release the pointer to the TypeSupportFactory.
        if (factory)
        {
            CORBA::release(factory);
        }

        // Free the underlying GAPI handle.
        gapi_free(_gapi_self);
    }
}

DDS::ReturnCode_t 
DDS::TypeSupport_impl::register_type(
    DDS::DomainParticipant_ptr participant,
    const char * type_name
) THROW_ORB_EXCEPTIONS
{
    bool alreadyRegistered;
    char *actual_name;
    DDS::DomainParticipant_impl *participantImpl;
    DDS::ReturnCode_t status = DDS::RETCODE_BAD_PARAMETER;
    
    // Cast the DomainParticipant pointer into its implementation class.
    participantImpl = dynamic_cast<DDS::DomainParticipant_impl *>(participant);

    // Only proceed if the cast was sucessfull.
    if (participantImpl)
    {
        gapi_domainParticipant participant_handle;
        
        // Obtain the GAPI handle from the DomainParticipant.
        participant_handle = participantImpl->_gapi_self;
        
        // Check if registered before. If type_name equals nil, then check for name of the struct.
        if (type_name)
        {
            actual_name = gapi_string_dup(const_cast<char *>(type_name));
        }
        else
        {
            actual_name = gapi_typeSupport_get_type_name(_gapi_self);
        }

        // Check if this TypeSupport was already registered before.
        alreadyRegistered = (gapi_domainParticipant_get_typesupport(participant_handle, actual_name) != NULL);

        gapi_free(actual_name);

        // Delegate the register_type method to the GAPI.
        status = gapi_fooTypeSupport_register_type(
            _gapi_self,
            participant_handle,
            reinterpret_cast<gapi_char*>(const_cast<char *>(type_name))
        );
        
        // If successful and not yet registered before, increase the RefCounter of the factory.
        if (status == DDS::RETCODE_OK && !alreadyRegistered)
        {
            DDS::TypeSupportFactory_ptr factory;
            CORBA::Object_ptr anObject;

            // Obtain the TypeSupportFactory object from the user_data field.
            anObject = static_cast<CORBA::Object_ptr>( gapi_object_get_user_data(_gapi_self) );
            factory = dynamic_cast<DDS::TypeSupportFactory_ptr>(anObject);

            // If successful, increase its RefCount by one.
            if (factory)
            {
               DDS::TypeSupportFactory::_duplicate(factory);
            }
        }
    };
    
    return status;
}

char *
DDS::TypeSupport_impl::get_type_name (
) THROW_ORB_EXCEPTIONS
{
    char * gapi_result = gapi_typeSupport_get_type_name(_gapi_self);
    char * result = CORBA::string_dup(gapi_result);
    gapi_free(gapi_result);
    return result;
}

