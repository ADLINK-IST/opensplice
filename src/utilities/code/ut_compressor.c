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
#include "vortex_os.h"
#include "os_report.h"
#include "c_typebase.h"
#include "ut__wrapper_lzf.h"
#include "ut__wrapper_snappy.h"
#include "ut__wrapper_zlib.h"

#include "ut_compressor.h"

ut_compressor
ut_compressorNew(
    const os_char *libName,
    const os_char *initFunc,
    const os_char *parameter)
{
    ut_compressor compressor = NULL;
    assert(initFunc);

    OS_UNUSED_ARG(parameter);

    compressor = os_malloc(C_SIZEOF(ut_compressor));
    compressor->compfn = NULL;
    compressor->uncompfn = NULL;
    compressor->compmaxfn = NULL;
    compressor->uncompmaxfn = NULL;
    compressor->exitfn = NULL;
    compressor->param = NULL;

    if ((strcmp(initFunc, "lzf") == 0) ||
        (strcmp(initFunc, "ut__wrapper_lzf_init") == 0)) {
        if (ut__wrapper_lzf_init(compressor, libName) != UT_RESULT_OK) {
            os_free(compressor);
            compressor = NULL;
        }
    } else if ((strcmp(initFunc, "snappy") == 0) ||
               (strcmp(initFunc, "ut__wrapper_snappy_init") == 0)) {
        if (ut__wrapper_snappy_init(compressor, libName) != UT_RESULT_OK) {
            os_free(compressor);
            compressor = NULL;
        }
    } else if ((strcmp(initFunc, "zlib") == 0) ||
               (strcmp(initFunc, "ospl_comp_zlib_init") == 0)) {
        if (ut__wrapper_zlib_init(compressor, libName, parameter) != UT_RESULT_OK) {
            os_free(compressor);
            compressor = NULL;
        }
    } else {
        OS_REPORT (OS_WARNING, OS_FUNCTION, 0, "Custom compressor is not supported");
        os_free(compressor);
        compressor = NULL;
    }

    return compressor;
}

const os_char *
ut_compressorVersion(
    ut_compressor compressor)
{
    const os_char *version = NULL;
    if (compressor) {
        version = compressor->version;
    }
    return version;
}

ut_result
ut_compressorCompress(
    ut_compressor compressor,
    const void *src,
    os_size_t srcLen,
    void **dst,
    os_size_t *dstLen,
    os_size_t *compressedLen)
{
    ut_result result = UT_RESULT_ILL_PARAM;

    if ((compressor) && (compressor->compfn)) {
        result = compressor->compfn(compressor->param, src, srcLen, dst, dstLen, compressedLen);
    }

    return result;
}

ut_result
ut_compressorUncompress(
    ut_compressor compressor,
    const void *src,
    os_size_t srcLen,
    void **dst,
    os_size_t *dstLen,
    os_size_t *uncompressedLen)
{
    ut_result result = UT_RESULT_ILL_PARAM;

    if ((compressor) && (compressor->uncompfn)) {
        result = compressor->uncompfn(compressor->param, src, srcLen, dst, dstLen, uncompressedLen);
    }

    return result;
}

ut_result
ut_compressorCompressMaxLen(
    ut_compressor compressor,
    os_size_t srcLen,
    os_size_t* maxDstLen)
{
    ut_result result = UT_RESULT_ILL_PARAM;

    if ((compressor) && (compressor->compmaxfn)) {
        result = compressor->compmaxfn(compressor->param, srcLen, maxDstLen);
    }

    return result;
}

ut_result
ut_compressorUncompressMaxLen(
    ut_compressor compressor,
    const void *src,
    os_size_t srcLen,
    os_size_t* maxDstLen)
{
    ut_result result = UT_RESULT_ILL_PARAM;

    if ((compressor) && (compressor->uncompmaxfn)) {
        result = compressor->uncompmaxfn(compressor->param, src, srcLen, maxDstLen);
    }

    return result;
}


void
ut_compressorFree(
    ut_compressor compressor)
{
    if (compressor) {
        if (compressor->exitfn) {
            compressor->exitfn(compressor->param);
        }
        os_free(compressor);
    }
}

