/*
 *                         Vortex OpenSplice
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
 *
 *   Licensed under the ADLINK Software License Agreement Rev 2.7 2nd October
 *   2014 (the "License"); you may not use this file except in compliance with
 *   the License.
 *   You may obtain a copy of the License at:
 *                      $OSPL_HOME/LICENSE
 *
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */
#include "d__capability.h"
#include "d__admin.h"
#include "d__durability.h"
#include "d__misc.h"
#include "d__configuration.h"
#include "d_message.h"
#include "vortex_os.h"



d_capability
d_capabilityNew(
    d_admin admin,
    c_ulong incarnation)
{
    d_capability capability;
    d_durability durability;
    d_configuration config;
    struct d_nameValue_s *nameValue;

    assert(d_adminIsValid(admin));

    durability = d_adminGetDurability(admin);
    config = d_durabilityGetConfiguration(durability);

    /* Allocate sampleRequest */
    capability = d_capability(os_malloc(C_SIZEOF(d_capability)));
    /* Initialize to NULL */
    memset(capability, 0, sizeof(C_STRUCT(d_capability)));
    /* Call super-init */
    d_messageInit(d_message(capability), admin);
    /* Add all my capabilities */
    nameValue = os_malloc(D_NUMBER_OF_CAPABILITIES * sizeof(struct d_nameValue_s));

    /* Add group hash capability */
    nameValue[0].name = os_strdup(D_CAPABILITY_GROUP_HASH);
    nameValue[0].value = os_malloc(sizeof(c_octet));
    ((c_octet*)nameValue[0].value)[0] = (config->capabilityGroupHash) ? 1 : 0;

    /* Add EOT support capability */
    nameValue[1].name = os_strdup(D_CAPABILITY_EOT_SUPPORT);
    nameValue[1].value = os_malloc(sizeof(c_octet));
    ((c_octet*)nameValue[1].value)[0] = (config->capabilityEOTSupport) ? 1 : 0;

    /* Add Y2038 capability */
    nameValue[2].name = os_strdup(D_CAPABILITY_Y2038READY);
    nameValue[2].value = os_malloc(sizeof(c_octet));
    ((c_octet*)nameValue[2].value)[0] = (config->capabilityY2038Ready) ? 1 : 0;

    /* Add Master selection version capability */
    nameValue[3].name = os_strdup(D_CAPABILITY_MASTER_SELECTION);
    nameValue[3].value = os_malloc(sizeof(c_ulong));
    {
        /* The value to provide is determined by the config */
        c_ulong value = config->capabilityMasterSelection;
        memcpy(nameValue[3].value, &value, sizeof(c_ulong));
    }

    /* Add incarnation number */
    nameValue[4].name = os_strdup(D_CAPABILITY_INCARNATION);
    nameValue[4].value = os_malloc(sizeof(c_ulong));
    {
        /* Set the incarnation value */
        c_ulong value = incarnation;
        memcpy(nameValue[4].value, &value, sizeof(c_ulong));
    }

    capability->capabilities = (c_sequence)nameValue;

    return capability;
}


void
d_capabilityFree(
    d_capability capability)
{
    struct d_nameValue_s *nameValue;
    c_ulong i;

    if (capability) {
        /* Clean up all existing capabilities */
        if (capability->capabilities) {

            nameValue = (struct d_nameValue_s *)capability->capabilities;

            for (i=0; i<D_NUMBER_OF_CAPABILITIES; i++) {
                os_free(nameValue[i].name);
                os_free(nameValue[i].value);
            }
            os_free(capability->capabilities);
            capability->capabilities = NULL;
        }

        d_messageDeinit(d_message(capability));
        os_free(capability);
    }
}

char *
d_capabilityToString(
    d_capability capability)
{
#define D_CAPABILITY_STRING_BLKSIZE (256)
    char *str = NULL;
    c_ulong blksize = D_CAPABILITY_STRING_BLKSIZE;
    c_ulong i,j;
    c_ulong offset = 0;
    c_ulong nameLen;
    c_ulong valueCount;
    c_ulong capabilitiesCount;
    const struct d_nameValue_s *capabilities;
    const c_octet *value;

    if (capability) {
        str = os_malloc(blksize);
        str[offset++] = '[';

        capabilitiesCount = c_sequenceSize(capability->capabilities);
        capabilities = (const struct d_nameValue_s *)capability->capabilities;

        for (i=0; i<capabilitiesCount; i++) {
            nameLen = (c_ulong)strlen(capabilities[i].name);
            valueCount = c_sequenceSize(capabilities[i].value);
            value = (const c_octet *)capabilities[i].value;

            while ((offset + /* no of characters used */
                    nameLen + /* length of the capabilities name */
                    capabilitiesCount-1 + /* ',' for after each following capability */
                    capabilitiesCount*2 + /* '(' and ')' for each capability */
                    valueCount*3 + /* max characters required for octet */
                    valueCount-1 + /* ',' for after each following octet */
                    2 /* ']' and '\0' */) > blksize) {
                blksize += D_CAPABILITY_STRING_BLKSIZE;
                str = os_realloc(str, blksize);
            }

            offset += (c_ulong)snprintf(&str[offset], blksize, "%s%s(", (i!=0)?",":"", capabilities[i].name);

            for (j=0;j<valueCount;j++) {
                offset += (c_ulong)snprintf(&str[offset], blksize, "%s%d", (j!=0)?",":"", value[j]);
            }
            offset += (c_ulong)snprintf(&str[offset], blksize, ")");
        }
        str[offset++] = ']';
        str[offset++] = '\0';
    }

#undef D_CAPABILITY_STRING_BLKSIZE
    return str;
}

