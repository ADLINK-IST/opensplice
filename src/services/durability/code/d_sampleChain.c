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

#include "d_sampleChain.h"
#include "d__admin.h"
#include "d__durability.h"
#include "d__misc.h"
#include "d_message.h"
#include "d_networkAddress.h"
#include "os_report.h"
#include "vortex_os.h"

d_sampleChain
d_sampleChainNew(
    d_admin admin,
    const c_char* partition,
    const c_char* topic,
    d_durabilityKind kind,
    d_networkAddress source)
{
    d_sampleChain sampleChain = NULL;

    assert(admin);
    assert(partition);
    assert(topic);

    if(admin && partition && topic){
        sampleChain = d_sampleChain(os_malloc(C_SIZEOF(d_sampleChain)));
        d_messageInit(d_message(sampleChain), admin);

        sampleChain->msgBody._d           = BEAD;
        sampleChain->msgBody._u.bead.size = 0;
        sampleChain->partition            = os_strdup(partition);
        sampleChain->topic                = os_strdup(topic);
        sampleChain->durabilityKind       = kind;
        sampleChain->addressees           = NULL;
        sampleChain->addresseesCount      = 0;
        sampleChain->source.systemId      = source->systemId;
        sampleChain->source.localId       = source->localId;
        sampleChain->source.lifecycleId   = source->lifecycleId;
    }
    return sampleChain;
}

void
d_sampleChainFree(
    d_sampleChain sampleChain)
{
    assert(sampleChain);

    if(sampleChain){
        if(sampleChain->partition){
            os_free(sampleChain->partition);
        }
        if(sampleChain->topic){
            os_free(sampleChain->topic);
        }
        if(sampleChain->addressees){
            os_free(sampleChain->addressees);
        }
        switch(sampleChain->msgBody._d){
            case LINK:
                /*Do nothing here*/
                break;
            case BEAD:
                if(sampleChain->msgBody._u.bead.size > 0){
                    os_free(sampleChain->msgBody._u.bead.value);
                }
                break;
            default:
                OS_REPORT(OS_ERROR,"d_sampleChainFree",0,
                            "Illegal message discriminator value (%d) detected.",
                            sampleChain->msgBody._d);
                assert(FALSE);
                break;
        }
        os_free(sampleChain);
    }
    return;
}

int
d_sampleChainCompare(
    d_sampleChain sampleChain1,
    d_sampleChain sampleChain2)
{
    int result = 0;
    if(sampleChain1 != sampleChain2){
        if(!sampleChain1 && !sampleChain2){
            result = 0;
        } else if(sampleChain1 && !sampleChain2){
            result = 1;
        } else if(!sampleChain1 && sampleChain2) {
            result = -1;
        } else {
            result = strcmp(sampleChain1->partition, sampleChain2->partition);

            if(result == 0){
                result = strcmp(sampleChain1->topic, sampleChain2->topic);

                if(result == 0){
                    if(sampleChain1->durabilityKind == sampleChain2->durabilityKind){
                        if(d_message(sampleChain1)->senderAddress.systemId == d_message(sampleChain2)->senderAddress.systemId){
                            if(d_message(sampleChain1)->addressee.systemId == d_message(sampleChain2)->addressee.systemId){
                                if(d_message(sampleChain1)->sequenceNumber == d_message(sampleChain2)->sequenceNumber){
                                    result = 0;
                                }
                            }
                        }
                    } else if(sampleChain1->durabilityKind > sampleChain2->durabilityKind){
                        result = 1;
                    } else {
                        result = -1;
                    }
                }
            }
        }
    }
    return result;
}

c_bool
d_sampleChainContainsAddressee(
    d_sampleChain sampleChain,
    d_networkAddress addressee)
{
    c_bool found;
    c_ulong i;
    d_networkAddress address;

    found = FALSE;

    if (sampleChain) {
        for(i=0; i<sampleChain->addresseesCount && !found; i++){
            address = &(d_networkAddress(sampleChain->addressees)[i]);

            if (d_networkAddressEquals(address, addressee)) {
                found = TRUE;
            }
        }
    }
    return found;
}

int
d_sampleChainPrintAddressees(
    c_char *buffer,
    c_size length,
    d_sampleChain chain)
{
    const c_char *space = "";
    c_size offset = 0;
    c_ulong index;
    d_networkAddress address;
    int count, total = 0;

    assert (buffer != NULL);
    assert (chain != NULL);

    for (index = 0; index < chain->addresseesCount && total >= 0; index++) {
        address = &d_networkAddress(chain->addressees)[index];
        assert (address != NULL);

        count = snprintf (
            buffer+offset, length-offset, "%s%u", space, address->systemId);
        if (count >= 0) {
            if ((c_size)count >= (length - offset)) {
                offset = length;
            } else {
                offset += (c_size)count;
            }
            total += count;
        } else {
            total = -1;
        }

        space = " ";
    }

    return total;
}
