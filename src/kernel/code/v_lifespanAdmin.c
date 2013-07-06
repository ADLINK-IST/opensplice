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
#include "v__lifespanAdmin.h"
#include "v_time.h"
#include "os_report.h"

/**************************************************************
 * Private functions
 **************************************************************/
#define CHECK_ADMIN(admin,sample)

/**************************************************************
 * constructor/destructor
 **************************************************************/
v_lifespanAdmin
v_lifespanAdminNew(
    v_kernel kernel)
{
    v_lifespanAdmin admin;

    admin = NULL;
    admin = c_new(c_resolve(c_getBase(kernel), "kernelModule::v_lifespanAdmin"));
    if (admin != NULL) {
        admin->sampleCount = 0;
        admin->head = NULL;
        admin->tail = NULL;
    } else {
        OS_REPORT(OS_ERROR,
                  "v_lifespanAdminNew",0,
                  "Failed to allocate lifespan admin.");
        assert(FALSE);
    }
    return admin;
}

/**************************************************************
 * Protected functions
 **************************************************************/
void
v_lifespanAdminInsert(
    v_lifespanAdmin admin,
    v_lifespanSample sample)
{
    v_lifespanSample placeHolder;
    c_equality eq;

    assert(C_TYPECHECK(admin,v_lifespanAdmin));
    assert(C_TYPECHECK(sample,v_lifespanSample));
    assert(admin->sampleCount >= 0);

    CHECK_ADMIN(admin, sample);
    eq = c_timeCompare(sample->expiryTime, C_TIME_INFINITE);
    if (eq == C_EQ) {
        return; /* no insert, since sample never expires! */
    }

    if (admin->head == NULL) {
        assert(admin->tail == NULL);
        admin->head = c_keep(sample);
        admin->tail = c_keep(sample);
    } else {
        placeHolder = admin->tail;
        eq = c_timeCompare(placeHolder->expiryTime, sample->expiryTime);
        while ((placeHolder->prev != NULL) && (eq != C_LT) /* >= */) {
            placeHolder = placeHolder->prev;
            if (placeHolder != NULL) {
                eq = c_timeCompare(placeHolder->expiryTime, sample->expiryTime);
            }
        }
        if (eq != C_LT) { /* insert before placeholder */
            assert(placeHolder == admin->head);
            sample->next = admin->head; /* transfer ref count */
            admin->head->prev = sample;
            admin->head = c_keep(sample);
        } else {
            if (placeHolder->next != NULL) {                
                placeHolder->next->prev = sample;
            } else {
                assert(placeHolder == admin->tail);
                c_free(admin->tail);
                admin->tail = c_keep(sample);
	    }
            sample->next = placeHolder->next; /* transfer refcount */
            placeHolder->next = c_keep(sample);
            sample->prev = placeHolder;
        }
    }
    admin->sampleCount++;
    CHECK_ADMIN(admin, sample);
}

void
v_lifespanAdminRemove(
    v_lifespanAdmin admin,
    v_lifespanSample sample)
{
    c_equality eq;

    assert(C_TYPECHECK(admin,v_lifespanAdmin));
    assert(C_TYPECHECK(sample,v_lifespanSample));

    eq = c_timeCompare(sample->expiryTime, C_TIME_INFINITE);
    if (eq == C_EQ) {
        return; /* no insert, since sample never expires! */
    }

    CHECK_ADMIN(admin, sample);
    if (sample == admin->head) {
        assert(sample->prev == NULL);
        if (sample == admin->tail) {
            assert(c_refCount(sample) > 2);
            assert(sample->next == NULL);
            c_free(sample); /* free tail reference */
            admin->head = NULL;
            admin->tail = NULL;
        } else {
            assert(c_refCount(sample) > 1);
            admin->head = sample->next; /* transfer refcount */
            if (sample->next) {
                sample->next = NULL;
                admin->head->prev = NULL; /* or sample->next->prev = NULL */
            }
        }
        c_free(sample); /* free head reference */
        admin->sampleCount--;
    } else {
        if (sample == admin->tail) {
            assert(sample->prev);
            assert(c_refCount(sample) > 2); /* Both tail and next field from prev sample. */
            assert(sample->next == NULL);
            c_free(admin->tail);
            admin->tail = c_keep(sample->prev);
            sample->prev = NULL;
            c_free(admin->tail->next); /* admin->tail->next == sample */
            admin->tail->next = NULL;
            admin->sampleCount--;
        } else {
            if ((sample->next != NULL) && (sample->prev != NULL)) {
                assert(c_refCount(sample) > 1);
                assert(admin->sampleCount > 0);
                v_lifespanSample(sample->prev)->next = sample->next; /* transfer refcount */
                sample->next->prev = sample->prev;
                sample->next = NULL;
                sample->prev = NULL;
                c_free(sample);
                admin->sampleCount--;
            } /* else sample not in admin, so no removing needed */
        }
    }

    CHECK_ADMIN(admin, sample);
}

void
v_lifespanAdminTakeExpired(
    v_lifespanAdmin admin,
    v_lifespanSampleAction action,
    c_voidp arg)
{
    c_bool proceed;
    v_lifespanSample removed;
    c_time now;
    c_equality eq;

    assert(C_TYPECHECK(admin,v_lifespanAdmin));

    CHECK_ADMIN(admin, NULL);
    if (admin->head != NULL) {
        now = v_timeGet();
        eq = c_timeCompare(now, admin->head->expiryTime);
        proceed = TRUE;
        while ((proceed) && (eq != C_LT /* >= */) && (admin->head != NULL)) {
            removed = admin->head;
            if (action) {
                proceed = action(removed, arg);
            } else {
                proceed = TRUE;
            }
            if ((proceed) && (removed == admin->head)) {
               /* The action routine might have already removed the sample, so 
                * we check if the head of the list has not changed! 
                */
                admin->head = removed->next; /* transfer refcount */
                removed->next = NULL;
                if (admin->head == NULL) {
                    assert(removed == admin->tail);
                    c_free(admin->tail);
                    admin->tail = NULL;
                    proceed = FALSE;
                } else {
                    admin->head->prev = NULL;
                }
                assert(admin->sampleCount > 0);
                admin->sampleCount--;
                CHECK_ADMIN(admin, removed);
                c_free(removed);
            }
            if (admin->head != NULL) {
                eq = c_timeCompare(now, admin->head->expiryTime);
            }
        }
    }
    CHECK_ADMIN(admin, NULL);
}

c_long
v_lifespanAdminSampleCount(
    v_lifespanAdmin admin)
{
    return admin->sampleCount;
}

/**************************************************************
 * Public functions
 **************************************************************/
