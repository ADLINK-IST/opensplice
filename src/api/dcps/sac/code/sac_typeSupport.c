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

DDS_TypeSupport
DDS_TypeSupport__alloc(
    const DDS_char *type_name,
    const DDS_char *type_keys,
    const DDS_char *type_desc)
{
   return (DDS_TypeSupport)
   gapi_typeSupport__alloc(
            (const gapi_char *)type_name,
            (const gapi_char *)type_keys,
            (const gapi_char *)type_desc);
}

DDS_ReturnCode_t
DDS_TypeSupport_register_type(
    DDS_TypeSupport this,
    DDS_DomainParticipant domain,
    const DDS_string name)
{
    return (DDS_ReturnCode_t)
        gapi_fooTypeSupport_register_type(
            (gapi_typeSupport)this,
            (gapi_domainParticipant)domain,
            (gapi_string)name);
}

DDS_char *
DDS_TypeSupport_get_description(
    DDS_TypeSupport this)
{
    return (DDS_char *)
        gapi_typeSupport_get_description(
            (gapi_typeSupport)this);
}

DDS_char *
DDS_TypeSupport_get_key_list(
    DDS_TypeSupport this)
{
    return (DDS_char *)
        gapi_typeSupport_get_key_list(
            (gapi_typeSupport)this);
}

DDS_char *
DDS_TypeSupport_get_type_name(
    DDS_TypeSupport _this)
{
    return (DDS_char *)
        gapi_typeSupport_get_type_name(
            (gapi_typeSupport)_this);
}

void *
DDS_TypeSupport_allocbuf(
    DDS_TypeSupport this,
    DDS_unsigned_long len)
{
    return
        gapi_typeSupport_allocbuf(
            (gapi_typeSupport)this,
            (gapi_unsigned_long)len);
}

DDS_ReturnCode_t
DDS_TypeSupport_parse_type_description(
	const DDS_string        description,
	DDS_TypeParserCallback  callback,
	void                   *argument)
{
    return (DDS_ReturnCode_t)
        gapi_typeSupport_parse_type_description(
            (const gapi_string)description,
            (gapi_typeParserCallback)callback,
            argument);

}

DDS_ReturnCode_t
DDS_TypeSupport_walk_type_description(
	DDS_TypeParserHandle    handle,
	DDS_TypeParserCallback  callback,
	void                   *argument)
{
    return (DDS_ReturnCode_t)
        gapi_typeSupport_walk_type_description(
            (gapi_typeParserHandle)handle,
            (gapi_typeParserCallback)callback,
            argument);
}
