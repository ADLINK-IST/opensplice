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
#include "u__types.h"

c_char *
u_resultImage(
    u_result result)
{
    c_char *image = NULL;

#define _CASE_(o) case o: image = #o; break;
    switch (result) {
    _CASE_(U_RESULT_UNDEFINED);
    _CASE_(U_RESULT_OK);
    _CASE_(U_RESULT_INTERRUPTED);
    _CASE_(U_RESULT_NOT_INITIALISED);
    _CASE_(U_RESULT_OUT_OF_MEMORY);
    _CASE_(U_RESULT_INTERNAL_ERROR);
    _CASE_(U_RESULT_ILL_PARAM);
    _CASE_(U_RESULT_CLASS_MISMATCH);
    _CASE_(U_RESULT_DETACHING);
    _CASE_(U_RESULT_TIMEOUT);
    _CASE_(U_RESULT_INCONSISTENT_QOS);
    _CASE_(U_RESULT_IMMUTABLE_POLICY);
    _CASE_(U_RESULT_PRECONDITION_NOT_MET);
    _CASE_(U_RESULT_ALREADY_DELETED);
    _CASE_(U_RESULT_UNSUPPORTED);
    default:
        image = "Internal error: no image for illegal result value";
    break;
    }
    return image;
#undef _CASE_
}

c_char *
u_kindImage(
    u_kind kind)
{
    c_char *image = NULL;

#define _CASE_(o) case o: image = #o; break;
    switch (kind) {
    _CASE_(U_UNDEFINED);
    _CASE_(U_ENTITY);
    _CASE_(U_PARTICIPANT);
    _CASE_(U_PUBLISHER);
    _CASE_(U_WRITER);
    _CASE_(U_SERVICE);
    _CASE_(U_SERVICEMANAGER);
    _CASE_(U_SUBSCRIBER);
    _CASE_(U_READER);
    _CASE_(U_NETWORKREADER);
    _CASE_(U_GROUPQUEUE);
    _CASE_(U_QUERY);
    _CASE_(U_DATAVIEW);
    _CASE_(U_PARTITION);
    _CASE_(U_TOPIC);
    _CASE_(U_GROUP);
    _CASE_(U_WAITSET);
    _CASE_(U_DOMAIN);
    default:
        image = "Internal error: no image for illegal result value";
    break;
    }
    return image;
#undef _CASE_
}

