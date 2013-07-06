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

#ifndef SAC_COMMON_H
#define SAC_COMMON_H

#define CHECK_NO_DATA(r,d,i) \
    if ( r == DDS_RETCODE_NO_DATA ) {\
        ((_Sequence *)d)->_length = 0;\
        ((_Sequence *)i)->_length = 0;\
    }


typedef struct _Sequence {
    gapi_unsigned_long  _maximum;
    gapi_unsigned_long  _length;
    void               *_buffer;
    gapi_boolean        _release;
} _Sequence;

#endif
