/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2011 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE
 *
 *   for full copyright notice and license terms.
 *
 */
#ifndef COLL_COMPARE_H
#define COLL_COMPARE_H

#if defined (__cplusplus)
extern "C" {
#endif

int
stringIsLessThen(
    void *left,
    void *right);

int
pointerIsLessThen(
    void *left,
    void *right);

#if defined (__cplusplus)
}
#endif

#endif /* COLL_COMPARE_H */
