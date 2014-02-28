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

#include "v__policy.h"
#include "v_kernel.h"
#include "v__messageQos.h"
#include "v__dataReaderInstance.h"
#include "v__groupInstance.h"
#include "v_public.h"

#include "c_stringSupport.h"

#include "os.h"
#include "os_report.h"

/**************************************************************
 * Private functions
 **************************************************************/

/**************************************************************
 * constructor/destructor
 **************************************************************/

/**************************************************************
 * Protected functions
 **************************************************************/
v_partitionPolicy
v_partitionPolicyAdd(
    v_partitionPolicy p,
    const c_char* expr,
    c_base base)
{
    c_iter list;
    c_char *str, *partition;
    c_long size;
    c_bool isIn;
    v_partitionPolicy newPolicy;

    newPolicy = NULL;

    assert(expr);

    if(p){
        isIn = FALSE;

        list = v_partitionPolicySplit(p);
        partition = c_iterTakeFirst(list);
    
        while(partition){
            if(strcmp(partition, expr) == 0){
               isIn = TRUE;
           }
           os_free(partition);
           partition = c_iterTakeFirst(list);
        }
        c_iterFree(list);

        if(isIn){
            /* It's already in there, so return the current value */
            newPolicy = c_stringNew(base, p);
        } else {
            /* It's not in there, so add it to the existing one */
            size = strlen(p) + 1 + strlen(expr) + 1;
            str = os_malloc(size);

            if (str) {
                os_strncpy(str, p, size);
                str = os_strcat(str, ",");
                str = os_strcat(str, expr);
                newPolicy = c_stringNew(base, str);
                os_free(str);
            } else {
                /* failed to allocate, so report error and return NULL */
                OS_REPORT(OS_ERROR, "v_partitionPolicyAdd", 0, "Failed to allocate partitionPolicy");
            }
        }
    } else {
        /* No policy exists yet, so make the expr the new policy */
        newPolicy = c_stringNew(base, expr);
    }
    return newPolicy;
}

v_partitionPolicy
v_partitionPolicyRemove(
    v_partitionPolicy p,
    const c_char *expr,
    c_base base)
{
    v_partitionPolicy newPolicy;
    c_char *str;
    c_char *start; /* start of expr in p */
    int len;

    newPolicy = NULL;
    if (p != NULL) {
        if (strcmp(p, expr) != 0) {
            len = strlen(p);
            str = (c_char *)os_malloc(len + 1);
            if (str != NULL) {
                start = strstr(p, expr);
                assert(start != NULL);
                assert((c_address)start >= (c_address)p);
                os_strncpy(str, p, (c_address)start - (c_address)p); /* includes ',' */
                str[(c_address)start - (c_address)p] = 0; /* make '\0' terminated */
                if (strcmp(start, expr) != 0) { /* not at the end */
                    os_strcat(str, (c_char *)((c_address)start + strlen(expr) + 1 /* , */));
                }
                newPolicy = c_stringNew(base, str);
                os_free(str);
            }
        }
    }

    return newPolicy;
}

c_iter
v_partitionPolicySplit(
    v_partitionPolicy p)
{
    const c_char *head, *tail;
    c_char *nibble;
    c_iter iter = NULL;
    c_long length;
    const c_char *delimiters = ",";

    if (p == NULL) return NULL;

    head = p;
    do {
        tail = c_skipUntil(head,delimiters);
        length = abs((c_address)tail - (c_address)head);
        if (length != 0) {
            length++;
            nibble = (c_string)os_malloc(length);
            os_strncpy(nibble, head, length);
            nibble[length-1]=0;
            iter = c_iterAppend(iter, nibble);
        } else {
            /* head points to one of the delimiters, so we
               add an empty string */
            length = 1;
            nibble = (c_string)os_malloc(length);
            nibble[length - 1 ] = 0;
            iter = c_iterAppend(iter, nibble);
        }
        head = tail;
        if (c_isOneOf(*head, delimiters)) {
            /* if the string ends with a delimiter, we also add an empty string */
            head++;
            if (*head == '\0') {
                length = 1;
                nibble = (c_string)os_malloc(length);
                nibble[length - 1 ] = 0;
                iter = c_iterAppend(iter, nibble);
            }
        }
    } while (*head != '\0');

    return iter;
}

/* FIXME:
 * "claim" parameter serves as a workaround for dds1784. We need to make sure
 * that the owner is not set for DataReader instances when there are no more
 * live DataWriters. Otherwise a new DataWriter with a lower strength would
 * never be able to take over again!
 *
 * THIS IS A WORKAROUND AND NEEDS A REAL FIX.
 */
v_ownershipResult
v_determineOwnershipByStrength (
    struct v_owner *owner,
    struct v_owner *candidate,
    c_bool claim)
{
    c_equality equality;
    v_ownershipResult result;

    assert (owner != NULL);
    assert (candidate != NULL);

    result = V_OWNERSHIP_NOT_OWNER;

    if (v_gidIsValid (candidate->gid)) {
        if (owner->exclusive == TRUE) {
            if (owner->exclusive == candidate->exclusive) {
                if (v_gidIsValid (owner->gid)) {
                    equality = v_gidCompare (owner->gid, candidate->gid);

                    if (candidate->strength > owner->strength) {
                        owner->strength = candidate->strength;
                        if (equality == C_EQ) {
                            result = V_OWNERSHIP_ALREADY_OWNER;
                        } else {
                            owner->gid = candidate->gid;
                            result = V_OWNERSHIP_OWNER;
                        }
                    } else if (candidate->strength < owner->strength) {
                        if (equality == C_EQ) {
                            /* The current message comes from the a writer,
                             * which is the owner AND which lowered it's
                             * strength. The strength associated with the
                             * ownership must be updated.
                             */
                            owner->strength = candidate->strength;
                            result = V_OWNERSHIP_ALREADY_OWNER;
                        } else {
                            result = V_OWNERSHIP_NOT_OWNER;
                        }
                    } else {
                        if (equality == C_EQ) {
                            result = V_OWNERSHIP_ALREADY_OWNER;
                        } else if (equality == C_GT) {
                            /* The current message comes from a writer, which
                             * is not owner AND has a strength that is equal to
                             * the strength of the current owner. So we must
                             * determine which writer should be the owner.
                             * Every reader must determine the ownership
                             * identically, so we determine it by comparing the
                             * identification of the writer. The writer with
                             * the highest gid will be the owner.
                             */
                            owner->gid = candidate->gid;
                            result = V_OWNERSHIP_OWNER;
                        } else {
                            result = V_OWNERSHIP_NOT_OWNER;
                        }
                    }
                } else if (claim == TRUE) {
                    owner->gid = candidate->gid;
                    owner->strength = candidate->strength;
                    result = V_OWNERSHIP_OWNER;
                } else {
                    /* Instance has no owner and no registrations either.
                     * This may happen during deletion of a DataReader.
                     */
                }
            } else {
                result = V_OWNERSHIP_INCOMPATIBLE_QOS;
            }
        } else {
            result = V_OWNERSHIP_SHARED_QOS;
        }
    } else {
        v_gidSetNil (owner->gid);
        result = V_OWNERSHIP_OWNER_RESET;
    }

    return result;
}

/**************************************************************
 * Public functions
 **************************************************************/
