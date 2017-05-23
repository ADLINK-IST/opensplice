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
#include "os_report.h"
#include "ut_compressor.h"
#include "snappy.h"

typedef int (*_snappy_init_env)(struct snappy_env *env);
typedef void (*_snappy_free_env)(struct snappy_env *env);
typedef int (*_snappy_uncompress)(const char *compressed, size_t n, char *uncompressed);
typedef int (*_snappy_compress)(struct snappy_env *env,
            const char *input,
            size_t input_length,
            char *compressed,
            size_t *compressed_length);
typedef int (*_snappy_uncompressed_length)(const char *buf, size_t len, size_t *result);
typedef size_t (*_snappy_max_compressed_length)(size_t source_len);

typedef struct wrapper_snappy {
    os_library lib;
    struct snappy_env env;
    _snappy_init_env init_env;
    _snappy_free_env free_env;
    _snappy_uncompress uncompress;
    _snappy_compress compress;
    _snappy_uncompressed_length uncompressed_length;
    _snappy_max_compressed_length max_compressed_length;
} wrapper_snappy;

/*
 * The lzf compress wrapper will return a buffer which the
 * following layout:
 * +=============================
 * |  0  |  1  |  2  | ... |  n
 * |flags|  compressed data
 * +=============================
 */
#define SNAPPY_WRAPPER_OVERHEAD (1)

static ut_result
wrapper_snappy_compress (
    void *param,
    const void *src,
    os_size_t srcLen,
    void **dst,
    os_size_t *dstLen,
    os_size_t *compressedLen)
{
    ut_result result = UT_RESULT_OK;
    int res;
    wrapper_snappy *wrapper = param;
    os_size_t length;
    os_boolean allocation = OS_FALSE;

    assert(param);
    assert(src);
    assert(dst);
    assert(dstLen);
    assert(compressedLen);
    assert((*dst == NULL) ? (*dstLen == 0) : (*dstLen != 0));

    length = wrapper->max_compressed_length(srcLen) + SNAPPY_WRAPPER_OVERHEAD;
    if ((*dst == NULL) && (*dstLen == 0)) {
        /* Let wrapper do the allocation */
        allocation = OS_TRUE;
        if ((*dst = os_malloc(length)) == NULL) {
            result = UT_RESULT_OUT_OF_MEMORY;
            OS_REPORT(OS_ERROR, OS_FUNCTION, result,
                      "Failed to allocate memory (%"PA_PRIuSIZE" bytes) for compression buffer", length);
        } else {
            *dstLen = length;
        }
    } else if (length > *dstLen) {
        result = UT_RESULT_COUNT;
    }

    if (result == UT_RESULT_OK) {
        char *ptr = *dst;
        char *compressed = &ptr[1];
        size_t compressed_length;

        res = wrapper->compress((struct snappy_env *)&wrapper->env, (const char*)src, srcLen, compressed, &compressed_length);
        if (res != 0) {
            result = UT_RESULT_ERROR;
            OS_REPORT(OS_ERROR, OS_FUNCTION, result,
                      "Failed to compress with error '%d'", res);
            if (allocation) {
                os_free(*dst);
                *dst = NULL;
                *dstLen = 0;
            }
        } else {
            ptr[0] = 0;
            *compressedLen = compressed_length + SNAPPY_WRAPPER_OVERHEAD;
        }
    }

    return result;
}

static ut_result
wrapper_snappy_uncompress (
    void *param,
    const void *src,
    os_size_t srcLen,
    void **dst,
    os_size_t *dstLen,
    os_size_t *uncompressedLen)
{
    ut_result result = UT_RESULT_OK;
    int res;
    size_t length;
    wrapper_snappy *wrapper = param;
    os_boolean allocation = OS_FALSE;
    const char *ptr = src;
    const char *buffer;
    size_t buffer_sz;

    assert(param);
    assert(src);
    assert(dst);
    assert(dstLen);
    assert(uncompressedLen);
    assert((*dst == NULL) ? (*dstLen == 0) : (*dstLen != 0));

    buffer = &ptr[1];
    buffer_sz = srcLen - SNAPPY_WRAPPER_OVERHEAD;
    if (!wrapper->uncompressed_length(buffer, buffer_sz, &length)) {
        result = UT_RESULT_ERROR;
        OS_REPORT(OS_ERROR, OS_FUNCTION, result,
                  "Failed to retrieve length");
    } else if ((*dst == NULL) && (*dstLen == 0)) {
        /* Let wrapper do the allocation */
        allocation = OS_TRUE;
        if ((*dst = os_malloc(length)) == NULL) {
            result = UT_RESULT_OUT_OF_MEMORY;
            OS_REPORT(OS_ERROR, OS_FUNCTION, result,
                      "Failed to allocate memory (%"PA_PRIuSIZE" bytes) for decompression buffer", length);
        } else {
            *dstLen = length;
        }
    } else if (length > *dstLen) {
        result = UT_RESULT_COUNT;
    }

    if (result == UT_RESULT_OK) {
        res = wrapper->uncompress (buffer, buffer_sz, (char *)*dst);
        if (res != 0) {
            result = UT_RESULT_ERROR;
            OS_REPORT(OS_ERROR, OS_FUNCTION, result,
                      "Failed to uncompress with error '%d'", res);
            if (allocation) {
                os_free(*dst);
                *dst = NULL;
                *dstLen = 0;
            }
        } else {
            *uncompressedLen = length;
        }
    }

    return result;
}

static ut_result
wrapper_snappy_compress_maxsize (
    void *param,
    os_size_t srcLen,
    os_size_t* maxDstLen)
{
    wrapper_snappy *wrapper = param;
    assert(param);
    *maxDstLen = wrapper->max_compressed_length(srcLen) + SNAPPY_WRAPPER_OVERHEAD;
    return UT_RESULT_OK;
}

static ut_result
wrapper_snappy_uncompress_maxsize (
    void *param,
    const void *src,
    os_size_t srcLen,
    os_size_t* maxDstLen)
{
    ut_result result = UT_RESULT_ERROR;
    wrapper_snappy *wrapper = param;
    const char *ptr = src;
    const char *buffer;
    size_t buffer_sz;

    assert(src);
    assert(param);
    assert(maxDstLen);

    buffer = &ptr[1];
    buffer_sz = srcLen - SNAPPY_WRAPPER_OVERHEAD;
    if (wrapper->uncompressed_length(buffer, buffer_sz, maxDstLen)) {
        result = UT_RESULT_OK;
    }

    return result;
}

static void
wrapper_snappy_exit (
    void *param)
{
    wrapper_snappy *wrapper = param;
    assert(param);
    wrapper->free_env ((struct snappy_env *)&wrapper->env);
    os_libraryClose(wrapper->lib);
    os_free (param);
}

ut_result
ut__wrapper_snappy_init (
    ut_compressor compressor,
    const char* libName)
{
    ut_result result = UT_RESULT_ERROR;
    os_libraryAttr attr;
    wrapper_snappy *wrapper;
    char *version = "unknown";

    assert(compressor);

    if (libName == NULL) {
#ifndef INCLUDE_SNAPPY
        OS_REPORT(OS_ERROR, OS_FUNCTION, 0,
                  "The built-in snappy compressor is not available for this platform");
        return UT_RESULT_NOT_IMPLEMENTED;
#endif
        version = "snappy-c e42f0b5990";
        libName = "snappy-ospl";
    }

    os_libraryAttrInit (&attr);
    wrapper = os_malloc(sizeof(wrapper_snappy));
    wrapper->lib = os_libraryOpen(libName, &attr);
    if (wrapper->lib != NULL) {
        /* Writing: wrapper->init_env = (snappy_init_env)os_libraryGetSymbol (compressor->lib, snappy_init_env);
         * would seem more natural, but results in: warning: ISO C forbids conversion of object pointer
         * to function pointer type [-pedantic]
         * dlopen man page suggests using the used assignment based on the POSIX.1-2003 (Technical
         * Corrigendum 1) workaround.
         */
        *(void **)&wrapper->init_env = os_libraryGetSymbol (wrapper->lib, "snappy_init_env");
        *(void **)&wrapper->free_env = os_libraryGetSymbol (wrapper->lib, "snappy_free_env");
        *(void **)&wrapper->uncompress = os_libraryGetSymbol (wrapper->lib, "snappy_uncompress");
        *(void **)&wrapper->compress = os_libraryGetSymbol (wrapper->lib, "snappy_compress");
        *(void **)&wrapper->uncompressed_length = os_libraryGetSymbol (wrapper->lib, "snappy_uncompressed_length");
        *(void **)&wrapper->max_compressed_length = os_libraryGetSymbol (wrapper->lib, "snappy_max_compressed_length");

        if (wrapper->init_env &&
            wrapper->free_env &&
            wrapper->uncompress &&
            wrapper->compress &&
            wrapper->uncompressed_length &&
            wrapper->max_compressed_length &&
            wrapper->init_env (&wrapper->env) == 0) {
            compressor->param = wrapper;
            compressor->version = version;
            compressor->compfn = wrapper_snappy_compress;
            compressor->uncompfn = wrapper_snappy_uncompress;
            compressor->compmaxfn = wrapper_snappy_compress_maxsize;
            compressor->uncompmaxfn = wrapper_snappy_uncompress_maxsize;
            compressor->exitfn = wrapper_snappy_exit;
            result = UT_RESULT_OK;
        } else {
            os_libraryClose(wrapper->lib);
            os_free(wrapper);
            OS_REPORT(OS_ERROR, OS_FUNCTION, 0,
                      "At least one of required functions 'snappy_init_env,snappy_free_env,snappy_uncompress"
                      ",snappy_compress,snappy_uncompressed_length,snappy_max_compressed_length' not found");
        }
    }

    return result;
}
