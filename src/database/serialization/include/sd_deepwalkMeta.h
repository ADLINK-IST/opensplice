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

typedef void   (*sd_deepwalkMetaFunc)(const c_char *name, c_type type,
                                      c_object *object, void *arg,
                                      sd_errorInfo *errorInfo, void *userData);

typedef c_bool (*sd_deepwalkMetaHook)(const char *name, c_baseObject propOrMem,
                                      c_object *object, void *arg,
                                      void *userData);

C_CLASS(sd_deepwalkMetaContext);

OS_API sd_deepwalkMetaContext sd_deepwalkMetaContextNew(
                           const sd_deepwalkMetaFunc actionPre,
                           const sd_deepwalkMetaFunc actionPost,
                           const sd_deepwalkMetaHook beforeAction,
                           void *actionArg,
                           c_bool doValidation,
                           void *userData);

OS_API void                   sd_deepwalkMetaContextFree(
                           sd_deepwalkMetaContext context);

OS_API c_bool                 sd_deepwalkMetaContextGetErrorInfo(
                           sd_deepwalkMetaContext context,
                           c_ulong *errorNumber,
                           c_char **name,
                           c_char **message,
                           c_char **location);


/* The function itself */

OS_API void                   sd_deepwalkMeta(
                           c_type type,
                           const c_char *name,
                           c_object *objectPtr,
                           sd_deepwalkMetaContext context);

#undef OS_API

#endif  /* SD_DEEPWALKMETA_H */
