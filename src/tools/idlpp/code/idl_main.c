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
#include <string.h>

#include "os.h"
#include <errno.h>
#include "sd_serializerXMLTypeinfo.h"
#include "sd_serializerXMLMetadata.h"
#include <ctype.h>
#include "c_stringSupport.h"

#include "idl_parser.h"
#include "idl_walk.h"
#include "idl_dependencies.h"
#include "idl_genLanguageHelper.h"
#include "idl_genSpliceDep.h"
#include "idl_fileMap.h"
#include "idl_streamsDef.h"

/* Splice types related support */
#include "idl_genSpliceType.h"
#include "idl_genSpliceLoad.h"
#include "idl_genMetaHelper.h"

/* Corba C++ related support */
#include "idl_genCorbaCxxHelper.h"
#include "idl_genCorbaCxxCopyin.h"
#include "idl_genCorbaCxxCopyout.h"
#include "idl_genCorbaCxxCcpp.h"
#include "idl_genCxxTypedClassDefs.h"
#include "idl_genCxxTypedClassImpl.h"
#include "idl_genCxxMeta.h"
#include "idl_genCorbaCxxStreamsCcpp.h"
#include "idl_genCxxStreamsImpl.h"
#include "idl_genCxxStreamsDefs.h"
#include "idl_genCxxStreamsIdl.h"
#include "idl_genISOCxxHeader.h"

/* C related support */
#include "idl_genCorbaCHelper.h"
#include "idl_genSacType.h"
#include "idl_genCorbaCCopyin.h"
#include "idl_genCorbaCCopyout.h"
#include "idl_genSacTypedClassDefs.h"
#include "idl_genSacTypedClassImpl.h"
#include "idl_genSacObjectControl.h"
#include "idl_genCHelper.h"
#include "idl_genSacMeta.h"

/* C# related support */
#include "idl_genSACSType.h"
#include "idl_genSACSTypedClassDefs.h"
#include "idl_genSACSSplDcps.h"

/* Java related support */
#include "idl_genCorbaJavaHelper.h"
#include "idl_genJavaHelper.h"
#include "idl_genSajType.h"
#include "idl_genSajHolder.h"
#include "idl_genSajTypedClass.h"
#include "idl_genSajMeta.h"
#include "idl__genSajMeta.h"

/* IDL related support */
#include "idl_genIdl.h"
#include "idl_genIdlHelper.h"

/* register Type code */
#include "idl_registerType.h"

/* include preprocessor */
#include "dds_cpp.h"

/* include DLL related header file */
#include "idl_dll.h"

/* TypeSize related header file */
#include "idl_genTypeSize.h"

#define DDS_DCPS_DEF "dds_dcps.idl"
#define MAX_FILE_POSTFIX_LENGTH (os_size_t) (20) /* maximum length postfixed to base file name */
#define MAX_CPP_COMMAND (4192)

#ifdef WIN32
    const char* DEFAULT_ORB = "DDS_OpenFusion_1_6_1";
    const char* QUOTE = "\"";
#else
    const char* DEFAULT_ORB = "DDS_OpenFusion_1_6_1";
    const char* QUOTE = "";
#endif
    const char* IDLPP_CMD_OPTIONS = "P:d:o:b:m:t:c:hECSTD:I:l:j:n:iX:";

int runCppGen(
   char* outputDir,
   char* cpp_command,
   char* filename,
   c_bool cppgenIgnoreInterfaces);

static void
print_usage(
    char *name)
{
#ifdef EVAL_V
    printf("%s EVALUATION VERSION\n", name);
#endif
    printf("Usage: %s [-c preprocessor-path] [-b ORB-template-path]\n"
           "       [-n <include-suffix>] [-I path] [-D macro[=definition]] [-S | -C] \n"
           "       [-l (c | c++ | cpp | isocpp | isoc++ | cs | java)] [-j [old]:<new>] [-d directory] [-i] \n"
           "       [-P dll_macro_name[,<h-file>]] [-o (dds-types | custom-psm | no-equality)] <filename>\n", name);
}

static void
print_help(
    char *name)
{
    print_usage(name);
    printf(
        "    -b ORB-template-path\n"
        "       Specifies the ORB specific path within the template path.\n"
        "       Depending of the used ORB, the OpenSplice preprocessor needs\n"
        "       template files to generate the ORB specific interfaces. This\n"
        "       option selects the specific ORB template files for the preprocessor.\n");
    printf(
        "       The ORB specific template path can also be set via the environment\n"
        "       variable OSPL_ORB_PATH, the command line option is however leading.\n"
        "       To complete the path to the templates, the environment variable\n"
        "       OSPL_TMPL_PATH is prepended to the ORB path.\n");
    printf(
        "       For C, OSPL_ORB_PATH will by default be set to SAC.\n"
        "       For C++, OSPL_ORB_PATH will by default be set to CCPP%s%s.\n"
        "       For Java, OSPL_ORB_PATH will by default be set to SAJ.\n",
        os_fileSep(), DEFAULT_ORB);
    printf(
        "    -n <include-suffix>\n"
        "       Overrides the suffix that is used to identify the ORB dependent\n"
        "       header file (specifying the data model) that needs to be included.\n"
        "       Normally the name of this include file is derived from the IDL\n"
        "       file name and followed by an ORB dependent suffix (e.g. 'C.h'\n");
    printf(
        "       for ACE-TAO based ORBs).\n"
        "       This option is only supported in Corba cohabitation mode for C++.\n"
        "       In all other cases it is simply ignored.\n"
        "       Example usage: -e .stub.hpp\n");
    printf(
        "       (For a file named 'foo.idl' this will include 'foo.stub.hpp'\n"
        "       instead of 'fooC.h', which is the default expectation for ACE-TAO.)\n");
    printf(
        "    -I path\n"
        "    -Ipath\n"
        "       Passes the include path directives to the C preprocessor.\n");
    printf(
        "    -D macro[=definition]\n"
        "    -Dmacro[=definition]\n"
        "       Passes the macro definition to the C preprocessor.\n");
    printf(
        "    -S\n"
        "       Defines standalone mode, which allows application programs to\n"
        "       be build and run without involvement of an ORB. The namespace\n"
        "       for standard types will be DDS instead of CORBA in this case.\n"
        "       This is the default mode for all language bindings\n");
    printf(
        "    -C\n"
        "       Defines ORB bound mode, which allows application programs to be\n"
        "       build and run integrated with an ORB. This mode is the default mode\n"
        "       for the C++ language.\n");
    printf(
        "    -i\n"
        "       In standalone mode, also process interfaces in the supplied IDL\n"
        "       file. This removes the -ignore_interfaces parameter that is normally\n"
        "       passed to cppgen. Only applicable for the C++ language.\n");
    printf(
        "    -j [old]:<new>\n"
        "       Only applicable to JAVA. Specifies that the (partial) package\n"
        "       name which matches [old] is substituted for the package name\n"
        "       which matches <new>. If [old] is not included then the package\n"
        "       name defined by <new> is prefixed to all JAVA packages.\n");
    printf(
        "       The package names may only be separated by '.' characters.\n"
        "       A trailing '.' character is not required, but may be used.\n"
        "       Example usage: -j :org.opensplice (prefixes all java packages).\n"
        "       Example usage: -j com.opensplice.:org.opensplice. (substitutes).\n");
    printf(
        "    -P dll_macro_name[,<header-file>]\n"
        "       Only applicable to C and C++. Sets export macro that will be\n"
        "       prefixed to all functions in the generated code. This allows\n"
        "       creating DLL's from generated code and controlling symbol visibility.\n"
        "       Optionally a header file can be given that will be included\n"
        "       in each generated file.\n");
    printf(
        "    -d directory\n"
        "       Specify the location where to place the generated files.\n");
    printf(
        "    -o (dds-types | custom-psm | no-equality)\n"
        "       'dds-types' enables support for standard DDS-DCPS definitions.\n"
        "       The OpenSplice preprocessor provides definitions for constants\n"
        "       and types as defined in the OMG-DDS-DCPS PSM. This implies that\n"
        "       these definitions can be used within application IDL.\n");

    printf(
        "       'custom-psm' enables support for alternative IDL language mappings.\n"
        "       Currently C-Sharp offers an alternative language mapping where\n"
        "       IDL names are translated to their PascalCase representation and\n"
        "       where '@' instead of '_' is used to escape reserved C#-keywords.\n");

    printf(
        "       'no-equality' Disables support for the automatically generated \n"
        "       equality operator on ISOC++ types.\n");

    printf(
        "    -l (c | c++ | cpp | isocpp | isoc++ | cs | java)\n"
        "       Defines the target language. Value 'cs' represents C-sharp\n"
        "       and 'cpp' is an alias for 'c++'.\n"
        "       The value 'isoc++' (or isocpp) means that the DCPS API for entities\n");
    printf(
        "       follows the ISO/IEC C++ 2003 Language DDS PSM instead of the\n"
        "       IDL-to-C++ platform specific mapping (language 'c++').\n"
        "       Note that the OpenSplice preprocessor does not support every\n"
        "       combination of mode and language. The default setting of the\n"
        "       OpenSplice preprocessor is c++.\n");
    printf(
        "    <filename>\n"
        "       Specifies the IDL input file to process.\n");
    printf("\n"
        "    Supported languages, ORBs and modes:\n"
        "       Lang    ORB    mode   library        OSPL_ORB_PATH\n"
        "       ---------------------------------------------------\n"
        "       c       N.A.   S      dcpssac        SAC\n");
    printf(
        "       c++     N.A.   C      dcpsccpp       CCPP%s%s\n"
        "       cpp     N.A.   C      dcpsccpp       CCPP%s%s\n"
        "       c++     N.A.   S      dcpssacpp      SACPP\n"
        "       cpp     N.A.   S      dcpssacpp      SACPP\n"
        "       isocpp  N.A.   S      dcpsisocpp     SACPP\n"
        "       isoc++  N.A.   S      dcpsisocpp     SACPP\n"
        "       cs      N.A.   S      dcpssacs       SACS\n"
        "       Java    JacOrb C      dcpscj         SAJ\n",
        os_fileSep(), DEFAULT_ORB, os_fileSep(), DEFAULT_ORB);
}

static char *
splitArg(
    char *arg,
    int segment)
{
    char *start;
    char *end;
    char *string_segment;
    int i;

    start = arg;
    while (segment) {
        if ((*start == '=') || (*start == ',') || (*start == '\0')) {
            segment--;
        }
        if (*start != '\0') {
            start++;
        }
    }
    if (*start == '\0') {
        return NULL;
    }
    end = start;
    while ((*end != '=') && (*end != ',') && (*end != '\0')) {
        end++;
    }
    string_segment = os_malloc ((size_t)(end - start + 1));
    memset(string_segment, 0, (size_t)(end - start + 1));
    for (i = 0; i < (end - start); i++) {
        string_segment[i] = start[i];
    }
    return string_segment;
}

void
idl_reportOpenError(
    char *fname)
{
    printf ("Error opening file %s for writing. Reason: %s (%d)\n", fname, strerror( errno ), errno);
    exit (-1);
}

static void
reportErrorAndExit(
    const char* error)
{
    idl_dllExit();
    printf("Error: %s\n", error);
    exit(-1);
}

static void
preLoadSpliceDdsDefinitions(
    void)
{
    c_base base;
    const char *templ_path;
    char fname[1024];
    templ_path = os_getenv ("OSPL_TMPL_PATH");
    if (templ_path == NULL) {
        printf ("Variable OSPL_TMPL_PATH not defined\n");
        exit (1);
    }
    /* preload DDS definitions. Hopefully this loads #includes too.
     * @todo Sort this out
     */
    snprintf(fname, sizeof(fname), "%s%c..%cidl%c%s", templ_path, OS_FILESEPCHAR,
                                                                   OS_FILESEPCHAR,
                                                                   OS_FILESEPCHAR,
                                                                   DDS_DCPS_DEF);
    base = idl_parseFile(fname, 0);
    OS_UNUSED_ARG(base);

    /** @todo Free existing file relations                 */
    /* With idl_fileMapDefGet retrieve current relations   */
    /* which can be freed then before setting to NULL      */

    /* Forget file relations so that no include directives */
    /* will be generated when DDS-DCPS types are referred   */
    idl_fileMapDefSet(NULL);
}

void
idl_exit(
    int exitCode)
{
    idl_dllExit();
    exit(exitCode);
}

static int
addDefine(
    const char *def)
{
    char *vPtr;
    char *value = NULL;
    int result;

    result = 0;
    vPtr = strchr(def, '=');
    if (vPtr) {
        if (vPtr != def) {
            *vPtr = '\0';
            vPtr++;
            value = os_strdup(vPtr);
        } else {
            printf("Error: must give name for -D%s\n", def);
            result = -1;
        }
    } else {
        value = os_strdup("1");
    }
    if (value) {
        define(def, -1, (unsigned char *)value, DEF_CMDLINE);
    }
    return result;
}


static char*
getDefaultCcppOrbPath()
{
    char* ccppOrbPath;
    const char* api = "OSPL_ORB_PATH=CCPP";
    const char* filesep = os_fileSep();

    char* splice_orb;

     /* The env setup scripts define SPLICE_ORB to indicate the CPP ORB - this
    is more appropriate than a hardcode default. */
    if ((splice_orb = os_getenv ("SPLICE_ORB")) == NULL)
    {
        /* Not defined - fall back on the hard code default. */
        ccppOrbPath = (char*)(os_malloc(strlen(api) + strlen(filesep) + strlen(DEFAULT_ORB) + 1));
        os_sprintf(ccppOrbPath, "%s%s%s", api, filesep, DEFAULT_ORB);
    }
    else
    {
        /* Defined - use as default */
        ccppOrbPath = (char*)(os_malloc(strlen(api) + strlen(filesep) + strlen(splice_orb) + 1));
        os_sprintf(ccppOrbPath, "%s%s%s", api, filesep, splice_orb);
    }

    return ccppOrbPath;
}

static void
defaultPaths(const char* arg_0)
{
#ifdef _WIN32
    int size = 0;
#endif
    char* exe_path;
    char* dir_end;
    char* possible_home;

    char* templ_path;
    char* ospl_home;

    possible_home = NULL;
    ospl_home = os_getenv("OSPL_HOME");
    templ_path = os_getenv("OSPL_TMPL_PATH");

    if (ospl_home != NULL && templ_path != NULL)
    {
        /* Nothing to do */
    }
    else if (ospl_home != NULL)
    {
        possible_home = (char*)os_malloc(strlen (ospl_home)
                                        + sizeof ("etcidlpp")
                                        + 3); /* 2*file sep + 1*nul */
        os_sprintf(possible_home, "%s%c%s%c%s", ospl_home, OS_FILESEPCHAR, "etc", OS_FILESEPCHAR, "idlpp" );
        os_setenv("OSPL_TMPL_PATH", possible_home);
    }
    else if (templ_path == NULL)
    {
#ifdef _WIN32
        exe_path = (char*) os_malloc(512);
        size = GetModuleFileNameA (NULL, exe_path, 512);
        if (! size > 0)
        {
            os_free(exe_path);
            exe_path = NULL;
        }
#else
        exe_path = os_locate(arg_0, OS_ROK|OS_XOK);
#endif
        if (exe_path != NULL)
        {
            dir_end = strrchr(exe_path, OS_FILESEPCHAR);
            if (dir_end != NULL)
            {
                *dir_end = '\0';
            }
            else
            {
                os_strcpy(exe_path, ".");
            }
            possible_home = (char*)os_malloc(strlen (exe_path)
                                            + sizeof ("....etcidlpp")
                                            + 5); /* 4 * file sep + 1 * nul */
            if (possible_home != NULL)
            {
                os_sprintf(possible_home, "%s%c%s%c%s%c%s", exe_path, OS_FILESEPCHAR, "..", OS_FILESEPCHAR, "etc", OS_FILESEPCHAR, "idlpp" );
                if (os_access(possible_home, OS_ROK) == os_resultSuccess)
                {
                    os_setenv("OSPL_TMPL_PATH", possible_home);
                    os_sprintf(possible_home, "%s%c%s", exe_path, OS_FILESEPCHAR, "..");
                    os_setenv("OSPL_HOME", possible_home);
                }
                else
                {
                    os_sprintf(possible_home, "%s%c%s%c%s%c%s%c%s", exe_path, OS_FILESEPCHAR, "..", OS_FILESEPCHAR, "..", OS_FILESEPCHAR, "etc", OS_FILESEPCHAR, "idlpp" );
                    if (os_access(possible_home, OS_ROK) == os_resultSuccess)
                    {
                        os_setenv("OSPL_TMPL_PATH", possible_home);
                        os_sprintf(possible_home, "%s%c%s%c%s", exe_path, OS_FILESEPCHAR, "..", OS_FILESEPCHAR, "..");
                        os_setenv("OSPL_HOME", possible_home);
                    }
                }
            }
            os_free(exe_path);
        }
    }
    os_free(possible_home);
}

OPENSPLICE_MAIN (ospl_idlpp)
{
    c_base base;
    char *fpathNorm;
    char *ptr;
    char *extension;
    char *filename;
    char *basename;
    os_char* clientHeader = NULL;
    char *databaseName = NULL;
    char *moduleName;
    char *cpp_arg = NULL;
    char *typeName = NULL;
    char orb[256];
    char* outputDir = NULL;
    char* tmpArg = NULL;
    char* includeIdlDir = NULL;
    c_bool makeMetrics = FALSE;
    c_bool outputDirSetSuccess = FALSE;
    c_bool traceWalk = FALSE;
    c_bool traceInput = FALSE;
    c_bool tracePreProcessed = FALSE;
    c_bool makeSpliceType = FALSE;
    c_bool makeSpliceLoad = FALSE;
    c_bool makeSpliceCopy = FALSE;
    c_bool makeSpliceHelper = FALSE;
    c_bool makeCorbaType = FALSE;
    c_bool makeDeallocCode = FALSE;
    c_bool makeMetaData = FALSE;
    c_bool makeTypeInfo = FALSE;
    c_bool makeRegisterType = FALSE;
    c_bool makeAll = TRUE;
    c_bool dcpsTypes = FALSE;
    c_bool customPSM = FALSE;
    c_bool attachDatabase = FALSE;
    c_bool cpp_ignoreInterfaces = TRUE;
    c_bool justCopyIn = FALSE;
    /* If strlen(suffix) > 0; then static type-caching is disabled for the
     * copyIn<suffix> functions. */
    char *suffix = os_strdup("");
    os_sharedAttr sharedAttr;
    os_sharedHandle sHandle = NULL;
    os_result result;
    int opt;
    int i;
    os_size_t len;
    c_iter includeDefinitions = NULL;
    c_iter macroDefinitions = NULL;
    c_iter typeNames = NULL;
    os_result osr;
    char* ccppOrbPath;
    char* dcpsIdlFileName;
    int returnCode = 0;
    const char *templ_path;
    char fnameA[1024];
    char* sub;
    char* tmpPtr;
    c_iter orgPackageList = NULL;
    c_iter tarPackageList = NULL;
 /*    struct stat stFileInfo; */
/*     int intStat; */

    /* Use a unique name, so pass NULL as parameter */
    osr = os_serviceStart(NULL);
    if (osr != os_resultSuccess) {
        printf("Failed to initialize.\n");
        exit(2);
    }
    os_osInit();

    init_preprocess();

    idl_dllInitialize(); /* initialise */

    defaultPaths(argv[0]);

    ccppOrbPath = getDefaultCcppOrbPath();
    while ((opt = getopt(argc, argv, IDLPP_CMD_OPTIONS)) != -1) {
        switch (opt) {
        case 'h':
            print_help(argv[0]);
            idl_exit(0);
        break;
        case 'n':
            if(!clientHeader)
            {
                clientHeader = os_strdup(optarg);
            } else
            {
                printf("Option '-n' can only be used once.\n");
                print_usage(argv[0]);
                idl_exit(-1);
            }

        break;
        case 'i':
            cpp_ignoreInterfaces = FALSE;
            break;
        case 'l':

            if (strcmp(optarg, "c") == 0) {
                idl_setLanguage(IDL_LANG_C);
                if (os_getenv("OSPL_ORB_PATH") == NULL) {
                    os_putenv("OSPL_ORB_PATH=SAC");
                }
            } else if (strcmp(optarg, "c++") == 0) {
                idl_setLanguage(IDL_LANG_CXX);
                if (os_getenv("OSPL_ORB_PATH") == NULL) {
                    os_putenv(ccppOrbPath);
                    /* os_putenv ("OSPL_ORB_PATH=TAO"); */
                }
            } else if (strcmp(optarg, "cpp") == 0) {
                idl_setLanguage(IDL_LANG_CXX);
                if (os_getenv("OSPL_ORB_PATH") == NULL) {
                    os_putenv(ccppOrbPath);
                }
            } else if (strcmp(optarg, "isoc++") == 0) {
                idl_setLanguage(IDL_LANG_CXX);
                idl_setIsISOCpp(OS_TRUE);
                idl_setIsISOCppTypes(OS_TRUE);
                idl_setIsGenEquality(OS_TRUE);
                if (os_getenv("OSPL_ORB_PATH") == NULL) {
                    os_putenv(ccppOrbPath);
                    /* os_putenv ("OSPL_ORB_PATH=TAO"); */
                }
            } else if (strcmp(optarg, "isocpp") == 0) {
                idl_setLanguage(IDL_LANG_CXX);
                idl_setIsISOCpp(OS_TRUE);
                idl_setIsISOCppTypes(OS_TRUE);
                idl_setIsGenEquality(OS_TRUE);
                if (os_getenv("OSPL_ORB_PATH") == NULL) {
                    os_putenv(ccppOrbPath);
                }
            } else if (strcmp(optarg, "cs") == 0) {
                idl_setLanguage(IDL_LANG_CS);
                if (os_getenv("OSPL_ORB_PATH") == NULL) {
                    os_putenv("OSPL_ORB_PATH=SACS");
                }
            } else if (strcmp(optarg, "java") == 0) {
                idl_setLanguage(IDL_LANG_JAVA);
                if (os_getenv("OSPL_ORB_PATH") == NULL) {
                    os_putenv("OSPL_ORB_PATH=SAJ");
                }
            } else {
                print_usage(argv[0]);
                idl_exit(-1);
            }

        break;
        case 'j':
            {
                /* <old>:<new> */
                sub = optarg;
                tmpPtr = strchr(sub, ':');
                if(tmpPtr)
                {
                    *tmpPtr = '\0';
                    if(strlen(sub) > 0)
                    {
                        orgPackageList = c_iterAppend(orgPackageList, os_strdup(sub));
                    }else {
                        orgPackageList = c_iterAppend(orgPackageList, os_strdup(""));
                    }
                    tmpPtr++;
                    tarPackageList = c_iterAppend(tarPackageList, os_strdup(tmpPtr));
                } else
                {
                    printf("Option '-j' is used incorrectly, argument %s does not match required pattern.\n", sub);
                    print_usage(argv[0]);
                    idl_exit(-1);
                }
            }
            break;
        case 'd':
            if (outputDir) {
                printf("Option '-d' can only be set once.\n");
                print_usage(argv[0]);
                idl_exit(-1);
            }
            outputDir = os_fileNormalize(optarg);
        break;
        case 'C':
            if (idl_getCorbaMode() == IDL_MODE_STANDALONE) {
                printf("ERROR --> Mode already set to: %s\n", idl_getCorbaModeStr());
                print_usage(argv[0]);
                idl_exit(-1);
            }
            idl_setCorbaMode(IDL_MODE_ORB_BOUND);
            idl_setIsISOCppTypes(OS_FALSE); /** @internal @bug OSPL-3369 */
        break;
        case 'P':
            if (idl_dllSetOption(optarg)) {
                print_usage(argv[0]);
                idl_exit(-1);
            }
        break;
        case 'S':
            if (idl_getCorbaMode() == IDL_MODE_ORB_BOUND) {
                printf("ERROR --> Mode already set to: %s\n", idl_getCorbaModeStr());
                print_usage(argv[0]);
                idl_exit(-1);
            }
            idl_setCorbaMode(IDL_MODE_STANDALONE);
        break;
        case 'T':
            if(idl_getIsISOCppTypes())
            {
                idl_setIsISOCppTestMethods(OS_TRUE);
            }
            else
            {
                printf("Option '-T' is to be used in conjunction with '-l isocpp' only\n");
                print_usage(argv[0]);
                idl_exit(-1);
            }
        break;
        case 't':
            /* check for trace options */
            if (strcmp(optarg, "walk") == 0) {
                /* Traces walk through the metadata */
                traceWalk = TRUE;
            } else if (strcmp(optarg, "input") == 0) {
                /* Traces the IDL parser */
                traceInput = TRUE;
            }
        break;
        case 'm':
            /* check for design based test options */
            if (strcmp(optarg, "SPLTYPE") == 0) {
                /* Generate Splice Type definitions */
                makeSpliceType = TRUE;
                makeAll = FALSE;
            } else if (strcmp(optarg, "SPLLOAD") == 0) {
                /* Generate Splice Metadata Load code */
                makeSpliceLoad = TRUE;
                makeAll = FALSE;
            } else if (strcmp(optarg, "SPLCOPY") == 0) {
                /* Generate Specialized Copy Routines code */
                makeSpliceCopy = TRUE;
                makeAll = FALSE;
            } else if (strcmp(optarg, "SPLHELP") == 0) {
                /* Generate Helper Functions code */
                makeSpliceHelper = TRUE;
                makeAll = FALSE;
            } else if (strcmp(optarg, "CORBATYPE") == 0) {
                /* Generate CORBA support code */
                makeCorbaType = TRUE;
                makeAll = FALSE;
            } else if (strcmp(optarg, "OBJCTRL") == 0) {
                /* Generate deallocation code */
                makeDeallocCode = TRUE;
                makeAll = FALSE;
            } else if (strncmp(optarg, "METADATA=", 9) == 0) {
                /* Generate MetaData description */
                makeMetaData = TRUE;
                makeAll = FALSE;
                typeName = splitArg(optarg, 1);
                if (typeName == NULL) {
                    printf("typeName not specified for option -m METADATA=\n");
                    idl_exit(-1);
                }
            } else if (strncmp(optarg, "TYPEINFO=", 9) == 0) {
                /* Generate TypeInfo description */
                makeTypeInfo = TRUE;
                makeAll = FALSE;
                typeName = splitArg(optarg, 1);
                if (typeName == NULL) {
                    printf("typeName not specified for option -m TYPEINFO=\n");
                    idl_exit(-1);
                }
            } else if (strncmp(optarg, "ATTACH=", 7) == 0) {
                /* Attach to an existing meta database, not parsing any input file */
                /* The database name is the first argument */
                databaseName = splitArg(optarg, 1);
                i = 2;
                moduleName = splitArg(optarg, i);
                while (moduleName != NULL) {
                    /* The next arguments specify the modules that have
                       to be processed, they are explicitely specified
                       because the metadata database does not contain a
                       mapping to files as the preprocessor generally keeps
                    */
                    idl_walkPresetModule(moduleName);
                    i++;
                    moduleName = splitArg(optarg, i);
                }
                attachDatabase = TRUE;
            } else if (strncmp(optarg, "TYPES=", 6) == 0) {
                os_putenv("OSPL_ORB_PATH=BIT");
                makeAll = FALSE;
                makeRegisterType = TRUE;
                /* select types to generate registerType code for */
                typeNames = c_iterNew(NULL);
                i = 1;
                while ((typeName = splitArg(optarg, i)) != NULL) {
                    typeNames = c_iterAppend(typeNames, typeName);
                    i++;
                }
            }
        break;
        case 'c':
            /* specifies the C preprocessor to use for preprocessing the IDL
               file. This option does not overrule the environment variable
               CPP which can also be used to specify the C preprocessor.
            */
            cpp_arg = optarg;
        break;
        case 'b':
            /* specifies the ORB specific path within the template directory,
               e.g. TAO for a TAO specific template. This path can also be
               specified by the environment variable OSPL_ORB_PATH. This
               option overrules the environment variable. The complete applied
               template path = $OSPL_TMPL_PATH/$OSPL_ORB_PATH.
            */
            snprintf(orb, sizeof(orb), "OSPL_ORB_PATH=%s", optarg);
            os_putenv(orb);
        break;
        case 'D':
            /* Define a Macro, which is used while preprocessing the IDL
               input file. The macro definition is stored via an iterator.
            */
            if (addDefine(optarg)) {
                print_usage(argv[0]);
                idl_exit(-1);
            }
            macroDefinitions = c_iterAppend(macroDefinitions, optarg);
        break;
        case 'E':
            /* Trace the preprocessing process */
            tracePreProcessed = TRUE;
        break;
        case 'I':
            /* Define an include PATH, which is used while preprocessing the IDL
               input file. The include path is stored via an iterator.
            */
            /* also append it to the preprocessor include path */

            /* remove ending '\' that VS adds to the path see dds3125*/
            tmpArg = os_strdup(optarg);
            if (tmpArg[strlen(tmpArg)-1] == '\\') {
                tmpArg[strlen(tmpArg)-1] = '\0';
            }
            Ifile(tmpArg);
            includeDefinitions = c_iterAppend(includeDefinitions, tmpArg);
        break;
        case 'o':
            if (strcmp(optarg, "dds-types") == 0) {
                dcpsTypes = TRUE;
            } else if (strcmp(optarg, "custom-psm") == 0) {
                customPSM = TRUE;
            } else if (strcmp(optarg, "metrics") == 0) {
                makeMetrics = TRUE;
            } else if (strcmp(optarg, "no-equality") == 0) {
                idl_setIsGenEquality(OS_FALSE);
            } else {
                print_usage(argv[0]);
                idl_exit(-1);
            }
        break;
        case 'X': /* Undocumented; emits copyIn functions with a suffix without caching */
            justCopyIn = TRUE;
            os_free(suffix);
            suffix = os_strdup(optarg);
        break;
        case '?':
            print_usage(argv[0]);
            idl_exit(-1);
        break;
        }
    }
    /* Set standalone (-S) mode as default for all bindings */
    if(idl_getCorbaMode() == IDL_MODE_UNKNOWN)
        idl_setCorbaMode(IDL_MODE_STANDALONE);

    if ((optind >= argc) || ((argc-optind) > 1)) {
        print_usage(argv[0]);
        idl_exit(-1);
    }

    /* check if mode has been set */
    if (!makeMetrics) {
        if (idl_getCorbaMode() == IDL_MODE_UNKNOWN) {
            printf("ERROR --> Corba mode has not been set!\n");
            print_usage(argv[0]);
            idl_exit(-1);
        }
        /* check if languague has been set */
        if (idl_getLanguage() == IDL_LANG_UNKNOWN) {
            printf("ERROR --> Language has not been set!\n");
            print_usage(argv[0]);
            idl_exit(-1);
        }
        /* check whether language and mode are supported */
        if (!idl_languageAndModeSupported()) {
            printf("ERROR --> Combination of language and generation mode not supported: LANG=%s MODE=%s\n",
                idl_getLanguageStr(), idl_getCorbaModeStr());
            print_usage(argv[0]);
            idl_exit(-1);
        }
    }
    if (outputDir) {
        outputDirSetSuccess = idl_dirOutNew(outputDir);

        if(!outputDirSetSuccess){
            idl_exit(-1);
        }
    }

#ifdef EVAL_V
    printf("%s EVALUATION VERSION\n", argv[0]);
#endif

    idl_genCorbaCxxCcpp_setClientHeader(clientHeader);
    idl_genCorbaCxxStreamsCcpp_setClientHeader(clientHeader);

    fpathNorm = os_fileNormalize(argv[optind]);
    /* Determine the start of the IDL input filename, removing all path related stuff.
       The information is used to determine the basename of the input IDL file. The
       basename is the filename of the input file without path and extension.
       Also take into account non-alphanum characters and replace with '_'
    */
    includeIdlDir = os_strdup(fpathNorm);
    ptr = os_rindex(fpathNorm, OS_FILESEPCHAR);
    if (ptr == NULL) {
        ptr = fpathNorm;
        if (includeIdlDir) {
            os_free(includeIdlDir);
        }
        includeIdlDir = os_strdup(".");
    } else {
        ptr++;
        if (includeIdlDir) {
            /* Determine the directory the idl is located in and add it to the include path (dds3125)
             * this is done by taking the length of the full path including the idl file name
             * and subtract that with the length of the idl file name
             * the -1 is for the ending \ or / in the path.
             */
            includeIdlDir[(strlen(fpathNorm)-strlen(ptr))-1] = '\0';
        }
    }
    if (includeIdlDir) {
        Ifile(includeIdlDir);
        includeDefinitions = c_iterAppend(includeDefinitions, includeIdlDir);
    }

    extension = os_rindex(ptr, '.');
    if (extension) {
        *extension = '\0';
        len = ((os_size_t)extension - (os_size_t)ptr);
    } else {
        len = strlen(ptr);
    }
    basename = os_malloc(len + 1);
    os_strncpy(basename, (const char *)ptr, len);
    basename[len] = '\0';

    /* reset extension, since it used again */
    if (extension) {
        *extension = '.';
    }
    /* replace all non alphanum characters with '_' */
    ptr = basename;
    while (*ptr != '\0') {
        if (!isalnum(*ptr)) {
            *ptr = '_';
        }
        ptr++;
    }

    /* Determine the input file with path excluding any preceding "./" path
       elements. The C preprocessor also removes this information from the
       file information tags in the processed input file. This information
       is used to determine if the specified meta objects relate to the
       input file.
    */
    i = 0;
    while ((fpathNorm[i] == '.') && (fpathNorm[i+1] == OS_FILESEPCHAR)) {
        i += 2;
    }
    filename = os_strdup(&fpathNorm[i]);
    os_free(fpathNorm);

    if (dcpsTypes) {
        if ((idl_getLanguage() == IDL_LANG_CXX) && (idl_getCorbaMode() == IDL_MODE_STANDALONE)) {
            char fname[1024];
            /* add include path OSPL_HOME/etc/idl as it contains the file dds_dcps.idl needed by
             * cppgen compiler.
             */
            if (os_getenv("OSPL_HOME") == NULL) {
                printf ("Variable OSPL_HOME not defined\n");
                idl_exit(1);
            }
            snprintf(fname, 1024, "%s/etc/idl",os_getenv("OSPL_HOME"));
            includeDefinitions = c_iterAppend(includeDefinitions,os_strdup(fname));
        }
        preLoadSpliceDdsDefinitions();
        /* Reset the preprocessor to its initial state */
        init_preprocess();
        /* add the macro definitions to the preprocessor engine */
        for (i = 0; i < c_iterLength(macroDefinitions); i++) {
            addDefine(c_iterObject(macroDefinitions, i));
        }
        for (i = 0; i < c_iterLength(includeDefinitions); i++) {
            Ifile(c_iterObject(includeDefinitions, i));
        }
    }

    if (!attachDatabase) {
        addDefine("OSPL_IDL_COMPILER");
        /* Parse the input file, the database is returned */
        base = idl_parseFile(filename, traceInput);

        /* Validate that all types are finalized */
        if(!idl_fileMapCheckFinalized(idl_fileMapDefGet(), filename)) {
            /* Error reporting is responsibility of CheckFinalized. */
            exit(-1);
        }
    } else {
        /* Attach to an existing database */
        os_sharedAttrInit(&sharedAttr);
        sHandle = os_sharedCreateHandle(databaseName, &sharedAttr, 0);
        result = os_sharedMemoryAttach(sHandle);
        if (result != os_resultSuccess) {
            printf("Attach shared memory failed\n");
            idl_exit(-1);
        }
        /* Specify the fictive file to process */
        idl_walkPresetFile(filename);
        /* Open the database */
        base = c_open(databaseName,os_sharedAddress(sHandle));
    }

    if (base) {
        /* If the database is available */

        c_iter source;
        char *fname;
        /* Create source for filename */
        source = idl_fileMapGetObjects(idl_fileMapDefGet(), filename);

        /* Allocate space for the output file name */
        fname = os_malloc(strlen(basename) + strlen(suffix) + MAX_FILE_POSTFIX_LENGTH);

        /* Initialize class to determine depedencies with other files */
        idl_depDefInit();
        /* Walk through metadata to determine depedencies with other files,
           the dependencies are used to generate include statements
        */
        {
            idl_program ip = idl_genSpliceDepProgram();
            ip->userData = suffix;
            idl_walk(base, filename, source, traceWalk, ip);
        }

        if (makeAll) {
            /* Do normal generation */

           if (idl_getLanguage() == IDL_LANG_CXX) {
                os_char* tmp;

                if (idl_getCorbaMode() == IDL_MODE_STANDALONE) {
                    os_putenv("OSPL_ORB_PATH=SACPP");
                } else if (os_getenv("OSPL_ORB_PATH") == NULL) {
                    os_putenv(ccppOrbPath);
                }

                if(!justCopyIn){
                    /* Generate the file that defines all DCPS specialized classes for the
                       user types
                    */
                    snprintf(fname,
                        strlen(basename) + MAX_FILE_POSTFIX_LENGTH,
                        "%sDcps_impl.h", basename);

                    idl_fileSetCur(idl_fileOutNew(fname, "w"));
                    if (idl_fileCur() == NULL) {
                        idl_reportOpenError(fname);
                    }

                    idl_walk(base, filename, source, traceWalk, idl_genCxxTypedClassDefsProgram());
                    idl_fileOutFree(idl_fileCur());

                    /* Generate the file that implements all DCPS specialized classes for the
                       user types
                    */
                    snprintf(fname,
                        strlen(basename) + MAX_FILE_POSTFIX_LENGTH,
                        "%sDcps_impl.cpp", basename);
                    idl_fileSetCur(idl_fileOutNew(fname, "w"));
                    if (idl_fileCur() == NULL) {
                        idl_reportOpenError(fname);
                    }
                    idl_walk(base, filename, source, traceWalk, idl_genCxxTypedClassImplProgram());
                    idl_walk(base, filename, source, traceWalk, idl_genCxxMetaProgram());
                    idl_fileOutFree(idl_fileCur());

                    /* Generate the file that specifies all Splice specialized data types for the
                       user types
                    */
                    snprintf(fname,
                        strlen(basename) + MAX_FILE_POSTFIX_LENGTH,
                        "%sSplDcps.h", basename);
                    idl_fileSetCur(idl_fileOutNew (fname, "w"));
                    if (idl_fileCur() == NULL) {
                        idl_reportOpenError(fname);
                    }
                    if (dcpsTypes) {
                        idl_fileOutPrintf(idl_fileCur(), "#include \"ccpp_ddsDcpsSplDcps.h\"\n\n");
                    }
                    if (idl_getIsISOCpp())
                    {
                        tmp = os_malloc(strlen(basename)+strlen("_DCPS.hpp")+(size_t)1);
                        snprintf
                            (tmp,
                            strlen(basename)+strlen("_DCPS.hpp")+(size_t)1,
                            "%s_DCPS.hpp", basename);
                    }
                    else
                    {
                        tmp = os_malloc(strlen(basename)+strlen("ccpp_")+(size_t)1);

                        snprintf
                            (tmp,
                            strlen(basename)+strlen("ccpp_")+(size_t)1,
                            "ccpp_%s", basename);
                    }

                    idl_genSpliceTypeSetIncludeFileName(tmp);
                    os_free(tmp);

                    idl_walk(base, filename, source, traceWalk, idl_genSpliceTypeProgram());
                    idl_fileOutFree(idl_fileCur());
                }


                /* Generate the file that implements all metadata load functions, Splice helper
                   functions as well as C++/CORBA copy routines to/from Splice for the user types
                */
                snprintf(fname,
                    strlen(basename) +strlen(suffix) + MAX_FILE_POSTFIX_LENGTH,
                    "%s%sSplDcps.cpp", basename, suffix);
                idl_fileSetCur(idl_fileOutNew(fname, "w"));
                if (idl_fileCur() == NULL) {
                    idl_reportOpenError(fname);
                }
                idl_fileOutPrintf(idl_fileCur(), "#include \"%sSplDcps.h\"\n", basename);
                if (idl_getIsISOCpp())
                {
                    idl_fileOutPrintf(idl_fileCur(), "#include \"%s_DCPS.hpp\"\n", basename);
                }
                else
                {
                    idl_fileOutPrintf(idl_fileCur(), "#include \"ccpp_%s.h\"\n", basename);
                    idl_fileOutPrintf(idl_fileCur(), "#include \"dds_type_aliases.h\"\n");
                }
                idl_fileOutPrintf(idl_fileCur(), "\n");

                if(!justCopyIn){
                    idl_walk(base, filename, source, traceWalk, idl_genCorbaCxxHelperProgram());
                }
                {
                    idl_program ci = idl_genCorbaCxxCopyinProgram();
                    /* If the suffix is empty, static type-caching is enabled.
                     * Disabled otherwise. */
                    ci->userData = suffix;
                    idl_walk(base, filename, source, traceWalk, ci);
                }
                if(!justCopyIn){
                    idl_walk(base, filename, source, traceWalk, idl_genCorbaCxxCopyoutProgram());
                }
                idl_fileOutFree(idl_fileCur());

                if(!justCopyIn){
                    /* Generate the file that includes ORB specific generated header files
                    */
                    if (idl_getIsISOCpp())
                    {
                        snprintf(fname,
                                strlen(basename) + MAX_FILE_POSTFIX_LENGTH,
                                "%s_DCPS.hpp", basename);
                    }
                    else
                    {
                        snprintf(fname,
                                strlen(basename) + MAX_FILE_POSTFIX_LENGTH,
                                "ccpp_%s.h", basename);
                    }
                    idl_fileSetCur(idl_fileOutNew(fname, "w"));
                    if (idl_fileCur() == NULL) {
                        idl_reportOpenError(fname);
                    }

                    if (idl_getIsISOCpp())
                    {
                        idl_walk(base, filename, source, traceWalk, idl_genISOCxxHeaderProgram());
                    }
                    else
                    {
                        idl_walk(base, filename, source, traceWalk, idl_genCorbaCxxCcppProgram());
                    }

                    idl_fileOutFree(idl_fileCur());

                    /* Expand IDL based application TypeSupport, DataReader and DataWriter interfaces
                     */

                    /* Create a list of keys and streams existing only in this idl file */
                    idl_walk(base, filename, source, traceWalk, idl_genIdlHelperProgram());

                    snprintf(fname,
                             strlen(basename) + MAX_FILE_POSTFIX_LENGTH,
                             "%sDcps.idl", basename);
                    idl_fileSetCur(idl_fileOutNew(fname, "w"));
                    if (idl_fileCur() == NULL) {
                       idl_reportOpenError(fname);
                    }
                    idl_walk(base, filename, source, traceWalk, idl_genIdlProgram());
                    idl_fileOutFree(idl_fileCur());
                    if (outputDir) {
                       dcpsIdlFileName = os_malloc(strlen(idl_dirOutCur()) + strlen(os_fileSep()) + strlen(fname) + 1);
                       os_sprintf(dcpsIdlFileName, "%s%s%s", idl_dirOutCur(), os_fileSep(), fname);
                    } else {
                       dcpsIdlFileName = os_strdup(fname);
                    }

                    /* Generate the Streams typed API if a #pragma streams is defined in the current file*/
                    if (os_iterLength(idl_idlScopeStreamsList) > 0) {
                        /* Generate the main include file which includes ORB specific generated header files */
                        snprintf(fname,
                            strlen(basename) + MAX_FILE_POSTFIX_LENGTH,
                            "%sStreamsApi.h", basename);
                        idl_fileSetCur(idl_fileOutNew(fname, "w"));
                        if (idl_fileCur() == NULL) {
                            idl_reportOpenError(fname);
                        }
                        idl_walk(base, filename, source, traceWalk, idl_genCorbaCxxStreamsCcppProgram());
                        idl_fileOutFree(idl_fileCur());

                        /* Generate the Stream API classes */
                        snprintf(fname,
                                strlen(basename) + MAX_FILE_POSTFIX_LENGTH,
                                "%sStreams_impl.h", basename);
                        idl_fileSetCur(idl_fileOutNew(fname, "w"));
                        if (idl_fileCur() == NULL) {
                            idl_reportOpenError(fname);
                        }
                        idl_walk(base, filename, source, traceWalk, idl_genCxxStreamsDefsProgram());
                        idl_fileOutFree(idl_fileCur());

                        snprintf(fname,
                                strlen(basename) + MAX_FILE_POSTFIX_LENGTH,
                                "%sStreams_impl.cpp", basename);
                        idl_fileSetCur(idl_fileOutNew(fname, "w"));
                        if (idl_fileCur() == NULL) {
                            idl_reportOpenError(fname);
                        }
                        idl_walk(base, filename, source, traceWalk, idl_genCxxStreamsImplProgram());
                        idl_fileOutFree(idl_fileCur());

                        /* Generate the IDL file containing the wrapper type and Streams API interfaces */
                        snprintf(fname,
                                strlen(basename) + MAX_FILE_POSTFIX_LENGTH,
                                "%sStreams.idl", basename);
                        idl_fileSetCur(idl_fileOutNew(fname, "w"));
                        if (idl_fileCur() == NULL) {
                            idl_reportOpenError(fname);
                        }
                        idl_walk(base, filename, source, traceWalk, idl_genCxxStreamsIdlProgram());
                        idl_fileOutFree(idl_fileCur());
                    }

                    if (idl_getCorbaMode() == IDL_MODE_STANDALONE)
                    {
                        /* Call cppgen for both the user provided IDL file (filename),
                         * and the generated IDL file (fname).
                         */
                        char cpp_command[MAX_CPP_COMMAND];
                        cpp_command[0] = '\0';

                        for (i = 0; i < c_iterLength(includeDefinitions); i++)
                        {
                            /* Extend command line with all include path options */
                            os_strncat (cpp_command, " -I", (size_t)3);
                            os_strncat (cpp_command, QUOTE, strlen(QUOTE));
                            os_strncat (cpp_command, c_iterObject(includeDefinitions, i),
                                     (size_t)(sizeof(cpp_command)-strlen(cpp_command)));
                            os_strncat (cpp_command, QUOTE, strlen(QUOTE));
                        }
                        /* Put the path to the dds_dcps.idl in the -I's for cppgen at the end */
                        templ_path = os_getenv ("OSPL_TMPL_PATH");
                        if (templ_path == NULL)
                        {
                           printf ("Variable OSPL_TMPL_PATH not defined\n");
                           exit (1);
                        }
                        snprintf(fnameA, sizeof(fnameA), "%s", templ_path);
                        os_strncat (cpp_command, " -I", (size_t)3);
                        os_strncat (cpp_command, QUOTE, strlen(QUOTE));
                        os_strncat (cpp_command, fnameA, strlen(fnameA));
                        os_strncat (cpp_command, QUOTE, strlen(QUOTE));


                        for (i = 0; i < c_iterLength(macroDefinitions); i++)
                        {
                            /* Extend command line with all macro definitions options */
                            os_strncat (cpp_command, " -D", (size_t)3);
                            os_strncat (cpp_command, QUOTE, strlen(QUOTE));
                            os_strncat (cpp_command, c_iterObject(macroDefinitions, i),
                                     (size_t)(sizeof(cpp_command)-strlen(cpp_command)));
                            os_strncat (cpp_command, QUOTE, strlen(QUOTE));
                        }

                        if(idl_getIsISOCpp())
                        {
                            os_strncat (cpp_command, " -isocpp", (size_t)8);
                        }

                        /* First on the dcps idl file that is generated by idlpp */
                        if (runCppGen (outputDir, cpp_command, dcpsIdlFileName, FALSE) != 0)
                        {
                           unlink(dcpsIdlFileName);
                           reportErrorAndExit("running cppgen.");
                        }
                        unlink(dcpsIdlFileName);
                        os_free(dcpsIdlFileName);


                        if(idl_getIsISOCppTypes())
                        {
                            /* Extend with the -iso type generation option if required */
                            os_strncat (cpp_command, " -iso", (size_t)5);
                            if (idl_getIsISOCppTestMethods())
                            {
                                os_strncat (cpp_command, " -isotest", (size_t)9);
                            }
                            if(idl_getIsGenEquality())
                            {
                                os_strncat (cpp_command, " -genequality", (size_t)13);
                            }
                        }

                        /* Now on the orignal idl file. Depending on the -i parameter, also generate code for interfaces */
                        if (runCppGen (outputDir, cpp_command, filename, cpp_ignoreInterfaces) != 0)
                        {
                           unlink(fname);
                           reportErrorAndExit("running cppgen.");
                        }
                    }

                }
            } else if (idl_getLanguage() == IDL_LANG_C) {
                os_char* tmp;
                idl_definitionClean();

                snprintf(fname,
                    strlen(basename) + MAX_FILE_POSTFIX_LENGTH,
                    "%s.h", basename);
                idl_fileSetCur(idl_fileOutNew(fname, "w"));
                if (idl_fileCur() == NULL) {
                    idl_reportOpenError(fname);
                }
                idl_fileOutPrintf(idl_fileCur(), "#if defined (__cplusplus)\n", basename);
                idl_fileOutPrintf(idl_fileCur(), "extern \"C\" {\n", basename);
                idl_fileOutPrintf(idl_fileCur(), "#endif\n\n", basename);
                idl_fileOutPrintf(idl_fileCur(), "#include \"%sSacDcps.h\"\n\n", basename);
                idl_fileOutPrintf(idl_fileCur(), "#if defined (__cplusplus)\n", basename);
                idl_fileOutPrintf(idl_fileCur(), "}\n", basename);
                idl_fileOutPrintf(idl_fileCur(), "#endif\n", basename);
                idl_fileOutFree(idl_fileCur());

                /* Generate the file that defines all DCPS specialized classes for the
                   user types
                */
                snprintf(fname,
                    strlen(basename) + MAX_FILE_POSTFIX_LENGTH,
                    "%sDcps.h", basename);
                idl_fileSetCur(idl_fileOutNew(fname, "w"));
                if (idl_fileCur() == NULL) {
                    idl_reportOpenError(fname);
                }
                idl_walk(base, filename, source, traceWalk, idl_genSacTypeProgram());
                idl_fileOutFree(idl_fileCur());

                /* Generate the file that specifies all Standalone C specialized data types for the
                   user types
                */
                snprintf(fname,
                    strlen(basename) + strlen(suffix) + MAX_FILE_POSTFIX_LENGTH,
                    "%s%sSplDcps.h", basename, suffix);
                idl_fileSetCur(idl_fileOutNew (fname, "w"));
                if (idl_fileCur() == NULL) {
                    idl_reportOpenError(fname);
                }
                if (dcpsTypes) {
                    idl_fileOutPrintf(idl_fileCur(), "#include \"dds_dcpsSplDcps.h\"\n\n");
                }
                tmp = os_malloc((size_t)((int)strlen(basename)+strlen(suffix)+strlen("Dcps"))+1);
                snprintf
                    (tmp,
                    (size_t)((int)strlen(basename) +strlen(suffix)+ strlen("Dcps")+1),
                    "%s%sDcps", basename, suffix);
                idl_genSpliceTypeSetIncludeFileName(tmp);
                os_free(tmp);
                idl_genSpliceTypeUseVoidPtrs(TRUE);
                idl_walk(base, filename, source, traceWalk, idl_genSpliceTypeProgram());
                idl_fileOutFree(idl_fileCur());

                /* Generate the file that implements all metadata load functions, Splice helper
                   functions as well as C/CORBA copy routines to/from Splice for the user types
                */
                snprintf(fname,
                    strlen(basename) + MAX_FILE_POSTFIX_LENGTH,
                    "%sSplDcps.c", basename);
                idl_fileSetCur(idl_fileOutNew(fname, "w"));
                if (idl_fileCur() == NULL) {
                    idl_reportOpenError(fname);
                }

                idl_fileOutPrintf(idl_fileCur(), "#if defined (__cplusplus)\n", basename);
                idl_fileOutPrintf(idl_fileCur(), "extern \"C\" {\n", basename);
                idl_fileOutPrintf(idl_fileCur(), "#endif\n\n", basename);

                idl_fileOutPrintf(idl_fileCur(), "#include \"%sSplDcps.h\"\n", basename);
                idl_fileOutPrintf(idl_fileCur(), "#include \"%sDcps.h\"\n\n", basename);

                idl_walk(base, filename, source, traceWalk, idl_genCorbaCHelperProgram());
                idl_walk(base, filename, source, traceWalk, idl_genCorbaCCopyinProgram());
                idl_walk(base, filename, source, traceWalk, idl_genCorbaCCopyoutProgram());
                idl_walk(base, filename, source, traceWalk, idl_genSacObjectControlProgram());
                idl_walk(base, filename, source, traceWalk, idl_genSacMetaProgram());

                idl_fileOutPrintf(idl_fileCur(), "#if defined (__cplusplus)\n", basename);
                idl_fileOutPrintf(idl_fileCur(), "}\n", basename);
                idl_fileOutPrintf(idl_fileCur(), "#endif\n", basename);

                idl_fileOutFree(idl_fileCur());

                /* Generate the file that defines all DCPS specialized classes for the
                   user types
                */
                snprintf(fname,
                    strlen(basename) + MAX_FILE_POSTFIX_LENGTH,
                    "%sSacDcps.h", basename);
                idl_fileSetCur(idl_fileOutNew(fname, "w"));
                if (idl_fileCur() == NULL) {
                    idl_reportOpenError(fname);
                }
               idl_walk(base, filename, source, traceWalk, idl_genSacTypedClassDefsProgram());
                idl_fileOutFree(idl_fileCur());

                /* Generate the file that implements all DCPS specialized classes for the
                   user types
                */
                snprintf(fname, (size_t)((int)strlen(basename)+20), "%sSacDcps.c", basename);
                idl_fileSetCur(idl_fileOutNew(fname, "w"));
                if (idl_fileCur() == NULL) {
                    idl_reportOpenError(fname);
                }

                idl_fileOutPrintf(idl_fileCur(), "#if defined (__cplusplus)\n", basename);
                idl_fileOutPrintf(idl_fileCur(), "extern \"C\" {\n", basename);
                idl_fileOutPrintf(idl_fileCur(), "#endif\n\n", basename);

                idl_walk(base, filename, source, traceWalk, idl_genSacTypedClassImplProgram());

                idl_fileOutPrintf(idl_fileCur(), "#if defined (__cplusplus)\n", basename);
                idl_fileOutPrintf(idl_fileCur(), "}\n", basename);
                idl_fileOutPrintf(idl_fileCur(), "#endif\n", basename);

                idl_fileOutFree(idl_fileCur());

                idl_definitionClean();

            } else if (idl_getLanguage() == IDL_LANG_CS) {
                SACSTypeUserData csUserData;
                SACSSplDcpsUserData splUserData;

                /**
                 * It is not possible to generate type-descriptors while walking over
                 * the data. This is caused by the fact that the walk holds a lock for
                 * the type it is currently processing, while the serializer that needs
                 * to generate the typedescriptor will try to claim the same lock.
                 * For that purpose we will use an iterator that caches the relevant
                 * meta-data on the first walk, and then process and generate the type
                 * descriptors in the template inititialization phase of the 2nd walk,
                 * where it does not hold any type locks yet.
                 */
                csUserData.idlpp_metaList = NULL;
                csUserData.tmplPrefix = NULL;
                csUserData.customPSM = customPSM;

                idl_definitionClean();

                /* Generate the file that defines all DCPS specialized classes for the
                 * user types
                 */
                snprintf(fname,
                    strlen(basename) + MAX_FILE_POSTFIX_LENGTH,
                    "%s.cs", basename);
                idl_fileSetCur(idl_fileOutNew(fname, "w"));
                if (idl_fileCur() == NULL) {
                    idl_reportOpenError(fname);
                }

                idl_walk(base, filename, source, traceWalk, idl_genSACSTypeProgram(&csUserData));
                idl_fileOutFree(idl_fileCur());

                /* Generate the file that defines the database representation of
                 * the IDL data, and that contains the marshalers that translate
                 * between database representation and C# representation.
                 */
                splUserData.customPSM = customPSM;
                snprintf(fname,
                    strlen(basename) + MAX_FILE_POSTFIX_LENGTH,
                    "%sSplDcps.cs", basename);
                idl_fileSetCur(idl_fileOutNew(fname, "w"));
                if (idl_fileCur() == NULL) {
                    idl_reportOpenError(fname);
                }

                idl_walk(base, filename, source, traceWalk, idl_genSACSSplDcpsProgram(&splUserData));
                idl_fileOutFree(idl_fileCur());

                /* Expand Typed Csharp TypeSupport, DataReader and DataWriter
                 * implementation classes.
                 */
                csUserData.tmplPrefix = "SacsTypedClassBody";
                snprintf(fname,
                    strlen(basename) + MAX_FILE_POSTFIX_LENGTH,
                    "%sDcps.cs", basename);
                idl_fileSetCur(idl_fileOutNew(fname, "w"));
                if (idl_fileCur() == NULL) {
                    idl_reportOpenError(fname);
                }
                idl_walk(
                        base,
                        filename,
                        source,
                        traceWalk,
                        idl_genSACSTypedClassDefsProgram(&csUserData));
                idl_fileOutFree(idl_fileCur());

                /* Expand Typed Csharp DataReader and DataWriter interfaces.
                 * There is no need for the meta-data in this phase anymore.
                 */
                csUserData.tmplPrefix = "SacsTypedClassSpec";
                csUserData.idlpp_metaList = NULL;
                snprintf(fname,
                    strlen(basename) + MAX_FILE_POSTFIX_LENGTH,
                    "I%sDcps.cs", basename);
                idl_fileSetCur(idl_fileOutNew(fname, "w"));
                if (idl_fileCur() == NULL) {
                    idl_reportOpenError(fname);
                }
                idl_walk(base, filename, source, traceWalk, idl_genSACSTypedClassDefsProgram(&csUserData));
                idl_fileOutFree(idl_fileCur());
            } else if (idl_getLanguage() == IDL_LANG_JAVA) {
                idl_genJavaHelperInit(orgPackageList, tarPackageList);
                idl_walk(base, filename, source, traceWalk, idl_genSajMetaProgram());
                idl_walk(base, filename, source, traceWalk, idl_genSajTypedClassProgram());
                if (idl_getCorbaMode() == IDL_MODE_ORB_BOUND) {
                    /* Expand IDL based application TypeSupport, DataReader and DataWriter interfaces */
                    idl_walk(base, filename, source, traceWalk, idl_genIdlHelperProgram());
                    snprintf(fname, strlen(basename) + MAX_FILE_POSTFIX_LENGTH, "%sDcps.idl", basename);
                    idl_fileSetCur(idl_fileOutNew(fname, "w"));
                    if (idl_fileCur() == NULL) {
                        idl_reportOpenError(fname);
                    }
                    idl_walk(base, filename, source, traceWalk, idl_genIdlProgram());
                    idl_fileOutFree(idl_fileCur());
                } else {
                    idl_walk(base, filename, source, traceWalk, idl_genSajTypeProgram());
                    idl_walk(base, filename, source, traceWalk, idl_genSajHolderProgram());
                }
            }
        }

        if (makeMetrics) {
            idl_genMetaSize(filename);
        }

        if (makeSpliceType) {
            /* For design based tests, generate Splice types from user types */
            snprintf(fname,
                strlen(basename) + MAX_FILE_POSTFIX_LENGTH,
                "%sSplType.h", basename);
            idl_fileSetCur(idl_fileOutNew(fname, "w"));
            if (idl_fileCur() == NULL) {
                idl_reportOpenError(fname);
            }
            idl_genSpliceTypeSetTestMode(TRUE);
            idl_walk(base, filename, source, traceWalk, idl_genSpliceTypeProgram());
            idl_fileOutFree(idl_fileCur());
        }

        if (makeSpliceLoad) {
            /* For design based tests, generate metadata load functions */
            snprintf(fname,
                strlen(basename) + MAX_FILE_POSTFIX_LENGTH,
                "%sSplLoad.c", basename);
            idl_fileSetCur(idl_fileOutNew(fname, "w"));
            if (idl_fileCur() == NULL) {
                idl_reportOpenError(fname);
            }
            idl_walk(base, filename, source, traceWalk, idl_genSpliceLoadProgram());
            idl_fileOutFree(idl_fileCur());
        }

        if (makeSpliceCopy) {
            /* For design based tests, generate C++/CORBA copy functions */
            snprintf(fname,
                strlen(basename) + MAX_FILE_POSTFIX_LENGTH,
                "%sSplCopy.c", basename);
            idl_fileSetCur(idl_fileOutNew(fname, "w"));
            if (idl_fileCur() == NULL) {
                idl_reportOpenError(fname);
            }
            switch (idl_getLanguage()) {
            case IDL_LANG_CXX:
                {
                    idl_program ci = idl_genCorbaCxxCopyinProgram();
                    /* If the suffix is empty, static type-caching is enabled.
                     * Disabled otherwise. */
                    ci->userData = suffix;
                    idl_walk(base, filename, source, traceWalk, ci);
                }
                idl_walk(base, filename, source, traceWalk, idl_genCorbaCxxCopyoutProgram());
            break;
            case IDL_LANG_C:
                idl_walk(base, filename, source, traceWalk, idl_genCorbaCCopyinProgram());
                idl_walk(base, filename, source, traceWalk, idl_genCorbaCCopyoutProgram());
            break;
            case IDL_LANG_JAVA:
            case IDL_LANG_CS:
                /* No copy code will be generated */
            break;
            case IDL_LANG_UNKNOWN:
            default:
            break;
            }
            idl_fileOutFree(idl_fileCur());
        }

        if (makeSpliceHelper) {
            /* For design based tests, generate helper functions */
            snprintf(fname,
                strlen(basename) + MAX_FILE_POSTFIX_LENGTH,
                "%sSplHelp.c", basename);
            idl_fileSetCur(idl_fileOutNew (fname, "w"));
            if (idl_fileCur() == NULL) {
                idl_reportOpenError(fname);
            }
            switch (idl_getLanguage()) {
            case IDL_LANG_CXX:
                idl_walk(base, filename, source, traceWalk, idl_genCorbaCxxHelperProgram());
            break;
            case IDL_LANG_C:
                idl_walk(base, filename, source, traceWalk, idl_genCorbaCHelperProgram());
            break;
            case IDL_LANG_JAVA:
                idl_walk(base, filename, source, traceWalk, idl_genCorbaJavaHelperProgram());
            break;
            case IDL_LANG_CS:
                /* No Helper code will be generated (yet...) */
            break;
            case IDL_LANG_UNKNOWN:
            default:
            break;
            }
            idl_fileOutFree(idl_fileCur());
        }

        if (makeCorbaType) {
            /* For design based tests, generate language type definitions/classes */
            snprintf(fname,
                strlen(basename) + MAX_FILE_POSTFIX_LENGTH,
                "%sCorbaType.h", basename);
            idl_fileSetCur(idl_fileOutNew(fname, "w"));
            if (idl_fileCur() == NULL) {
                idl_reportOpenError(fname);
            }
            switch (idl_getLanguage()) {
            case IDL_LANG_CXX:
                /* Not yet supported */
            break;
            case IDL_LANG_C:
                idl_walk(base, filename, source, traceWalk, idl_genSacTypeProgram());
            break;
            case IDL_LANG_JAVA:
                idl_walk(base, filename, source, traceWalk, idl_genSajTypeProgram());
            break;
            case IDL_LANG_UNKNOWN:
                printf("error\n");
            default:
            break;
            }
            idl_fileOutFree(idl_fileCur());
        }

        if (makeDeallocCode) {
            /* For design based tests, generate helper functions */
            snprintf(fname,
                strlen(basename) + MAX_FILE_POSTFIX_LENGTH,
                "%sObjCtrl.c", basename);
            idl_fileSetCur(idl_fileOutNew(fname, "w"));
            if (idl_fileCur() == NULL) {
                idl_reportOpenError(fname);
            }
            switch (idl_getLanguage()) {
            case IDL_LANG_CXX:
                /* Not yet supported */
            break;
            case IDL_LANG_C:
                idl_walk(base, filename, source, traceWalk, idl_genSacObjectControlProgram());
            break;
            case IDL_LANG_JAVA:
                /* Not yet supported */
            break;
            case IDL_LANG_UNKNOWN:
            default:
            break;
            }
            idl_fileOutFree(idl_fileCur());
        }
        if (makeMetaData) {
            sd_serializer metaSer;
            sd_serializedData serData;
            char *metaDescription = NULL;
            c_metaObject type;

            type = c_metaResolve((c_metaObject)base, (const char *)typeName);
            if (type) {
                metaSer = sd_serializerXMLMetadataNew (base);
                if (metaSer) {
                    serData = sd_serializerSerialize(metaSer, c_object(type));
                    if (serData) {
                        metaDescription = sd_serializerToString(metaSer, serData);
                    }
                    sd_serializerFree(metaSer);
                }
                printf("%s\n", metaDescription);
                os_free(metaDescription);
            } else {
                printf("Specified type %s not found\n", typeName);
                returnCode = -1;
            }
        }
        if (makeTypeInfo) {
            sd_serializer metaSer;
            sd_serializedData serData;
            char *metaDescription = NULL;
            c_metaObject type;

            type = c_metaResolve((c_metaObject)base, (const char *)typeName);
            if (type) {
                metaSer = sd_serializerXMLTypeinfoNew(base, FALSE);
                if (metaSer) {
                    serData = sd_serializerSerialize(metaSer, c_object(type));
                    if (serData) {
                        metaDescription = sd_serializerToString(metaSer, serData);
                    }
                    sd_serializerFree(metaSer);
                }
                printf("%s\n", metaDescription);
                os_free(metaDescription);
            } else {
                printf("Specified type %s not found\n", typeName);
                returnCode = -1;
            }
        }
        if (makeRegisterType) {
            idl_registerType(base, basename, typeNames);
            typeName = c_iterTakeFirst(typeNames);
            while (typeName != NULL) {
                os_free(typeName);
                typeName = c_iterTakeFirst(typeNames);
            }
            c_iterFree (typeNames);
        }

        /* Free resources related to the file dependencies */
        idl_depDefExit();
        os_free(fname);
    } else {
        returnCode = -1;
    }
    if (attachDatabase) {
        /* Detach from the database */
        os_sharedMemoryDetach(sHandle);
        os_sharedDestroyHandle(sHandle);
    }
    os_free(suffix);
    os_free(filename);
    os_free(basename);
    os_free(ccppOrbPath);
    if (includeIdlDir) {
        os_free(includeIdlDir);
    }
    if (tmpArg) {
        os_free(tmpArg);
    }
    if (outputDir) {
        os_free(outputDir);
        idl_dirOurFree();
    }
    if(clientHeader)
    {
        os_free(clientHeader);
    }
    os_serviceStop();

    c_iterFree(tarPackageList);
    c_iterFree(orgPackageList);

    idl_dllExit();
    return returnCode;
}

int runCppGen (
   char* outputDir,
   char* cpp_command,
   char* filename,
   c_bool ignoreInterfaces
)
{
    char* cppgenArgs;
    const char* CPPGEN_COMMAND = "cppgen" OS_EXESUFFIX;
    os_result osr;
    char* extIdlpp;
    os_procAttr cppgenProcAttr;
    os_procId cppgenProcId;
    os_int32 cppgenExitStatus;
    os_time sleepTime;
    char* cppgenIgnoreInterfaces;

    extIdlpp = os_locate(CPPGEN_COMMAND, OS_ROK|OS_XOK);
    if (!extIdlpp)
    {
      printf ("Error: Unable to os_locate the %s executable. Please check the PATH is correctly set. PATH is currently: %s\n", CPPGEN_COMMAND, os_getenv ("PATH"));
        return -1;
    }
    osr = os_procAttrInit(&cppgenProcAttr);
    if (osr != os_resultSuccess)
    {
        os_free(extIdlpp);
        return -1;
    }
    cppgenProcAttr.activeRedirect = TRUE;

    if (ignoreInterfaces) {
        cppgenIgnoreInterfaces="-ignore_interfaces";
    } else {
        cppgenIgnoreInterfaces="";
    }

    if (outputDir)
    {
        if (strcmp(idl_dllGetMacro(), "") != 0)
        {
            int macroHeaderLength = 0;
            if(idl_dllGetHeaderFile() && strlen(idl_dllGetHeaderFile()) != 0)
            {
                macroHeaderLength = strlen(idl_dllGetHeaderFile()) + 1;
            }
            cppgenArgs = os_malloc(strlen(cpp_command) +
                                strlen(cppgenIgnoreInterfaces) + 2 /* spaces */ +
                                16 + strlen(idl_dllGetMacro()) + macroHeaderLength +
                                9 /* -output=*/+ strlen(QUOTE) +
                                strlen(idl_dirOutCur()) +
                                strlen(QUOTE) + 1 +
                                strlen(QUOTE) + strlen(filename) + strlen(QUOTE) + 1);
            if(idl_dllGetHeaderFile() && strlen(idl_dllGetHeaderFile()) != 0)
            {
                os_sprintf(cppgenArgs, "%s %s -import_export=%s,%s -output=%s%s%s %s%s%s",
                     cpp_command,
                     cppgenIgnoreInterfaces,
                     idl_dllGetMacro(),
                     idl_dllGetHeaderFile(),
                     QUOTE, idl_dirOutCur(), QUOTE,
                     QUOTE, filename, QUOTE);
            } else
            {
                os_sprintf(cppgenArgs, "%s %s -import_export=%s -output=%s%s%s %s%s%s",
                     cpp_command,
                     cppgenIgnoreInterfaces,
                     idl_dllGetMacro(),
                     QUOTE, idl_dirOutCur(), QUOTE,
                     QUOTE, filename, QUOTE);
            }
        }
        else
        {
            cppgenArgs = os_malloc(strlen(cpp_command) +
                                strlen(cppgenIgnoreInterfaces) + 2 /* spaces */ +
                                8 /*-output=*/+ strlen(QUOTE) +
                                strlen(idl_dirOutCur()) + strlen(QUOTE) + 1 +
                                strlen(QUOTE) + strlen(filename) + strlen(QUOTE) + 1);
            os_sprintf(cppgenArgs, "%s %s -output=%s%s%s %s%s%s",
                 cpp_command,
                 cppgenIgnoreInterfaces,
                 QUOTE, idl_dirOutCur(), QUOTE,
                 QUOTE, filename, QUOTE);
        }
    }
    else
    {
        if (strcmp(idl_dllGetMacro(), "") != 0)
        {
            int macroHeaderLength = 0;
            if(idl_dllGetHeaderFile() && strlen(idl_dllGetHeaderFile()) != 0)
            {
                macroHeaderLength = strlen(idl_dllGetHeaderFile()) + 1;
            }
            cppgenArgs = os_malloc(strlen(cpp_command) +
                                strlen(cppgenIgnoreInterfaces) + 2 /* spaces */ +
                                16 + strlen(idl_dllGetMacro()) + macroHeaderLength +
                                strlen(QUOTE) + strlen(filename) + strlen(QUOTE) + 1);
            if(idl_dllGetHeaderFile() && strlen(idl_dllGetHeaderFile()) != 0)
            {
                os_sprintf(cppgenArgs, "%s %s -import_export=%s,%s %s%s%s",
                     cpp_command,
                     cppgenIgnoreInterfaces,
                     idl_dllGetMacro(),
                     idl_dllGetHeaderFile(),
                     QUOTE, filename, QUOTE);
            } else
            {
                os_sprintf(cppgenArgs, "%s %s -import_export=%s %s%s%s",
                     cpp_command,
                     cppgenIgnoreInterfaces,
                     idl_dllGetMacro(),
                     QUOTE, filename, QUOTE);
            }
        }
        else
        {
         cppgenArgs = os_malloc(strlen(cpp_command) +
                                strlen(cppgenIgnoreInterfaces) + 2 /* spaces */ +
                                strlen(QUOTE) + strlen(filename) + strlen(QUOTE) + 1);
         os_sprintf(cppgenArgs, "%s %s %s%s%s",
                 cpp_command,
                 cppgenIgnoreInterfaces,
                 QUOTE, filename, QUOTE);
        }
    }
    /* printf("Running: %s%s\n", extIdlpp, cppgenArgs);*/
    osr = os_procCreate(extIdlpp, "cppgen",  cppgenArgs,
                       &cppgenProcAttr, &cppgenProcId);
    os_free(cppgenArgs);
    if (osr != os_resultSuccess)
    {
      return -1;
    }
    sleepTime.tv_sec  = 0;
    sleepTime.tv_nsec = 100000000; /*100 ms*/
    do
    {
      osr = os_procCheckStatus(cppgenProcId, &cppgenExitStatus);
      if (osr != os_resultSuccess)
      {
         os_nanoSleep(sleepTime);
      }
    } while (osr != os_resultSuccess);

    os_free(extIdlpp);
    return cppgenExitStatus;
}
