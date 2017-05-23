/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR PrismTech
 *   Limited, its affiliated companies and licensors. All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */
#include "vortex_os.h"

#define CMX__RR_STORAGERESULT_NOTIMPLEMENTED "<rr_storageResult>NotImplemented</rr_storageResult>"

c_char*
cmx_storageOpen (
    const c_char* attrs)
{
    c_char * notImplemented;
    int notImplementedLen = 128;
    int actualLen;

    /* Generate output */
    notImplemented = (c_char*)os_malloc(notImplementedLen);
    actualLen = snprintf(notImplemented, notImplementedLen, "<rr_storageOpenResult>"CMX__RR_STORAGERESULT_NOTIMPLEMENTED"<rr_storage>%p</rr_storage></rr_storageOpenResult>", (void *) NULL);
    if(actualLen >= notImplementedLen){
        c_char * tmp = notImplemented;
        notImplemented = (c_char*)os_realloc(notImplemented, actualLen + 1);
        snprintf(notImplemented, actualLen + 1, "<rr_storageOpenResult>"CMX__RR_STORAGERESULT_NOTIMPLEMENTED"<rr_storage>%p</rr_storage></rr_storageOpenResult>", (void *) NULL);
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
