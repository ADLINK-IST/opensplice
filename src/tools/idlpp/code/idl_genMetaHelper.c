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


#include "c_iterator.h"
#include "c_base.h"

#include "sd_serializerXMLTypeinfo.h"

#include "idl_genMetaHelper.h"
#include "idl_catsDef.h"
#include "idl_stacDef.h"
#include "os_heap.h"

#define MIN_SUBMETA_LENGTH 100

char *
idl_cutXMLmeta (
     char *meta,
     c_ulong *nrOfElements)
{
    char *result;
    int metaLength;
    int currentPosLength;
    char *currentPos = meta;
    char *tmp;
    assert(meta != NULL);
    *nrOfElements = 1;
    metaLength = strlen(meta);
    /* We will add at most 3 chars every MIN_SUBMETA_LENGTH chars (+ \0) */
    result = os_malloc(metaLength + ((int)(metaLength/MIN_SUBMETA_LENGTH))*4 + 1);
    result[0] = 0;
    while(currentPos < meta + metaLength)
    {
        currentPosLength = strlen(currentPos);         
        /* We don't want the meta string to be cut anywhere (i.e. not between a '\' and a '"')
         * let's cut it between two tags.
         * So let's find a '>' after the first MIN_SUBMETA_LENGTH chars after we check that are
         * sufficient chars to do this, if not it means we have split it sufficiently and just 
         * use the remaining chars. */
        if(currentPosLength > MIN_SUBMETA_LENGTH)
        {
            tmp = strchr(currentPos + MIN_SUBMETA_LENGTH, '>');
            if(tmp != NULL)
            {
                ++tmp;
                strncat(result, currentPos, tmp - currentPos);
                currentPos = tmp;
                if(currentPos < meta + metaLength)
                {
                    strcat(result, "\",\n\"");
                    (*nrOfElements)++;
                }
            }
            else
            {
	        printf("\nERROR: Malformed XML meta identified! No closing element > found\n"); 
                exit (-1);
	    }
	}
        else
	{
            strcat(result, currentPos);
            currentPos = meta + metaLength;
        }
    }
    return result;
}


char *
idl_genXMLmeta (
    c_type type)
{
    sd_serializer metaSer;
    sd_serializedData serData;
    char *metaDescription = NULL;
    c_iter replaceInfo;
    c_iter replaceInfoStac;

    replaceInfo = idl_catsDefConvertAll(idl_catsDefDefGet());
    replaceInfoStac = idl_stacDefConvertAll(idl_stacDefDefGet());
    metaSer = sd_serializerXMLTypeinfoNew (c_getBase(c_object(type)), TRUE);
    if (metaSer)
    {
        serData = sd_serializerSerialize (metaSer, c_object(type));
        if (serData)
        {
	    metaDescription = sd_serializerToString (metaSer, serData);
	}
        sd_serializerFree (metaSer);
    }
    idl_catsDefRestoreAll(idl_catsDefDefGet(), replaceInfo);
    idl_stacDefRestoreAll(idl_stacDefDefGet(), replaceInfoStac);
    return metaDescription;
}
