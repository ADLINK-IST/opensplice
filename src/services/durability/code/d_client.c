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

#include "d__client.h"
#include "d__thread.h"
#include "d__durability.h"
#include "d__misc.h"
#include "d__admin.h"
#include "d_networkAddress.h"
#include "os_heap.h"

d_client
d_clientNew(
    d_networkAddress address)
{
    d_client client;

    /* Allocate client object */
    client = d_client(os_malloc(C_SIZEOF(d_client)));
    if (client) {
        /* Call super-init */
        d_lockInit(d_lock(client), D_CLIENT,
                   (d_objectDeinitFunc)d_clientDeinit);
        /* Initialize the client */
        client->address = d_networkAddressNew(address->systemId,
                                              address->localId,
                                              address->lifecycleId);
        client->clientId.prefix              = 0;
        client->clientId.suffix              = 0;
        client->readers                      = 0; /* No readers detected yet for this client */
        client->requiredReaders              = D_BASIC_CLIENT_DURABILITY_READER_FLAGS;
        client->isConfirmed                  = FALSE;  /* set to TRUE when clientId is set */
    }
    return client;
}


void
d_clientDeinit(
    d_client client)
{
    assert(d_clientIsValid(client));

    if (client->address) {
        d_networkAddressFree(client->address);
        client->address = NULL;
    }
    /* Call super-deinit */
    d_lockDeinit(d_lock(client));

}


void
d_clientFree(
    d_client client)
{
    assert(d_clientIsValid(client));

    d_objectFree(d_object(client));
}


int
d_clientCompareByAddress(
    d_client client1,
    d_client client2)
{
    int result = 0;

    if (client1 != client2) {
        result = d_networkAddressCompare(client1->address, client2->address);
    }
    return result;
}


d_networkAddress
d_clientGetAddress(
    d_client client)
{
    assert(d_clientIsValid(client));

    return d_networkAddressNew(client->address->systemId, client->address->localId, client->address->lifecycleId);
}


void
d_clientAddReader(
    d_client client,
    c_ulong reader)
{
    assert(d_clientIsValid(client));

    d_lockLock(d_lock(client));
    if ((client->readers & reader) == 0) {
        d_durability durability = d_threadsDurability();
        d_networkAddress clientAddr;

        clientAddr = d_clientGetAddress(client);
        client->readers |= reader;
        d_printTimedEvent(durability, D_LEVEL_FINER,
            "Client durability reader %04lx for federation %u discovered (readers: %04lx, required readers: %04lx).\n",
            reader, clientAddr->systemId, client->readers, client->requiredReaders);
        d_networkAddressFree(clientAddr);
    }
    d_lockUnlock(d_lock(client));
}



void
d_clientRemoveReader(
    d_client client,
    c_ulong reader)
{
    assert(d_clientIsValid(client));

    /* Note: there is an issue when there are more than 1 clients per participant.
     * The first client that leaves will remove the readers and hence prevent communication
     */

    d_lockLock(d_lock(client));
    if ((client->readers & reader) == reader) {
        d_durability durability = d_threadsDurability();
        d_admin admin = durability->admin;
        d_networkAddress clientAddr;
        d_client result;

        clientAddr = d_clientGetAddress(client);
        client->readers &= (~reader);
        d_printTimedEvent(durability, D_LEVEL_FINER,
            "Client durability reader %lx for federation %u lost (readers: %lx, requiredReaders: %lx).\n",
            reader, clientAddr->systemId, client->readers, client->requiredReaders);
        if (client->readers == 0) {
            /* No client durability reader exists for this client anymore.
             * Remove the client. When the client reappears discovery
             * of readers will be started again
             */
            d_printTimedEvent(durability, D_LEVEL_FINER,
                "All client durability readers for federation %u have been lost\n",
                clientAddr->systemId);
            if ((result = d_adminRemoveClient(admin, client)) != NULL) {
                d_clientFree(result);
            }
        }
        d_networkAddressFree(clientAddr);
    }
    d_lockUnlock(d_lock(client));
}

 /* Client that sets the requestId will be confirmed by definition */
void
d_clientSetClientId(
    d_client client,
    struct _DDS_Gid_t clientId)
{
    assert(d_clientIsValid(client));

    if (!client->isConfirmed) {
        d_durability durability = d_threadsDurability();

        d_lockLock(d_lock(client));
        client->clientId = clientId;
        client->isConfirmed = TRUE;

        d_printTimedEvent(durability, D_LEVEL_FINER,
            "Confirming client for federation %u\n", client->address->systemId);

        d_lockUnlock(d_lock(client));
    }
}


c_bool
d_clientIsConfirmed(
    d_client client)
{
    c_bool result;

    assert(d_clientIsValid(client));

    d_lockLock(d_lock(client));
    result = client->isConfirmed;
    d_lockUnlock(d_lock(client));
    return result;
}


/**
 * \brief Check if all readers in the mask have been discovered for the client
 *
 * If waitForRemoteReaders is FALSE then there is no need to discover readers,
 * so TRUE is returned.
 */
static c_bool
d_clientHasDiscoveredReaders(
    d_client client,
    c_ulong mask,
    c_bool waitForRemoteReaders)
{
    c_bool result = TRUE;

    assert(d_clientIsValid(client));

    d_lockLock(d_lock(client));
    if (waitForRemoteReaders) {
        /* Check if the discovered readers of the fellow
         * match the ones specified in the mask.
         */
        result = (((client->readers) & (mask)) == (mask));
    }
    d_lockUnlock(d_lock(client));
    return result;

}


c_bool
d_clientIsResponsive(
    d_client client,
    c_ulong requiredReaders,
    c_bool waitForRemoteReaders)
{
    assert(d_clientIsValid(client));

    return (d_clientHasDiscoveredReaders(client, requiredReaders, waitForRemoteReaders) && d_clientIsConfirmed(client));
}

