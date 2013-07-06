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
#include "os_heap.h"

#include "gapi_vector.h"

C_STRUCT(gapi_vector) {
    gapi_unsigned_long  length;
    gapi_unsigned_long  maximum;
    gapi_unsigned_long  increment;
    gapi_unsigned_long  elementSize;
    gapi_char          *data;
};

gapi_vector
gapi_vectorNew (
    gapi_unsigned_long length,
    gapi_unsigned_long increment,
    gapi_unsigned_long elementSize)
{
    gapi_vector v = NULL;

    if ( elementSize > 0 ) {
        v = (gapi_vector) os_malloc(C_SIZEOF(gapi_vector));
        if ( v ) {
            v->length = length;
            v->increment = increment;
            v->elementSize = elementSize;

            v->maximum = length + increment;

            if ( v->maximum > 0 ) {
                v->data = os_malloc(v->maximum * elementSize);
                if ( v->data ) {
                    memset(v->data, 0, v->maximum * elementSize);
                } else {
                    os_free(v);
                    v = NULL;
                }
            } else {
                v->data = NULL;
            }
        }
    }

    return v;
}
                
void
gapi_vectorFree (
    gapi_vector v)
{
    if ( v->data ) {
        os_free(v->data);
    }
    os_free(v);
}

static void
gapi_vectorResize (
    gapi_vector        v,
    gapi_unsigned_long length)
{
    void *data;
    gapi_unsigned_long maximum;

    if ( v->increment ) {
        maximum = length + v->increment;

        data = os_malloc(maximum * v->elementSize);
        if ( data ) {
            memset(data, 0, maximum * v->elementSize);
            if ( v->data ) {
                memcpy(data, v->data, v->length * v->elementSize);
                os_free(v->data);
            }
            v->data    = data;
            v->length  = length;
            v->maximum = maximum;
        }
    }
}

gapi_unsigned_long
gapi_vectorGetLength (
    gapi_vector v)
{
    assert(v);

    return v->length;
}


gapi_unsigned_long
gapi_vectorSetLength (
    gapi_vector        v,
    gapi_unsigned_long length)
{
    assert(0);
    
    if ( length > v->length ) {
        gapi_vectorResize(v, length);
    }

    return v->length;
}

void *
gapi_vectorAt (
    gapi_vector v,
    gapi_unsigned_long index)
{
    void *data = NULL;
    
    assert(v);
    
    if ( index >= v->length ) {
        gapi_vectorResize(v, index+1);
    }
    if ( index < v->length ) {
        data = &v->data[index * v->elementSize];
    }

    return data;
}
