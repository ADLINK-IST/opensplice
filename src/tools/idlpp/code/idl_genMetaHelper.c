#include "idl_genMetaHelper.h"

#include <sd_serializerXMLTypeinfo.h>
#include <c_base.h>

char *
idl_genXMLmeta (
    c_type type)
{
    sd_serializer metaSer;
    sd_serializedData serData;
    char *metaDescription = NULL;

    metaSer = sd_serializerXMLTypeinfoNew (c_getBase(c_object(type)), TRUE);
    if (metaSer) {
	serData = sd_serializerSerialize (metaSer, c_object(type));
	if (serData) {
	    metaDescription = sd_serializerToString (metaSer, serData);
	}
	sd_serializerFree (metaSer);
    }
    return metaDescription;
}
