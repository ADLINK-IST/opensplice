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
#include "idl_registerType.h"
#include "idl_tmplExp.h"

#include "os_heap.h"
#include "os_stdlib.h"
#include <errno.h>
#include "c_iterator.h"
#include "sd_serializerXMLTypeinfo.h"

#include <ctype.h>

static void
idl_reportOpenError(
    char *fname)
{
    printf ("Error opening file %s for writing. Reason: %s (%d)\n", fname, strerror( errno ), errno);
    exit (-1);
}

/** @brief generate name which will be used as a macro to prevent multiple inclusions
 *
 * From the specified basename create a macro which will
 * be used to prevent multiple inclusions of the generated
 * header file. The basename characters are translated
 * into uppercase characters and the append string is
 * appended to the macro.
 */
static c_char *
idl_macroFromBasename(
    const char *basename,
    const char *append)
{
    static c_char macro[200];
    c_long i;

    for (i = 0; i < (c_long)strlen(basename); i++) {
        macro[i] = toupper (basename[i]);
        macro[i+1] = '\0';
    }
    os_strncat(macro, append, (size_t)((int)sizeof(macro)-(int)strlen(append)));

    return macro;
}

static char *
idl_cScopedTypeName (
    const char *typeName
    )
{
    char *src;
    char *cTypeName;
    char *dst;

    src = (char *)typeName;
    dst = os_malloc (strlen(src)+1);
    cTypeName = dst;
    while (*src != '\0') {
	if (*src != ':') {
	    *dst = *src;
	    dst++;
	} else {
	    src++;
	    if (*src == ':') {
		*dst = '_';
	    } else {
		*dst = *src;
	    }
	    dst++;
	}
	src++;
    }
    *dst = *src;

    return cTypeName;
}

static void
idl_registerBodyHeader (
    const char *basename
    )
{
    idl_fileOutPrintf (idl_fileCur(), "#include <c_metabase.h>\n");
    idl_fileOutPrintf (idl_fileCur(), "#include <sd_serializerXMLTypeinfo.h>\n");
    idl_fileOutPrintf (idl_fileCur(), "\n");
    idl_fileOutPrintf (idl_fileCur(), "#include \"%s_register.h\"\n", basename);
}

static void
idl_registerBody (
    const char *typeName,
    const char *metaDescriptor
    )
{
    idl_fileOutPrintf (idl_fileCur(), "\n");
    idl_fileOutPrintf (idl_fileCur(), "c_metaObject\n");
    idl_fileOutPrintf (idl_fileCur(), "%s__register_type (\n", idl_cScopedTypeName(typeName));
    idl_fileOutPrintf (idl_fileCur(), "    c_base base\n");
    idl_fileOutPrintf (idl_fileCur(), "    )\n");
    idl_fileOutPrintf (idl_fileCur(), "{\n");
    idl_fileOutPrintf (idl_fileCur(), "    sd_serializer     serializer;\n");
    idl_fileOutPrintf (idl_fileCur(), "    sd_serializedData serData;\n");
    idl_fileOutPrintf (idl_fileCur(), "    c_metaObject      typeSpec;\n");
    idl_fileOutPrintf (idl_fileCur(), "\n");
    idl_fileOutPrintf (idl_fileCur(), "    static const char metaDescriptor[] = \"%s\";\n", metaDescriptor);
    idl_fileOutPrintf (idl_fileCur(), "\n");
    idl_fileOutPrintf (idl_fileCur(), "    serializer = sd_serializerXMLTypeinfoNew(base, TRUE);\n");
    idl_fileOutPrintf (idl_fileCur(), "    serData    = sd_serializerFromString(serializer, metaDescriptor);\n");
    idl_fileOutPrintf (idl_fileCur(), "    typeSpec   = c_metaObject(sd_serializerDeserializeValidated(serializer, serData));\n");
    idl_fileOutPrintf (idl_fileCur(), "    sd_serializedDataFree(serData);\n");
    idl_fileOutPrintf (idl_fileCur(), "    sd_serializerFree(serializer);\n");
    idl_fileOutPrintf (idl_fileCur(), "\n");
    idl_fileOutPrintf (idl_fileCur(), "    return typeSpec;\n");
    idl_fileOutPrintf (idl_fileCur(), "}\n");
}

static void
idl_registerHeaderFile (
    const char *basename
    )
{
    idl_fileOutPrintf (idl_fileCur(), "#ifndef %s\n", idl_macroFromBasename(basename,"__REGISTER_H"));
    idl_fileOutPrintf (idl_fileCur(), "#define %s\n", idl_macroFromBasename(basename,"__REGISTER_H"));
    idl_fileOutPrintf (idl_fileCur(), "\n");
    idl_fileOutPrintf (idl_fileCur(), "#include <c_base.h>\n");
    idl_fileOutPrintf (idl_fileCur(), "\n");
    idl_fileOutPrintf (idl_fileCur(), "void %s__register_types (c_base base);\n", basename);
    idl_fileOutPrintf (idl_fileCur(), "\n");
    idl_fileOutPrintf (idl_fileCur(), "#endif\n");
}

void
idl_registerType (
    c_base base,
    const char *basename,
    c_iter typeNames
    )
{
    sd_serializer metaSer;
    sd_serializedData serData;
    char *metaDescription = NULL;
    char *typeName;
    c_char *fname;
    c_metaObject type;
    int i;

    fname = os_malloc((size_t)((int)strlen(basename)+20));
    snprintf (fname, (size_t)((int)strlen(basename)+20), "%s_register.h", basename);
    idl_fileSetCur (idl_fileOutNew (fname, "w"));
    if (idl_fileCur () == NULL) {
        idl_reportOpenError (fname);
    }
    idl_registerHeaderFile (basename);
    idl_fileOutFree (idl_fileCur());


    snprintf (fname, (size_t)((int)strlen(basename)+20), "%s_register.c", basename);
    idl_fileSetCur (idl_fileOutNew (fname, "w"));
    if (idl_fileCur () == NULL) {
        idl_reportOpenError (fname);
    }
    idl_registerBodyHeader (basename);
    for (i = 0; i < c_iterLength(typeNames); i++) {
	typeName = c_iterObject(typeNames, i);
        type = c_metaResolve ((c_metaObject)base, (const char *)typeName);
        if (type) {
            metaSer = sd_serializerXMLTypeinfoNew (base, TRUE);
            if (metaSer) {
                serData = sd_serializerSerialize (metaSer, c_object(type));
                if (serData) {
                    metaDescription = sd_serializerToString (metaSer, serData);
                    if (metaDescription) {
                        idl_registerBody (typeName, metaDescription);
                        os_free (metaDescription);
                    }
                }
                sd_serializerFree (metaSer);
            }
        } else {
	    printf ("Specified type %s not found\n", typeName);
        }
    }
    idl_fileOutPrintf (idl_fileCur(), "\n");
    idl_fileOutPrintf (idl_fileCur(), "void\n");
    idl_fileOutPrintf (idl_fileCur(), "%s__register_types (c_base base)\n", basename);
    idl_fileOutPrintf (idl_fileCur(), "{\n");
    for (i = 0; i < c_iterLength(typeNames); i++) {
	typeName = c_iterObject(typeNames, i);
	idl_fileOutPrintf (idl_fileCur(), "    %s__register_type (base);\n", idl_cScopedTypeName(typeName));
    }
    idl_fileOutPrintf (idl_fileCur(), "}\n");
    idl_fileOutFree (idl_fileCur());
    os_free (fname);
}
