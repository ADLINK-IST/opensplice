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
#include "gapi_common.h"
#include "gapi_entity.h"
#include "gapi_typeSupport.h"
#include "gapi_fooTypeSupport.h"

C_STRUCT(_FooTypeSupport) {
    C_EXTENDS(_TypeSupport);
};

gapi_fooTypeSupport
gapi_fooTypeSupport__alloc (
    const gapi_char *type_name,
    const gapi_char *type_keys,
    const gapi_char *type_def,
    gapi_typeSupportLoad type_load,
    gapi_copyIn copy_in,
    gapi_copyOut copy_out,
    gapi_unsigned_long alloc_size,
    gapi_topicAllocBuffer alloc_buffer,
    gapi_writerCopy writer_copy,
    gapi_readerCopy reader_copy )
{
    _TypeSupport typesupport;

    typesupport = _TypeSupportNew(type_name, type_keys,
                                  type_def, type_load,
	                          copy_in, copy_out,
                                  alloc_size, alloc_buffer,
	                          reader_copy, writer_copy);
    return (gapi_fooTypeSupport)_EntityRelease(typesupport);
}

gapi_returnCode_t
gapi_fooTypeSupport_register_type (
    gapi_typeSupport _this,
    gapi_domainParticipant dp,
    gapi_string name)
{
    return gapi_typeSupport_register_type (_this, dp, name);
}

