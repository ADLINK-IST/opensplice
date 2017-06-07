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
#ifndef UT_COMPRESSOR_H
#define UT_COMPRESSOR_H

#if defined (__cplusplus)
extern "C" {
#endif

#include "vortex_os.h"
#include "ut_result.h"

#ifdef OSPL_BUILD_CORE
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif

/* Function to compress data */
typedef ut_result (*ut_compressor_compfn)(
        void *param,
        const void *src,
        os_size_t srcLen,
        void **dest,
        os_size_t *destLen,
        os_size_t *compressedLen);

/* Function to uncompress data */
typedef ut_result (*ut_compressor_uncompfn)(
        void *param,
        const void *src,
        os_size_t srcLen,
        void **dst,
        os_size_t *dstLen,
        os_size_t *uncompressedLen);

/* Function determine worst case length that can be be expected when
 * compressing data with the given length.
 */
typedef ut_result (*ut_compressor_compmaxfn)(
        void *param,
        os_size_t srcLen,
        os_size_t *maxDstLen);

/* Function determine worst case length that can be be expected when
 * uncompressing data.
 */
typedef ut_result (*ut_compressor_uncompmaxfn)(
        void *param,
        const void *src,
        os_size_t srcLen,
        os_size_t *maxDstLen);

/* Function to clean up compressor resources */
typedef void (*ut_compressor_exitfn)(
        void *param);

/* Compressor declaration and definition */
OS_STRUCT(ut_compressor){
    ut_compressor_compfn compfn;
    ut_compressor_uncompfn uncompfn;
    ut_compressor_compmaxfn compmaxfn;
    ut_compressor_uncompmaxfn uncompmaxfn;
    ut_compressor_exitfn exitfn;
    void* param;
    const os_char *version;
};

OS_CLASS(ut_compressor);

/** Allocates a new compressor. The following compression algorithms
  * are supported:
  * - lzf
  * - snappy
  * - zlib
  * - custom (user-defined)
  *
  * @param libName The name of the compression library to dynamically load.
  *                The library must be part of the LD_LIBRARY_PATH (POSIX)
  *                or PATH (Windows) environment to allow it to be loaded.
  *                To use the built-in zlib, lzf or snappy compression,
  *                this parameter must be NULL.
  * @param initFunc The name of the function to initialize the compression
  *                 algorithm. The signature of initFunc needs to be:
  *
  *                 int initFunc(ut_compressor, void** param);
  *
  *                 The first parameter points to the compressor and
  *                 the initFunc is responsible to set at least the
  *                 maxfn, compfn and uncompfn attributes. Optionally,
  *                 the exitfn and param attributes may be set. The param
  *                 attribute is an opaque attribute that will be passed to
  *                 all compressor functions.
  *                 - maxfn - Needs to set maxDstLen to the maximum length
  *                           data with srcLen will become when uncompressing.
  *                 - compfn - Needs to compress data in src and store in dst
  *                 - uncompfn - Needs to uncompress data in src and store in
  *                              dst
  *                 - exitfn - Needs to clean up all resources used by the
  *                            compressor.
  *                 The second parameter of initFunc is passed on from the
  *                 parameter attribute of the ut_compressorNew function.
  *                 When initFunc returns 0, initialization is
  *                 expected to be successful. With any other return code
  *                 it is assumed that initialization failed and the
  *                 ut_compressorNew function will return NULL.
  *
  *                 To use one of the built-in compressors use
  *                 the following names:
  *                 - zlib: "zlib" or "ut__wrapper_zlib_init"
  *                 - lzf: "lzf" or "ut__wrapper_lzf_init"
  *                 - snappy: "snappy" or "ut__wrapper_snappy_init"
  * @param parameter Parameter to be passed to initFunc.
  * @returns The compressor if initialization succeeded or NULL otherwise.
  */
OS_API ut_compressor
ut_compressorNew(
    const os_char *libName,
    const os_char *initFunc,
    const os_char *parameter);

/** Get the version of the compressor library used.
  *
  * @param compressor The compressor to get version from.
  *
  * @return NULL if a bad parameter is provided.
  *         pointer to version.
  */
OS_API const os_char *
ut_compressorVersion(
    ut_compressor compressor);

/** Compresses src into dst while taking into account srcLen and dstLen
  *
  * @param compressor An initialized compressor.
  * @param src The source that points to the uncompressed data.
  * @param srcLen The size of src
  * @param[in,out] dst The destination to store the compressed data.
  *                    Can be NULL to let the compressor allocate the
  *                    buffer or a pre-allocated buffer.
  * @param[in,out] dstLen The size of dst. Should be 0 if the compressor
  *                       should allocate the buffer then this function will
  *                       store the size of the allocated buffer.
  * @param[out] compressedLen Out parameter where this function will store
  *                      the size of the compressed data.
  * @return UT_RESULT_OK if successful,
  *         UT_RESULT_ILL_PARAM if a bad parameter is provided.
  *         UT_RESULT_COUNT if a pre-allocated buffer is supplied that
  *                         is not large enough
  *         UT_RESULT_OUT_OF_MEMORY if not enough memory is available
  *         UT_RESULT_ERROR if an internal error occurred.
  */
OS_API ut_result
ut_compressorCompress(
    ut_compressor compressor,
    const void *src,
    os_size_t srcLen,
    void **dst,
    os_size_t *dstLen,
    os_size_t *compressedLen);

/** Uncompresses src into dst while taking into account srcLen and dstLen
  *
  * @param compressor An initialized compressor.
  * @param src The source that points to the compressed data.
  * @param srcLen The size of src
  * @param[in,out] dst The destination to store the compressed data.
  *                    Can be NULL to let the compressor allocate the
  *                    buffer or a pre-allocated buffer.
  * @param[in,out] dstLen The size of dst. Should be 0 if the compressor
  *                       should allocate the buffer then this function will
  *                       store the size of the allocated buffer.
  * @param[out] uncompressedLen Out parameter where this function will store
  *                        the size of the uncompressed data.
  * @return UT_RESULT_OK if successful,
  *         UT_RESULT_ILL_PARAM if a bad parameter is provided.
  *         UT_RESULT_COUNT if a pre-allocated buffer is supplied that
  *                         is not large enough
  *         UT_RESULT_OUT_OF_MEMORY if not enough memory is available
  *         UT_RESULT_ERROR if an internal error occurred.
  */
OS_API ut_result
ut_compressorUncompress(
    ut_compressor compressor,
    const void *src,
    os_size_t srcLen,
    void **dst,
    os_size_t *dstLen,
    os_size_t *uncompressedLen);

/** Determines maximum size of compressed data given a uncompressed data size
  *
  * @param compressor An initialized compressor.
  * @param srcLen The size of the compressed data.
  * @param[out] maxDstLen Out parameter where this function will store
  *                       the worst case size of the compressed data.
  *                       Will be 0 when unable the compressor is unable to
  *                       determine the maximum size.
  * @return UT_RESULT_OK if successful,
  *         UT_RESULT_ILL_PARAM if a bad parameter is provided.
  *         UT_RESULT_OUT_OF_MEMORY if not enough memory is available
  *         UT_RESULT_ERROR if an internal error occurred.
  */
OS_API ut_result
ut_compressorCompressMaxLen(
    ut_compressor compressor,
    os_size_t srcLen,
    os_size_t *maxDstLen);

/** Determines maximum size of uncompressed data given compressed data.
  *
  * @param compressor An initialized compressor.
  * @param src The source that points to the compressed data.
  * @param srcLen The size of the compressed data.
  * @param[out] maxDstLen Out parameter where this function will store
  *                       the worst case size of the uncompressed data.
  *                       Will be 0 when unable the compressor is unable to
  *                       determine the maximum size.
  * @return UT_RESULT_OK if successful,
  *         UT_RESULT_ILL_PARAM if a bad parameter is provided.
  *         UT_RESULT_OUT_OF_MEMORY if not enough memory is available
  *         UT_RESULT_ERROR if an internal error occurred.
  */
OS_API ut_result
ut_compressorUncompressMaxLen(
    ut_compressor compressor,
    const void *src,
    os_size_t srcLen,
    os_size_t *maxDstLen);

/** Frees the given compressor and all its resources. As part of this function
  * it will call the exitfn routine that is stored in the compressor if it is
  * set.
  *
  * @param compressor The compressor to free.
  */
OS_API void
ut_compressorFree(
    ut_compressor compressor);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif /* UT_COMPRESSOR_H */
