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

/** \file os/posix/code/os_memMapFile.c
 *  \brief Posix memory mapped file management
 *
 * Implements memory mapped file management for POSIX.
 */

#include "os_report.h"
#include "os_stdlib.h"
#include <sys/mman.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include "../common/code/os_memMapFileHandle.c"

/** Defines the permissions for the created memory mapped file */
#define OS_PERMISSION \
        (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)


/** \brief Create and open a memory mapped file with the specified initial size.
 *
 * If the file already exists, the creation fails and this operation
 * returns an error status
 */
os_result
os_posix_mmfCreate(
    os_mmfHandle mmfHandle,
    os_address size)
{
    os_result result = os_resultSuccess;
    if (mmfHandle->fd != 0) {
		OS_REPORT_1(OS_ERROR,
					"os_posix_mmfCreate", 1,
					"file %s is already open",
					mmfHandle->filename);
		result = os_resultFail;
    } else {
		/* check if file already exists */
		if (os_mmfFileExist(mmfHandle)) {
			OS_REPORT_1(OS_ERROR,
						"os_posix_mmfCreate", 1,
						"file %s already exists",
						mmfHandle->filename);
			result = os_resultFail;
		} else {
			/* create file */
			if ((mmfHandle->fd = open(mmfHandle->filename, O_RDWR|O_CREAT, OS_PERMISSION)) == -1) {
				OS_REPORT_2(OS_ERROR,
							"os_posix_mmfCreate", 1,
							"creation of file %s failed with error: %s",
							mmfHandle->filename, strerror(errno));
				mmfHandle->fd = 0;
				result = os_resultFail;
			}
			else {
				/* set file to requested size */
				if (ftruncate(mmfHandle->fd, size) == -1) {
					OS_REPORT_3(OS_ERROR,
								"os_posix_mmfCreate", 1,
								"increase size of file %s to %d bytes failed with error: %s",
								mmfHandle->filename, size, strerror(errno));
					result = os_resultFail;
					os_mmfClose(mmfHandle);
				} else {
					mmfHandle->size = size;
				}
			}
		}
    }
    return result;
}


/** \brief Open an existing memory mapped file.
 *
 * If the file doesn't exist, the open fails and this operation
 * returns an error status
 */
os_result
os_posix_mmfOpen(
    os_mmfHandle mmfHandle)
{
    os_result result = os_resultSuccess;
    struct stat sbuf;
    if (mmfHandle->fd != 0) {
		OS_REPORT_1(OS_ERROR,
					"os_posix_mmfOpen", 1,
					"file %s is already open",
					mmfHandle->filename);
		result = os_resultFail;
    } else {
		/* open file */
		if ((mmfHandle->fd = open(mmfHandle->filename, O_RDWR)) == -1) {
			OS_REPORT_2(OS_ERROR,
						"os_posix_mmfOpen", 1,
						"open of file %s failed with error: %s",
						mmfHandle->filename, strerror(errno));
			mmfHandle->fd = 0;
			result = os_resultFail;
		}
		else if (fstat(mmfHandle->fd, &sbuf) == -1) {
			OS_REPORT_2(OS_ERROR,
						"os_posix_mmfOpen", 1,
						"stat of file %s failed with error: %s",
						mmfHandle->filename, strerror(errno));
			result = os_resultFail;
			os_mmfClose(mmfHandle);
		}
		else {
			mmfHandle->size = sbuf.st_size;
		}
    }
    return result;
}


/** \brief Close an open memory mapped file.
 */
os_result
os_posix_mmfClose(
	os_mmfHandle mmfHandle)
{
    os_result result = os_resultSuccess;
    if (mmfHandle->fd != 0) {
		if (close(mmfHandle->fd) == -1) {
			OS_REPORT_2(OS_ERROR,
						"os_posix_mmfClose", 1,
						"close of file %s failed with error: %s",
						mmfHandle->filename, strerror(errno));
			result = os_resultFail;
		} else {
			mmfHandle->fd = 0;
			mmfHandle->size = 0;
		}
    }
	return result;
}


/** \brief Resize the memory mapped file.
 */
os_result
os_posix_mmfResize(
    os_mmfHandle mmfHandle,
    os_uint32 new_size)
{
    os_result result = os_resultSuccess;
    void* remapped_address;
    if (mmfHandle->fd == 0) {
		OS_REPORT_1(OS_ERROR,
					"os_posix_mmfResize", 1,
					"file %s is not open; cannot resize",
					mmfHandle->filename);
		result = os_resultFail;
    } else {
		/* set file to requested size */
		if (ftruncate(mmfHandle->fd, new_size) == -1) {
			OS_REPORT_3(OS_ERROR,
						"os_posix_mmfResize", 1,
						"resize of file %s to %d bytes failed with error: %s",
						mmfHandle->filename, new_size, strerror(errno));
			result = os_resultFail;
		} else {
			/* if memory was already mapped, remap */
			if (mmfHandle->mapped_address != 0) {
				remapped_address = mremap(mmfHandle->mapped_address,
										  mmfHandle->size,
										  new_size,
										  0);          /* no flags: don't move mapped address */
				if (remapped_address == MAP_FAILED) {
					OS_REPORT_3(OS_ERROR,
								"os_posix_mmfResize", 1,
								"mremap of file %s to %d bytes failed with error: %s",
								mmfHandle->filename, new_size, strerror(errno));
					result = os_resultFail;
				}
			}
			mmfHandle->size = new_size;
		}
    }
    return result;
}


/** \brief Attach the memory mapped file in memory.
 */
os_result
os_posix_mmfAttach(
	os_mmfHandle mmfHandle)
{
    os_result result = os_resultSuccess;
    if (mmfHandle->fd == 0) {
		OS_REPORT_1(OS_ERROR,
					"os_posix_mmfAttach", 1,
					"file %s is not open; cannot attach",
					mmfHandle->filename);
		result = os_resultFail;
    } else if (mmfHandle->mapped_address != 0) {
		OS_REPORT_1(OS_ERROR,
					"os_posix_mmfAttach", 1,
					"file %s is already attached",
					mmfHandle->filename);
		result = os_resultFail;
    } else {
		mmfHandle->mapped_address = mmap(mmfHandle->attr.map_address,
										 mmfHandle->size,
										 PROT_READ|PROT_WRITE,
										 MAP_SHARED|MAP_FIXED,
										 mmfHandle->fd,
										 0);
		if (mmfHandle->mapped_address == MAP_FAILED) {
			OS_REPORT_2(OS_ERROR,
						"os_posix_mmfAttach", 1,
						"mmap of file %s failed with error: %s",
						mmfHandle->filename, strerror(errno));
			mmfHandle->mapped_address = 0;
			result = os_resultFail;
		}
    }
    return result;
}


/** \brief Detach the memory mapped file from memory.
 */
os_result
os_posix_mmfDetach(
	os_mmfHandle mmfHandle)
{
    os_result result = os_resultSuccess;
    if (mmfHandle->mapped_address == NULL) {
		OS_REPORT_1(OS_ERROR,
					"os_posix_mmfDetach", 1,
					"file %s is not attached; cannot detach",
					mmfHandle->filename);
		result = os_resultFail;
    } else {
		if (munmap(mmfHandle->mapped_address, mmfHandle->size) == -1) {
			OS_REPORT_2(OS_ERROR,
						"os_posix_mmfDetach", 1,
						"munmap of file %s failed with error: %s",
						mmfHandle->filename, strerror(errno));
			result = os_resultFail;
		} else {
			mmfHandle->mapped_address = 0;
		}
    }
    return result;
}


/** \brief Force system to synchronize the attached memory
 *         to the memory mapped file.
 */
os_result
os_posix_mmfSync(
	os_mmfHandle mmfHandle)
{
    os_result result = os_resultSuccess;
    if (mmfHandle->mapped_address == NULL) {
		OS_REPORT_1(OS_ERROR,
					"os_posix_mmfAttach", 1,
					"file %s is not attached; cannot sync",
					mmfHandle->filename);
		result = os_resultFail;
    } else {
		if (msync(mmfHandle->mapped_address, mmfHandle->size, MS_SYNC) == -1) {
			OS_REPORT_2(OS_ERROR,
						"os_posix_mmfSync", 1,
						"msync of file %s failed with error: %s",
						mmfHandle->filename, strerror(errno));
			result = os_resultFail;
		}
    }
    return result;
}

