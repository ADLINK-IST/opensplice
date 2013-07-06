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

/** \file os/linux/code/os_memMapFile.c
 *  \brief Linux 2.6 memory mapped file management
 *
 * Implements memory mapped file management for Win32
 */

#include <assert.h>
#include "os_memMapFile.h"
#include "os_report.h"


os_result
os_mmfAttrInit (
    os_mmfAttr *mmfAttr)
{
	OS_REPORT_1(OS_INFO, "os_mmfAttrInit", 0, "NOT IMPLEMENTED", NULL);
    return os_resultUnavailable;
}

os_result
os_mmfCreate(
    os_mmfHandle mmfHandle,
    os_address size)
{
	OS_REPORT_1(OS_INFO, "os_mmfAttrInit", 0, "NOT IMPLEMENTED", NULL);
    return os_resultUnavailable;
}

os_result
os_mmfOpen(
    os_mmfHandle mmfHandle)
{
	OS_REPORT_1(OS_INFO, "os_mmfAttrInit", 0, "NOT IMPLEMENTED", NULL);
    return os_resultUnavailable;
}

os_result
os_mmfClose(
    os_mmfHandle mmfHandle)
{
	OS_REPORT_1(OS_INFO, "os_mmfAttrInit", 0, "NOT IMPLEMENTED", NULL);
    return os_resultUnavailable;
}

os_result
os_mmfResize(
    os_mmfHandle mmfHandle,
    os_uint32 new_size)
{
	OS_REPORT_1(OS_INFO, "os_mmfAttrInit", 0, "NOT IMPLEMENTED", NULL);
    return os_resultUnavailable;
}

os_result
os_mmfAttach(
    os_mmfHandle mmfHandle)
{
	OS_REPORT_1(OS_INFO, "os_mmfAttrInit", 0, "NOT IMPLEMENTED", NULL);
    return os_resultUnavailable;
}

os_result
os_mmfDetach(
    os_mmfHandle mmfHandle)
{
	OS_REPORT_1(OS_INFO, "os_mmfAttrInit", 0, "NOT IMPLEMENTED", NULL);
    return os_resultUnavailable;
}

os_result
os_mmfSync(
    os_mmfHandle mmfHandle)
{
	OS_REPORT_1(OS_INFO, "os_mmfAttrInit", 0, "NOT IMPLEMENTED", NULL);
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
	OS_REPORT_1(OS_INFO, "os_mmfAttrInit", 0, "NOT IMPLEMENTED", NULL);
    return NULL;
}

void
os_mmfDestroyHandle (
    os_mmfHandle mmfHandle)
{
	OS_REPORT_1(OS_INFO, "os_mmfAttrInit", 0, "NOT IMPLEMENTED", NULL);
    return;
}

const char *
os_mmfFilename (
		os_mmfHandle mmfHandle)
{
	OS_REPORT_1(OS_INFO, "os_mmfAttrInit", 0, "NOT IMPLEMENTED", NULL);
    return NULL;
}

void *
os_mmfAddress (
		os_mmfHandle mmfHandle)
{
	OS_REPORT_1(OS_INFO, "os_mmfAttrInit", 0, "NOT IMPLEMENTED", NULL);
    return NULL;
}

os_size_t
os_mmfSize (
		os_mmfHandle mmfHandle)
{
	OS_REPORT_1(OS_INFO, "os_mmfAttrInit", 0, "NOT IMPLEMENTED", NULL);
    return 0;
}

os_boolean
os_mmfFileExist (
		os_mmfHandle mmfHandle)
{
	OS_REPORT_1(OS_INFO, "os_mmfAttrInit", 0, "NOT IMPLEMENTED", NULL);
    return OS_FALSE;
}

