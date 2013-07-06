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

#include "mcl.h"

#include "os.h"

#define CLAIM_BLOCK 128U

struct claim
{
    void *mem;
    cfg_memoryClaimListFreeFunc free;
};

C_STRUCT(cfg_memoryClaimList)
{
    c_long nrClaims;
    struct claim *claims;
};

/**************************************************************
 * Private functions
 **************************************************************/

/**************************************************************
 * constructor/destructor
 **************************************************************/
cfg_memoryClaimList
cfg_memoryClaimListNew()
{
    cfg_memoryClaimList mcl;

    mcl = os_malloc((os_uint32)C_SIZEOF(cfg_memoryClaimList));
    if (mcl != NULL) {
        mcl->nrClaims = 0;
        mcl->claims = os_malloc((os_uint32)(sizeof(struct claim) * CLAIM_BLOCK));
        if (mcl->claims == NULL) {
            os_free(mcl);
            mcl = NULL;
        }
    }

    return mcl;
}

void
cfg_memoryClaimListFree(
    cfg_memoryClaimList mcl)
{
    if (mcl != NULL) {
        mcl->nrClaims = 0;
        os_free(mcl->claims);
        os_free(mcl);
    }
}

/**************************************************************
 * Protected functions
 **************************************************************/

/**************************************************************
 * Public functions
 **************************************************************/
void *
cfg_memoryClaimListAdd(
    cfg_memoryClaimList mcl,
    void *memory,
    cfg_memoryClaimListFreeFunc freeFunc)
{
    struct claim *newClaims;
    c_long newNrClaims;

    assert(mcl != NULL);

    if (memory != NULL) {
        newNrClaims = mcl->nrClaims + 1;
        if (newNrClaims % CLAIM_BLOCK == 0) {
            newClaims = os_malloc((os_uint32)(sizeof(struct claim) * (newNrClaims + CLAIM_BLOCK)));
            if (newClaims != NULL) {
                memcpy(newClaims, mcl->claims, sizeof(struct claim) * mcl->nrClaims);
                os_free(mcl->claims);
                mcl->claims = newClaims;
                mcl->claims[mcl->nrClaims].mem = memory;
                mcl->claims[mcl->nrClaims].free = freeFunc;
                mcl->nrClaims = newNrClaims;
            } else {
                freeFunc(memory);
                memory = NULL;
            }
        } else {
            mcl->claims[mcl->nrClaims].mem = memory;
            mcl->claims[mcl->nrClaims].free = freeFunc;
            mcl->nrClaims = newNrClaims;
        }
    }

    return memory;
}

void *
cfg_memoryClaimListRemove(
    cfg_memoryClaimList mcl,
    void *memory)
{
    c_long i,j;

    assert(mcl != NULL);

    i = 0;
    j = -1;
    while ((i < mcl->nrClaims) && (j == -1)) {
        if (C_ADDRESS(mcl->claims[i].mem) == C_ADDRESS(memory)) {
            j = i;
        }
        i++;
    }
    if (j != -1) {
        mcl->nrClaims--;
        for (i = j; i < mcl->nrClaims; i++) {
            mcl->claims[i] = mcl->claims[i + 1];
        }
    } else {
printf("Remove: mem not found!\n");
        memory = NULL;
    }

    return memory;
}

void
cfg_memoryClaimListReleaseAll(
    cfg_memoryClaimList mcl)
{
    c_long i;

    assert(mcl != NULL);

    for (i = mcl->nrClaims - 1; i >= 0; i--) {
        mcl->claims[i].free(mcl->claims[i].mem);
    }
    mcl->nrClaims = 0;
}

c_long
cfg_memoryClaimListClaimCount(
    cfg_memoryClaimList mcl)
{
    assert(mcl != NULL);

    return mcl->nrClaims;
}
