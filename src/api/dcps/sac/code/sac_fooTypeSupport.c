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

#if 1
DDS_TypeSupport
DDS__FooTypeSupport__alloc (
    const DDS_char *type_name,
    const DDS_char *type_keys,
    const DDS_char *type_def,
    DDS_typeSupportLoad type_load,
    const DDS_copyIn copy_in,
    const DDS_copyOut copy_out,
    const DDS_unsigned_long alloc_size,
    DDS_typeSupportAllocBuffer alloc_buffer)
{
    return (DDS_TypeSupport)
        gapi_fooTypeSupport__alloc(
            (const gapi_char *)type_name,
            (const gapi_char *)type_keys,
            (const gapi_char *)type_def,
            (gapi_typeSupportLoad)type_load,
            (gapi_copyIn)copy_in,
            (gapi_copyOut)copy_out,
            (gapi_unsigned_long)alloc_size,
            (gapi_topicAllocBuffer)alloc_buffer,
            (gapi_writerCopy)NULL,
            (gapi_readerCopy)NULL); // ,
//            (gapi_createDataWriter)NULL,
//            (gapi_createDataReader)NULL);
}

DDS_ReturnCode_t
DDS__FooTypeSupport_register_type (
    DDS_TypeSupport this,
    DDS_DomainParticipant participant,
    const DDS_string name)
{
    return (DDS_ReturnCode_t)
        gapi_fooTypeSupport_register_type(
            (gapi_typeSupport)this,
            (gapi_domainParticipant)participant,
            (gapi_string)name);
}
#endif

DDS_string
DDS__FooTypeSupport_get_type_name(
    DDS_TypeSupport this)
{
    return (DDS_string)
        gapi_typeSupport_get_type_name(
            (gapi_typeSupport)this);
}

void *
DDS__FooTypeSupport_allocbuf(
    DDS_TypeSupport this,
    DDS_unsigned_long len)
{
    return
        gapi_fooTypeSupport_allocbuf(
            (gapi_typeSupport)this,
            (gapi_unsigned_long)len);
}
