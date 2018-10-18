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


#include "c_iterator.h"
#include "c_base.h"

#include "sd_serializerXMLTypeinfo.h"

#include "idl_genMetaHelper.h"
#include "idl_catsDef.h"
#include "idl_stacDef.h"
#include "os_heap.h"

#define MIN_SUBMETA_LENGTH 100

typedef struct
{
    const char *typeName;
    const char *internalTypeName;
} BuiltinTopicInternalTypeMap;

static BuiltinTopicInternalTypeMap builtinTopicMap[] =
{
    { "DDS::ParticipantBuiltinTopicData",   "kernelModule::v_participantInfo"   },
    { "DDS::TopicBuiltinTopicData",         "kernelModule::v_topicInfo"         },
    { "DDS::PublicationBuiltinTopicData",   "kernelModule::v_publicationInfo"   },
    { "DDS::SubscriptionBuiltinTopicData",  "kernelModule::v_subscriptionInfo"  },
    { "DDS::CMParticipantBuiltinTopicData", "kernelModule::v_participantCMInfo" },
    { "DDS::CMPublisherBuiltinTopicData",   "kernelModule::v_publisherCMInfo"   },
    { "DDS::CMSubscriberBuiltinTopicData",  "kernelModule::v_subscriberCMInfo"  },
    { "DDS::CMDataWriterBuiltinTopicData",  "kernelModule::v_dataWriterCMInfo"  },
    { "DDS::CMDataReaderBuiltinTopicData",  "kernelModule::v_dataReaderCMInfo"  },
    { "DDS::TypeBuiltinTopicData",          "kernelModule::v_typeInfo"          }
};

static int builtinTopicMapSize = sizeof(builtinTopicMap)/sizeof(BuiltinTopicInternalTypeMap);

char *
idl_cutXMLmeta (
     char *meta,
     c_ulong *nrOfElements,
     size_t *descriptorLength)
{
    char *result;
    os_size_t metaLength;
    os_size_t currentPosLength;
    char *currentPos = meta;
    char *tmp;
    assert(meta != NULL);
    *nrOfElements = 1;
    metaLength = strlen(meta);
    /* We will add at most 3 chars every MIN_SUBMETA_LENGTH chars (+ \0) */
    result = os_malloc(metaLength + (metaLength/MIN_SUBMETA_LENGTH)*4 + 1);
    result[0] = 0;
    while(currentPos < meta + metaLength)
    {
        currentPosLength = strlen(currentPos);
        /* We don't want the meta string to be cut anywhere (i.e. not between a '\' and a '"')
         * let's cut it between two tags.
         * So let's find a '>' after the first MIN_SUBMETA_LENGTH chars after we check that are
         * sufficient chars to do this, if not it means we have split it sufficiently and just
         * use the remaining chars.
         */
        if(currentPosLength > MIN_SUBMETA_LENGTH)
        {
            tmp = strchr(currentPos + MIN_SUBMETA_LENGTH, '>');
            if(tmp != NULL)
            {
                ++tmp;
                strncat(result, currentPos, (size_t) (tmp - currentPos));
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
    *descriptorLength = metaLength;
    return result;
}


char *
idl_genXMLmeta (
    c_type type,
    c_bool escapeQuote)
{
    sd_serializer metaSer;
    sd_serializedData serData;
    char *metaDescription = NULL;
    c_iter replaceInfo;
    c_iter replaceInfoStac;

    replaceInfo = idl_catsDefConvertAll(idl_catsDefDefGet());
    replaceInfoStac = idl_stacDefConvertAll(idl_stacDefDefGet());
    metaSer = sd_serializerXMLTypeinfoNew (c_getBase(c_object(type)), escapeQuote);
    if (metaSer)
    {
        serData = sd_serializerSerialize (metaSer, c_object(type));
        if (serData)
        {
            metaDescription = sd_serializerToString (metaSer, serData);
            sd_serializedDataFree(serData);
        }
        sd_serializerFree (metaSer);
    }
    idl_catsDefRestoreAll(idl_catsDefDefGet(), replaceInfo);
    idl_stacDefRestoreAll(idl_stacDefDefGet(), replaceInfoStac);
    return metaDescription;
}

const char *
idl_internalTypeNameForBuiltinTopic(const char *typeName)
{
    int i;
    const char *internalName = "";

    for (i = 0; i < builtinTopicMapSize && strlen(internalName) == 0; i++) {
        if (strcmp(typeName, builtinTopicMap[i].typeName) == 0) {
            internalName = builtinTopicMap[i].internalTypeName;
        }
    }
    return internalName;

}
