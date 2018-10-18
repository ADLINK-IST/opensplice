/*
 *                         Vortex OpenSplice
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
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

#include <os_memMapFile.h>
#include <os_report.h>


void
os_mmfAttrInit (
    os_mmfAttr *mmfAttr)
{
    OS_UNUSED_ARG (mmfAttr);
}

os_result
os_mmfCreate(
    os_mmfHandle mmfHandle,
    os_address size)
{
    OS_UNUSED_ARG (mmfHandle);
    OS_UNUSED_ARG (size);
    OS_REPORT(OS_INFO, "os_mmfCreate", 0, "NOT IMPLEMENTED");
    return os_resultUnavailable;
}

os_result
os_mmfOpen(
    os_mmfHandle mmfHandle)
{
    OS_UNUSED_ARG (mmfHandle);
    OS_REPORT(OS_INFO, "os_mmfOpen", 0, "NOT IMPLEMENTED");
    return os_resultUnavailable;
}

os_result
os_mmfClose(
    os_mmfHandle mmfHandle)
{
    OS_UNUSED_ARG (mmfHandle);
    OS_REPORT(OS_INFO, "os_mmfClose", 0, "NOT IMPLEMENTED");
    return os_resultUnavailable;
}

os_result
os_mmfResize(
    os_mmfHandle mmfHandle,
    os_uint32 new_size)
{
    OS_UNUSED_ARG (mmfHandle);
    OS_UNUSED_ARG (new_size);
    OS_REPORT(OS_INFO, "os_mmfResize", 0, "NOT IMPLEMENTED");
    return os_resultUnavailable;
}

os_result
os_mmfAttach(
    os_mmfHandle mmfHandle)
{
    OS_UNUSED_ARG (mmfHandle);
    OS_REPORT(OS_INFO, "os_mmfAttach", 0, "NOT IMPLEMENTED");
    return os_resultUnavailable;
}

os_result
os_mmfDetach(
    os_mmfHandle mmfHandle)
{
    OS_UNUSED_ARG (mmfHandle);
    OS_REPORT(OS_INFO, "os_mmfDetach", 0, "NOT IMPLEMENTED");
    return os_resultUnavailable;
}

os_result
os_mmfSync(
    os_mmfHandle mmfHandle)
{
    OS_UNUSED_ARG (mmfHandle);
    OS_REPORT(OS_INFO, "os_mmfSync", 0, "NOT IMPLEMENTED");
    return os_resultUnavailable;
}


struct os_mmfHandle_s {
    os_mmfAttr attr;              /* platform specific configuration attributes */
    void       *mapped_address;   /* memory mapped address */
    char       *filename;         /* filename of memory mapped file */
    int        fd;                /* file descriptor of memory mapped file */
    os_size_t  size;              /* size of memory mapped file */
};

os_mmfHandle
os_mmfCreateHandle (
    const char *filename,
    const os_mmfAttr *mmfAttr)
{
    OS_UNUSED_ARG (filename);
    OS_UNUSED_ARG (mmfAttr);
    OS_REPORT(OS_INFO, "os_mmfCreateHandle", 0, "NOT IMPLEMENTED");
    return NULL;
}

void
os_mmfDestroyHandle (
    os_mmfHandle mmfHandle)
{
    OS_UNUSED_ARG (mmfHandle);
    OS_REPORT(OS_INFO, "os_mmfDestroyHandle", 0, "NOT IMPLEMENTED");
    return;
}

const char *
os_mmfFilename (
        os_mmfHandle mmfHandle)
{
    OS_UNUSED_ARG (mmfHandle);
    OS_REPORT(OS_INFO, "os_mmfFilename", 0, "NOT IMPLEMENTED");
    return NULL;
}

void *
os_mmfAddress (
        os_mmfHandle mmfHandle)
{
    OS_UNUSED_ARG (mmfHandle);
    OS_REPORT(OS_INFO, "os_mmfAddress", 0, "NOT IMPLEMENTED");
    return NULL;
}

os_size_t
os_mmfSize (
        os_mmfHandle mmfHandle)
{
    OS_UNUSED_ARG (mmfHandle);
    OS_REPORT(OS_INFO, "os_mmfSize", 0, "NOT IMPLEMENTED");
    return 0;
}

os_boolean
os_mmfFileExist (
        os_mmfHandle mmfHandle)
{
    OS_UNUSED_ARG (mmfHandle);
    OS_REPORT(OS_INFO, "os_mmfFileExist", 0, "NOT IMPLEMENTED");
    return OS_FALSE;
}

