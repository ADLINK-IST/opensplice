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

/** \file os/common/code/os_memMapFile.c
 *  \brief common memory mapped file implementation
 *
 * Implements memory mapped file for POSIX platforms
 */

#include <assert.h>

os_result
os_mmfCreate(
    os_mmfHandle mmfHandle,
    os_address size)
{
    os_result result = os_resultFail;
    assert(mmfHandle != NULL);
    assert(mmfHandle->filename != NULL);
    assert(mmfHandle->fd == 0);
    assert(size > 0);

    result = os_posix_mmfCreate(mmfHandle, size);

    return result;
}

os_result
os_mmfOpen(
    os_mmfHandle mmfHandle)
{
    os_result result = os_resultFail;
    assert(mmfHandle != NULL);
    assert(mmfHandle->filename != NULL);

    result = os_posix_mmfOpen(mmfHandle);

    return result;
}

os_result
os_mmfClose(
    os_mmfHandle mmfHandle)
{
    os_result result = os_resultFail;
    assert(mmfHandle != NULL);
    assert(mmfHandle->filename != NULL);

    result = os_posix_mmfClose(mmfHandle);

    return result;
}

os_result
os_mmfResize(
    os_mmfHandle mmfHandle,
    os_uint32 new_size)
{
    os_result result = os_resultFail;
    assert(mmfHandle != NULL);
    assert(mmfHandle->filename != NULL);
    assert(mmfHandle->fd != 0);
    assert(new_size > 0);

    result = os_posix_mmfResize(mmfHandle, new_size);

    return result;
}

os_result
os_mmfAttach(
    os_mmfHandle mmfHandle)
{
    os_result result = os_resultFail;
    assert(mmfHandle != NULL);
    assert(mmfHandle->filename != NULL);

    result = os_posix_mmfAttach(mmfHandle);

    return result;
}

os_result
os_mmfDetach(
    os_mmfHandle mmfHandle)
{
    os_result result = os_resultFail;
    assert(mmfHandle != NULL);
    assert(mmfHandle->filename != NULL);

    result = os_posix_mmfDetach(mmfHandle);

    return result;
}

os_result
os_mmfSync(
    os_mmfHandle mmfHandle)
{
    os_result result = os_resultFail;
    assert(mmfHandle != NULL);
    assert(mmfHandle->filename != NULL);

    result = os_posix_mmfSync(mmfHandle);

    return result;
}
