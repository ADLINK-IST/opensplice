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
#include "gapi_typeSupport.h"
#include "gapi_domainParticipantFactory.h"
#include "gapi_domainParticipant.h"
#include "gapi_kernel.h"
#include "gapi_error.h"
#include "gapi_builtin.h"
#include "gapi_genericCopyCache.h"
#include "gapi_genericCopyIn.h"
#include "gapi_genericCopyOut.h"

#include "os_heap.h"
#include "c_metabase.h"
#include "u_user.h"
#include "v_entity.h"
#include "sd_serializerXMLMetadata.h"
#include "sd_serializerXMLTypeinfo.h"
#include "sd_typeInfoParser.h"

typedef struct {
    gapi_typeSupportLoad load_function;
    c_metaObject typeSpec;
} register_typeActionArg;

static void
register_type_action (
    v_entity e,
    c_voidp argument)
{
    register_typeActionArg *arg = (register_typeActionArg *)argument;

    c_base base = c_getBase(c_object(e));
    arg->typeSpec = arg->load_function(base);
}

static gapi_boolean
_TypeSupport_free (
    void *object)
{
    _TypeSupport ts = (_TypeSupport) object;
    gapi_boolean result;

    if (--ts->refCount == 0) {
        gapi_free(ts->type_name);
        gapi_free(ts->type_keys);
        gapi_free(ts->type_def);
        if (ts->copy_cache) {
            gapi_copyCacheFree(ts->copy_cache);
        }
        result = TRUE;
    } else {
        result = FALSE;
    }
    return result;
}


_TypeSupport
_TypeSupportNew (
    const char *type_name,
    const char *type_keys,
    const char *type_def,
    gapi_typeSupportLoad type_load,
    gapi_copyIn copy_in,
    gapi_copyOut copy_out,
    gapi_unsigned_long alloc_size,
    gapi_topicAllocBuffer alloc_buffer,
    gapi_readerCopy reader_copy,
    gapi_writerCopy writer_copy)
{
    _TypeSupport newTypeSupport = _TypeSupportAlloc();

    if (newTypeSupport) {
        newTypeSupport->refCount = 1;
        newTypeSupport->typeSpec = NULL;
        newTypeSupport->type_name = gapi_string_dup (type_name);
        newTypeSupport->type_keys = gapi_string_dup(type_keys);
        newTypeSupport->type_def = gapi_string_dup(type_def);
        newTypeSupport->type_load = type_load;
        newTypeSupport->copy_in = copy_in;
        newTypeSupport->copy_out = copy_out;
        newTypeSupport->copy_cache = NULL;
        newTypeSupport->alloc_size = alloc_size;
        newTypeSupport->alloc_buffer = alloc_buffer;
        newTypeSupport->reader_copy = reader_copy;
        newTypeSupport->writer_copy = writer_copy;
        newTypeSupport->useTypeinfo = TRUE;
    }

    return newTypeSupport;
}


gapi_typeSupport
gapi_typeSupport__alloc (
    const gapi_char *type_name,
    const gapi_char *type_keys,
    const gapi_char *type_desc)
{
    _TypeSupport typesupport;

    typesupport = _TypeSupportNew(type_name, type_keys, type_desc,
                                  NULL, NULL, NULL, 0L, NULL, NULL, NULL);
    if ( typesupport ) {
        typesupport->useTypeinfo = TRUE;
    }

    return (gapi_typeSupport)_EntityRelease(typesupport);
}


void
_TypeSupportDup (
    _TypeSupport typesupport)
{
    typesupport->refCount++;
}

gapi_typeSupportLoad
_TypeSupportTypeLoad (
    _TypeSupport typesupport)
{
    return typesupport->type_load;
}

gapi_char *
_TypeSupportTypeName (
    _TypeSupport typesupport)
{
    return typesupport->type_name;
}

gapi_char *
_TypeSupportTypeKeys (
    _TypeSupport typesupport)
{
    return typesupport->type_keys;
}

gapi_readerCopy
_TypeSupportGetReaderCopy (
    _TypeSupport typesupport)
{
    return typesupport->reader_copy;
}

gapi_writerCopy
_TypeSupportGetWriterCopy (
    _TypeSupport typesupport)
{
    return typesupport->writer_copy;
}

gapi_copyIn
_TypeSupportCopyIn (
    _TypeSupport typesupport)
{
    return typesupport->copy_in;
}

gapi_copyOut
_TypeSupportCopyOut (
    _TypeSupport typesupport)
{
    return typesupport->copy_out;
}

gapi_copyCache
_TypeSupportCopyCache (
    _TypeSupport typesupport)
{
    return typesupport->copy_cache;
}

gapi_unsigned_long
_TypeSupportTopicAllocSize (
    _TypeSupport typesupport)
{
    return typesupport->alloc_size;
}


gapi_topicAllocBuffer
_TypeSupportTopicAllocBuffer (
    _TypeSupport typesupport)
{
    return typesupport->alloc_buffer;
}


static gapi_returnCode_t
registerTypeUsingDescriptor (
    _TypeSupport typeSupport,
    _DomainParticipant participant)
{
    gapi_returnCode_t result = GAPI_RETCODE_OK;
    sd_serializer     serializer;
    sd_serializedData serData;
    c_metaObject      typeSpec;
    c_base            base;

    assert(typeSupport);
    assert(typeSupport->type_def);

    base = kernelGetBase(u_entity(_DomainParticipantUparticipant(participant)));

    if ( base ) {
        if ( typeSupport->useTypeinfo ) {
            serializer = sd_serializerXMLTypeinfoNew(base, TRUE);
        } else {
            serializer = sd_serializerXMLMetadataNew(base);
        }

        serData  = sd_serializerFromString(serializer, typeSupport->type_def);
        typeSpec = c_metaObject(sd_serializerDeserializeValidated(serializer, serData));

        typeSupport->typeSpec = typeSpec;

        if ( sd_serializerLastValidationResult(serializer) == SD_VAL_SUCCESS ) {
            result = GAPI_RETCODE_OK;
        } else {
            result = GAPI_RETCODE_PRECONDITION_NOT_MET;
        }

        sd_serializedDataFree(serData);
        sd_serializerFree(serializer);
    } else {
        result = GAPI_RETCODE_ERROR;
    }

    return result;
}

static gapi_returnCode_t
registerTypeUsingLoadFunction (
    _TypeSupport typeSupport,
    _DomainParticipant participant)
{
    gapi_returnCode_t result = GAPI_RETCODE_OK;
    register_typeActionArg arg;

    arg.load_function = typeSupport->type_load;
    if ( u_entityWriteAction(u_entity(_DomainParticipantUparticipant(participant)),
                        register_type_action, (c_voidp)&arg) == U_RESULT_OK ) {
        if ( arg.typeSpec ) {
            typeSupport->typeSpec = arg.typeSpec;
            result = GAPI_RETCODE_OK;
        } else {
            result = GAPI_RETCODE_ERROR;
        }
    } else {
        result = GAPI_RETCODE_ERROR;
    }

    return result;
}

static void
setDefaultBuiltinSettings (
    _TypeSupport typeSupport,
    const BuiltinTopicTypeInfo *builtinInfo)
{
    if ( !typeSupport->copy_out ) {
        typeSupport->copy_out = builtinInfo->defaultCopyOut;
    }
    if ( typeSupport->alloc_size == 0 ) {
        typeSupport->alloc_size = builtinInfo->defaultAllocSize;
    }
    if  ( !typeSupport->alloc_buffer ) {
        typeSupport->alloc_buffer = builtinInfo->defaultAllocBuffer;
    }
    if ( !typeSupport->reader_copy ) {
        typeSupport->reader_copy = builtinInfo->defaultReaderCopy;
    }
}

gapi_returnCode_t
gapi_typeSupport_register_type (
    gapi_typeSupport _this,
    gapi_domainParticipant dp,
    gapi_string name)
{
    gapi_returnCode_t result = GAPI_RETCODE_OK;
    _TypeSupport typeSupport       = NULL;
    _DomainParticipant participant = NULL;
    _TypeSupport oldTypeSupport    = NULL;
    const BuiltinTopicTypeInfo *builtinInfo = NULL;
    gapi_boolean typeExists = FALSE;
    const gapi_char *regName = NULL;

    typeSupport = gapi_typeSupportClaim(_this, &result);

    if ( typeSupport ) {
        builtinInfo = _BuiltinTopicFindTypeInfoByType(typeSupport->type_name);
#if 1
        if ( !typeSupport->type_def && !typeSupport->type_load ) {
            if ( !builtinInfo ) {
                result = GAPI_RETCODE_ERROR;
            }
        } else if ( typeSupport->type_def && typeSupport->type_load ) {
            result = GAPI_RETCODE_ERROR;
        }
#else
        if ( !typeSupport->type_def && !builtinInfo ) {
            result = GAPI_RETCODE_PRECONDITION_NOT_MET;
        }
#endif
    }

    if ( result == GAPI_RETCODE_OK ) {
        participant = gapi_domainParticipantClaim(dp, NULL);
        if ( !participant ) {
            result = GAPI_RETCODE_BAD_PARAMETER;
        }
    }

    if ( result == GAPI_RETCODE_OK ) {
        if ( name ) {
            regName = name;
        } else {
            regName = typeSupport->type_name;
        }
        if ( !regName ) {
            result = GAPI_RETCODE_BAD_PARAMETER;
        }
    }

    if ( result == GAPI_RETCODE_OK ) {
        oldTypeSupport = (_TypeSupport)_DomainParticipantFindType(participant, regName);
        if ( oldTypeSupport ) {
            /* registry type name is used before */
            /* OSPL-103: PRECONDITION_NOT_MET must be returned if type_keys
               does not match. _TypeSupportEquals uses a simple strcmp to
               verify, I'm assuming that's enough and the order of keys is out
               of our scope. */
            if (strcmp (typeSupport->type_name, oldTypeSupport->type_name) == 0) {
                if (strcmp (typeSupport->type_keys, oldTypeSupport->type_keys) == 0) {
                    /* check if the definition matches */
                    if ( typeSupport->type_def ) {
                        result = registerTypeUsingDescriptor(typeSupport, participant);
                    } else if ( typeSupport->type_load ) {
                        result = registerTypeUsingLoadFunction(typeSupport, participant);
                    } else {
                        /* nothing to do */
                    }
                    if (result == GAPI_RETCODE_OK ) {
                        typeExists = TRUE;
                    } else {
                        /* different type definition for the same registry name, this is not allowed */
                        result = GAPI_RETCODE_PRECONDITION_NOT_MET;
                    }
                } else {
                    /* different type supports will be registered with the same
                       registry name, this is not allowed */
                    result = GAPI_RETCODE_PRECONDITION_NOT_MET;
                    OS_REPORT_3 (OS_API_INFO,
                        "gapi_typeSupport_register_type", 0,
                        "keyList '%s' in newly registered type with typeName '%s' does not match existing keyList '%s'.",
                        typeSupport->type_keys, typeSupport->type_name, oldTypeSupport->type_keys);
                }
            } else {
                /* different type supports will be registered with the same
                   registry name, this is not allowed */
                result = GAPI_RETCODE_PRECONDITION_NOT_MET;
                OS_REPORT_2 (OS_API_INFO,
                    "gapi_typeSupport_register_type", 0,
                    "newly registered type uses typeName '%s' which is different from existing typeName '%s'. ",
                    typeSupport->type_name, oldTypeSupport->type_name);
            }
        }
    }

    if ( !typeExists ) {
        if ( result == GAPI_RETCODE_OK ) {
             _TypeSupportDup(typeSupport);
        }

        if ( result == GAPI_RETCODE_OK ) {
            if ( typeSupport->type_def ) {
                result = registerTypeUsingDescriptor(typeSupport, participant);
            } else if ( typeSupport->type_load ) {
                result = registerTypeUsingLoadFunction(typeSupport, participant);
            } else {
                /* nothing to do */
            }
        }

        if ( result == GAPI_RETCODE_OK ) {
            if( (typeSupport->copy_in  == NULL) || (typeSupport->copy_out == NULL)) {
                /* If either of the copy routines is NULL,
                   the generic copy routines will be used for both */
                result = _TypeSupportGenericCopyInit(typeSupport, participant);
            }
        }

        if ( result == GAPI_RETCODE_OK ) {
            if ( builtinInfo ) {
                setDefaultBuiltinSettings(typeSupport, builtinInfo);
            }
            result = _DomainParticipantRegisterType(participant, typeSupport, regName);
        }
    }

    _EntityRelease(participant);
    _EntityRelease(typeSupport);

    return result;
}

/*     string
 *     get_type_name();
 */
gapi_string
gapi_typeSupport_get_type_name (
    gapi_typeSupport _this)
{
    gapi_returnCode_t result = GAPI_RETCODE_OK;
    _TypeSupport typeSupport = NULL;
    char *typeName;

    typeSupport = gapi_typeSupportClaim(_this, &result);
    typeName = _TypeSupportTypeName (typeSupport);
    _EntityRelease(typeSupport);

    return gapi_string_dup(typeName);
}

/*     string
 *     get_key_list();
 */
gapi_string
gapi_typeSupport_get_key_list (
    gapi_typeSupport _this)
{
    gapi_returnCode_t result = GAPI_RETCODE_OK;
    _TypeSupport typeSupport = NULL;
    char *keyList;

    typeSupport = gapi_typeSupportClaim(_this, &result);
    keyList = _TypeSupportTypeKeys (typeSupport);
    _EntityRelease(typeSupport);

    return gapi_string_dup(keyList);
}

/* gapi_char *
 * get_description ();
 */
gapi_char *
gapi_typeSupport_get_description (
    gapi_typeSupport _this)
{
    gapi_char *description = NULL;
    gapi_char *result = NULL;
    _TypeSupport typeSupport;
    _DomainParticipant participant = NULL;
    sd_serializer serializer;
    sd_serializedData serData;
    c_base base;
    c_type type = NULL;

    typeSupport = gapi_typeSupportClaim(_this, NULL);

    if ( typeSupport ) {
        participant = _DomainParticipantFactoryFindParticipantFromType(typeSupport);

        if ( participant ) {
            u_participant uParticipant = _DomainParticipantUparticipant(participant);
            base = kernelGetBase(u_entity(uParticipant));
            if ( base ) {
                type = c_resolve(base, typeSupport->type_name);
            }
        }
        if ( type ) {
            serializer = sd_serializerXMLTypeinfoNew(base, FALSE);
            if ( serializer ) {
                serData = sd_serializerSerialize(serializer, (c_object)type);
                if ( serData ) {
                    description = sd_serializerToString(serializer, serData);
                    if ( description ) {
                        result = gapi_string_dup(description);
                        os_free(description);
                    }
                    sd_serializedDataFree(serData);
                }
                sd_serializerFree(serializer);
            }
            c_free(type);
        }
        _EntityRelease(typeSupport);
    }

    return result;
}

gapi_boolean
_TypeSupportEquals (
    _TypeSupport t1,
    _TypeSupport t2)
{
    gapi_boolean equal = TRUE;

    if ( t1->type_name && t2->type_name ) {
        if ( strcmp(t1->type_name, t2->type_name) != 0 ) {
            equal = FALSE;
        }
    } else {
        equal = FALSE;
    }

    if ( equal ) {
        if ( t1->type_keys && t2->type_keys ) {
            if ( strcmp(t1->type_keys, t2->type_keys) != 0 ) {
                equal = FALSE;
            }
        } else if ( t1->type_keys ) {
            equal = FALSE;
        } else if ( t2->type_keys ) {
            equal = FALSE;
        }
    }

    if ( equal ) {
        if ( t1->type_def && t2->type_def ) {
            if ( strcmp(t1->type_def, t2->type_def) != 0 ) {
                equal = FALSE;
            }
        }
    }

    if ( equal ) {
        if ( (t1->type_load         != t1->type_load)         ||
             (t1->copy_in           != t1->copy_in)           ||
             (t1->copy_out          != t1->copy_out)          ||
             (t1->copy_cache        != t1->copy_cache)        ||
             (t1->alloc_size        != t1->alloc_size)        ||
             (t1->alloc_buffer      != t1->alloc_buffer)      ||
             (t1->reader_copy       != t1->reader_copy)       ||
             (t1->writer_copy       != t1->writer_copy)       ||
             (t1->useTypeinfo       != t1->useTypeinfo) ) {
            equal = FALSE;
        }
    }

    return equal;
}

static void *
allocBufferFromRegistered (
    _TypeSupport typeSupport,
     gapi_unsigned_long len)
{
    _DomainParticipant participant = NULL;
    _TypeSupport       registered  = NULL;
    void              *buffer      = NULL;

    participant = _DomainParticipantFactoryFindParticipantFromType(typeSupport);

    if ( participant ) {
        registered = _DomainParticipantFindRegisteredTypeSupport(participant, typeSupport);
        if ( registered ) {
            if ( registered->alloc_buffer ) {
                buffer = registered->alloc_buffer(len);
            } else {
                if ( registered->copy_cache ) {
                    buffer = gapi_copyOutAllocBuffer(registered->copy_cache, len);
                }
            }
        }
    }
    return buffer;
}



void *
gapi_typeSupport_allocbuf (
    gapi_typeSupport _this,
    gapi_unsigned_long len)
{
    void *buffer = NULL;
    _TypeSupport typeSupport;

    typeSupport = gapi_typeSupportClaim(_this, NULL);

    if ( typeSupport ) {
        if ( typeSupport->alloc_buffer ) {
            buffer = typeSupport->alloc_buffer(len);
        } else {
            if ( typeSupport->copy_cache ) {
                buffer = gapi_copyOutAllocBuffer(typeSupport->copy_cache, len);
            } else {
                buffer = allocBufferFromRegistered(typeSupport, len);
            }
        }
        _EntityRelease(typeSupport);
    }

    return buffer;
}


typedef struct gapi_parserCallbackArg_s {
    gapi_typeParserCallback  callback;
    void                    *argument;
} gapi_parserCallbackArg;

static c_bool
gapi_parserCallback (
    sd_typeInfoKind   kind,
    c_char           *name,
    sd_list           attributes,
    void             *argument,
    sd_typeInfoHandle handle)
{
    gapi_parserCallbackArg *info = (gapi_parserCallbackArg *) argument;
    gapi_typeAttributeSeq   seq;
    gapi_long               size;
    gapi_long               i;
    c_bool                  result = FALSE;

    memset(&seq, 0, sizeof(seq));

    size = sd_listSize(attributes);
    if ( size > 0 ) {
        seq._buffer  = (gapi_typeAttribute *) os_malloc(sizeof(gapi_typeAttribute) * size);
        seq._maximum = size;
        seq._length  = size;

        for ( i = 0; i < size; i++ ) {
            sd_typeInfoAttribute *attribute = (sd_typeInfoAttribute *) sd_listAt(attributes, i);
            seq._buffer[i].name = attribute->name;
            switch ( attribute->d ) {
                case SD_TYPEINFO_ATTR_KIND_STRING:
                    seq._buffer[i].value._d = GAPI_TYPE_ATTRIBUTE_KIND_STRING;
                    seq._buffer[i].value._u.svalue = attribute->u.svalue;
                    break;
                case SD_TYPEINFO_ATTR_KIND_NUMBER:
                    seq._buffer[i].value._d = GAPI_TYPE_ATTRIBUTE_KIND_NUMBER;
                    seq._buffer[i].value._u.nvalue = attribute->u.nvalue;
                    break;
            }
        }
    }
    if ( info->callback ) {
        if ( info->callback(kind, name, &seq, handle, info->argument) ) {
            result = TRUE;
        }
    }
    if ( seq._length > 0 ) {
        os_free(seq._buffer);
    }
    return result;
}

gapi_returnCode_t
gapi_typeSupport_parse_type_description (
	const gapi_string        description,
	gapi_typeParserCallback  callback,
	void                    *argument)
{
    gapi_returnCode_t      result = GAPI_RETCODE_BAD_PARAMETER;
    gapi_parserCallbackArg info;
    sd_errorReport         errorInfo = NULL;

    info.callback = callback;
    info.argument = argument;

    if ( sd_typeInfoParserParse((c_char *)description, gapi_parserCallback, &info, &errorInfo) ) {
        result = GAPI_RETCODE_OK;
    } else {
        gapi_typeParseError(errorInfo);
        sd_errorReportFree(errorInfo);
    }
    return result;
}

gapi_returnCode_t
gapi_typeSupport_walk_type_description (
	gapi_typeParserHandle    handle,
	gapi_typeParserCallback  callback,
	void                    *argument)
{
    gapi_returnCode_t      result = GAPI_RETCODE_BAD_PARAMETER;
    gapi_parserCallbackArg info;

    if ( callback ) {
        info.callback = callback;
        info.argument = argument;

        if ( sd_typeInfoParserNext((sd_typeInfoHandle)handle, gapi_parserCallback, &info) ) {
            result = GAPI_RETCODE_OK;
        }
    } else {
        if ( sd_typeInfoParserNext((sd_typeInfoHandle)handle, NULL, NULL) ) {
            result = GAPI_RETCODE_OK;
        }
    }
    return result;
}


gapi_returnCode_t
_TypeSupportGenericCopyInit (
    _TypeSupport       typeSupport,
    _DomainParticipant participant)
{
    gapi_returnCode_t result = GAPI_RETCODE_OK;
    c_metaObject      meta_data;

    typeSupport->copy_in  = gapi_copyInStruct;
    typeSupport->copy_out = gapi_copyOutStruct;

    meta_data = _DomainParticipant_get_type_metadescription(participant, typeSupport->type_name);
    if (meta_data == NULL) {
        OS_REPORT_1(OS_ERROR, "_TypeSupportGenericCopyInit", 0,
                              "Unsupported type %s", typeSupport->type_name);
        result = GAPI_RETCODE_BAD_PARAMETER;
    } else {
        typeSupport->copy_cache = gapi_copyCacheNew(meta_data) ;
        c_free(meta_data);
        if ( typeSupport->copy_cache != NULL ) {
            if ( typeSupport->alloc_size == 0 ) {
                typeSupport->alloc_size = gapi_copyCacheGetUserSize(typeSupport->copy_cache);
            } else {
                assert(typeSupport->alloc_size == gapi_copyCacheGetUserSize(typeSupport->copy_cache));
            }
        } else {
            result = GAPI_RETCODE_OUT_OF_RESOURCES;
        }
    }
    return result;
}
