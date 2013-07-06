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
#include "os.h"

#define CMX__RR_STORAGERESULT_NOTIMPLEMENTED "<rr_storageResult>NotImplemented</rr_storageResult>"

c_char*
cmx_storageOpen (
    const c_char* attrs)
{
    c_char * notImplemented;
    int notImplementedLen = 128;

    /* Generate output */
    notImplemented = (c_char*)os_malloc(notImplementedLen);
    if(notImplemented){
        int actualLen;
        actualLen = snprintf(notImplemented, notImplementedLen, "<rr_storageOpenResult>"CMX__RR_STORAGERESULT_NOTIMPLEMENTED"<rr_storage>%p</rr_storage></rr_storageOpenResult>", (void *) NULL);
        if(actualLen >= notImplementedLen){
            c_char * tmp = notImplemented;
            notImplemented = (c_char*)os_realloc(notImplemented, actualLen + 1);
            if(notImplemented){
                snprintf(notImplemented, actualLen + 1, "<rr_storageOpenResult>"CMX__RR_STORAGERESULT_NOTIMPLEMENTED"<rr_storage>%p</rr_storage></rr_storageOpenResult>", (void *) NULL);
            } else {
                os_free(tmp);
                /* NULL is returned */
            }
        }
    }

    return notImplemented;
}

c_char* /* <rr_storageResult>Success</rr_storageResult> */
cmx_storageClose (
    const c_char* storage)
{
    return os_strdup(CMX__RR_STORAGERESULT_NOTIMPLEMENTED);
}

c_char*
cmx_storageAppend (
    const c_char* storage,
    const c_char* metadata,
    const c_char* data)
{
    return os_strdup(CMX__RR_STORAGERESULT_NOTIMPLEMENTED);
}

c_char*
cmx_storageRead (
    const c_char* storage)
{
    return os_strdup("<rr_storageReadResult>"CMX__RR_STORAGERESULT_NOTIMPLEMENTED"<rr_storageReadDataXML></rr_storageReadDataXML></rr_storageReadResult>");
}

c_char*
cmx_storageGetType (
    const c_char* xmlStorage,
    const c_char* xmlTypeName)
{
    return os_strdup("<rr_storageGetTypeResult><rr_storageType></rr_storageType></rr_storageGetTypeResult>");
}
