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
#ifndef GAPI_VECTOR_H
#define GAPI_VECTOR_H

#include "gapi.h"
#include "gapi_common.h"

C_CLASS(gapi_vector);

gapi_vector
gapi_vectorNew (
    gapi_unsigned_long length,
    gapi_unsigned_long increment,
    gapi_unsigned_long elementSize);

void
gapi_vectorFree (
    gapi_vector v);

gapi_unsigned_long
gapi_vectorGetLength (
    gapi_vector v);

gapi_unsigned_long
gapi_vectorSetLength (
    gapi_vector        v,
    gapi_unsigned_long length);

void *
gapi_vectorAt (
    gapi_vector v,
    gapi_unsigned_long index);

#endif
