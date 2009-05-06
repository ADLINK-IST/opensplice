/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2009 PrismTech 
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE 
 *
 *   for full copyright notice and license terms. 
 *
 */
#include "idl_genSACPPHelper.h"

#include <os_heap.h>
#include <os_stdlib.h>

/* Return the C++ specific type identifier for the
   specified type specification
*/
c_char *
idl_sacppTypeFromTypeSpec(
    idl_typeSpec typeSpec)
{
    c_char *typeName;

    /* QAC EXPECT 3416; No side effects here */
    if (idl_typeSpecType(typeSpec) == idl_tbasic) {
        /* if the specified type is a basic type */
	switch (idl_typeBasicType(idl_typeBasic(typeSpec))) {
	case idl_short:
	    typeName = os_strdup("DDS_DCPS::Short");
	    break;
	case idl_ushort:
	    typeName = os_strdup("DDS_DCPS::UShort");
	    break;
	case idl_long:
	    typeName = os_strdup("DDS_DCPS::Long");
	    break;
	case idl_ulong:
	    typeName = os_strdup("DDS_DCPS::ULong");
	    break;
	case idl_longlong:
	    typeName = os_strdup("DDS_DCPS::LongLong");
	    break;
	case idl_ulonglong:
	    typeName = os_strdup("DDS_DCPS::ULongLong");
	    break;
	case idl_float:
	    typeName = os_strdup("DDS_DCPS::Float");
	    break;
	case idl_double:
	    typeName = os_strdup("DDS_DCPS::Double");
	    break;
	case idl_char:
	    typeName = os_strdup("DDS_DCPS::Char");
	    break;
	case idl_string:
	    typeName = os_strdup("char *");
	    break;
	case idl_boolean:
	    typeName = os_strdup("DDS_DCPS::Boolean");
	    break;
	case idl_octet:
	    typeName = os_strdup("DDS_DCPS::Octet");
	    break;
	default:
	    /* No processing required, empty statement to satisfy QAC */
	    break;
	/* QAC EXPECT 2016; Default case must be empty here */
	}
        /* QAC EXPECT 3416; No side effects here */
    } else if ((idl_typeSpecType(typeSpec) == idl_tseq) ||
	(idl_typeSpecType(typeSpec) == idl_tarray)) {
	/* sequence does not have an identification */
	typeName = os_strdup ("");
	printf ("idl_corbaCxxTypeFromTypeSpec: Unexpected type handled\n");
        assert(0);
    } else {
        /* if a user type is specified build it from its scope and its name.
	   The type should be one of idl_ttypedef, idl_tenum, idl_tstruct,
           idl_tunion.
	*/
        typeName = idl_scopeStackCxx(
            idl_typeUserScope(idl_typeUser(typeSpec)),
            "::",
            idl_typeSpecName(typeSpec));
    }
    return typeName;
    /* QAC EXPECT 5101; The switch statement is simple, therefor the total complexity is low */
}
