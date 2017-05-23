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

/** \file services/serialization/code/sd__deepwalkMeta.h
 *  \brief Prototypes for the deepwalkMeta functionality, to be used by
 *         \b serializer descendants which need extended meta information.
 */

#ifndef SD_DEEPWALKMETA_H
#define SD_DEEPWALKMETA_H

#include "c_typebase.h"
#include "c_metabase.h"

#ifdef OSPL_BUILD_CORE
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */


/* A helper class for storing context information concerning the metaDeepwalk */

C_CLASS(sd_errorInfo);

typedef c_bool (*sd_deepwalkMetaFunc)(const c_char *name, c_type type, c_object *object, void *arg, sd_errorInfo *errorInfo, void *userData) __nonnull((2, 3, 4, 5)) __attribute_warn_unused_result__;
typedef c_bool (*sd_deepwalkMetaHook)(c_bool *doRecurse, const char *name, c_baseObject propOrMem, c_object *object, void *arg, sd_errorInfo *errorInfo, void *userData) __nonnull((1, 3, 4, 5, 6)) __attribute_warn_unused_result__;

C_STRUCT(sd_deepwalkMetaContext) {
  /* The deepwalk itself */
  sd_deepwalkMetaFunc actionPre;
  sd_deepwalkMetaFunc actionPost;
  sd_deepwalkMetaHook beforeAction;
  void *actionArg;
  void *userData; /* Can be used in the hook */
  /* Validation */
  sd_errorInfo errorInfo; /* To be set in case of error */
};

C_CLASS(sd_deepwalkMetaContext);

OS_API void sd_deepwalkMetaContextInit(
                           sd_deepwalkMetaContext context,
                           const sd_deepwalkMetaFunc actionPre,
                           const sd_deepwalkMetaFunc actionPost,
                           const sd_deepwalkMetaHook beforeAction,
                           void *actionArg,
                           void *userData);

OS_API void                   sd_deepwalkMetaContextDeinit(
                           sd_deepwalkMetaContext context);

OS_API c_bool                 sd_deepwalkMetaContextGetErrorInfo(
                           sd_deepwalkMetaContext context,
                           c_ulong *errorNumber,
                           c_char **name,
                           c_char **message,
                           c_char **location);


/* The function itself */

OS_API c_bool              sd_deepwalkMeta(
                           c_type type,
                           const c_char *name,
                           c_object *objectPtr,
                           sd_deepwalkMetaContext context)
  __nonnull((1, 3, 4)) __attribute_warn_unused_result__;

#undef OS_API

#endif  /* SD_DEEPWALKMETA_H */
