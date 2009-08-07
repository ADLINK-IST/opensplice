/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2009 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE
 *
 *   for full copyright notice and license terms.
 *
 */
#include <string.h>

#include <os.h>

#include <sd_serializerXMLTypeinfo.h>
#include <sd_serializerXMLMetadata.h>
#include "c_stringSupport.h"

#include "idl_parser.h"
#include "idl_walk.h"
#include "idl_dependencies.h"
#include "idl_genLanguageHelper.h"
#include "idl_genSpliceDep.h"
#include "idl_fileMap.h"

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
#include "idl_genCxxIdl.h"

/* Standalone C++ related support */
#include "idl_genSACPPTypedClassDefs.h"
#include "idl_genSACPPTypedClassImpl.h"
#include "idl_genSACPPType.h"
#include "idl_genSACPPTypeImpl.h"

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

/* Java related support */
#include "idl_genCorbaJavaHelper.h"
#include "idl_genJavaHelper.h"
#include "idl_genSajType.h"
#include "idl_genSajHolder.h"
#include "idl_genSajTypedClass.h"
#include "idl_genSajMeta.h"
#include "idl__genSajMeta.h"

/* register Type code */
#include "idl_registerType.h"

/* include preprocessor */
#include "dds_cpp.h"

/* include DLL related header file */
#include "idl_dll.h"

#define DDS_DCPS_DEF "dds_dcps.idl"
#define MAX_FILE_POSTFIX_LENGTH (20) /* maximum length postfixed to base file name */
#define MAX_CPP_COMMAND (4192)

const char* IDLCPP_COMMAND = "idlcpp" OS_EXESUFFIX;

#ifdef WIN32
    const char* DEFAULT_ORB = "DDS_OpenFusion_1_5_1";
    const char* QUOTE = "\"";
    const char* IDLPP_CMD_OPTIONS = "P:d:o:b:m:t:c:hECSD:I:l:";
#else
    const char* DEFAULT_ORB = "DDS_OpenFusion_1_4_1";
    const char* QUOTE = "";
    const char* IDLPP_CMD_OPTIONS = "d:o:b:m:t:c:hECSD:I:l:";
#endif

static void
print_usage(
    char *name)
{
    printf("Usage: %s [-c preprocessor-path] [-b ORB-template-path]\n"
           "       [-I path] [-D macro[=definition]] [-S | -C] \n"
           "       [-l (c | c++ | cpp | cs | java)] [-d directory] \n"
#ifdef WIN32
           "       [-P dll_macro_name[,<h-file>]] [-o dds-types] <filename>\n", name);
#else
           "       [-o dds-types] <filename>\n", name);
#endif
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
        "       This is the default mode for the C and Java language.\n");
    printf(
        "    -C\n"
        "       Defines ORB bound mode, which allows application programs to be\n"
        "       build and run integrated with an ORB. This mode is the default mode\n"
        "       for the C++ language.\n");
#ifdef WIN32
    printf(
        "    -P dll_macro_name[,<header-file>]\n"
        "       Only applicable to C and C++. Sets export macro that will be\n"
        "       prefixed to all functions in the generated code. This allows\n"
        "       creating DLL's from generated code. Optionally a header file\n"
        "       can be given that will be included in each generated file.\n");
#endif
    printf(
        "    -d directory\n"
        "       Specify the location where to place the generated files.\n");
    printf(
        "    -o dds-types\n"
        "       Enables support for standard DDS-DCPS definitions. The OpenSplice\n"
        "       preprocessor provides definitions for constants and types as defined\n"
        "       in the OMG-DDS-DCPS PSM. This implies that these definitions can be\n"
        "       used within application IDL.\n");
    printf(
        "       For C, OSPL_ORB_PATH will by default be set to SAC.\n"
        "       For C++, OSPL_ORB_PATH will by default be set to CCPP%s%s.\n"
        "       For Java, OSPL_ORB_PATH will by default be set to SAJ.\n",
        os_fileSep(), DEFAULT_ORB);
    printf(
            "    -l (c | c++ | cpp | cs | java)\n"
            "       Defines the target language, where 'cs' represents C-sharp\n"
            "       and 'cpp' is just an alias for 'c++'.\n"
            "       Note that the OpenSplice preprocessor does not support any\n"
            "       combination of mode and language. The default setting of the\n"
            "       OpenSplice preprocessor is c++.\n");
    printf(
            "    <filename>\n"
            "       Specifies the IDL input file to process.\n");
    printf("\n"
        "    Supported languages, ORBs and modes:\n"
        "       Lang    ORB    mode   library        OSPL_ORB_PATH\n"
        "       ---------------------------------------------------\n"
        "       c       N.A.   S      dcpssac        SAC\n"
        "       c++     N.A.   C      dcpsccpp       CCPP%s%s\n"
        "       cpp     N.A.   C      dcpsccpp       CCPP%s%s\n"
        "       c++     N.A.   S      dcpssacpp      SACPP\n"
        "       cpp     N.A.   S      dcpssacpp      SACPP\n"
        "       cs      N.A.   S      dcpssacs       SACS\n"
        "       Java    N.A.   S      dcpssaj        SAJ\n",
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
    printf ("Error opening file %s for writing\n", fname);
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
    snprintf(fname, sizeof(fname), "%s%c%s", templ_path, OS_FILESEPCHAR, DDS_DCPS_DEF);
    base = idl_parseFile(fname, 0);

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
    char *value;
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
    define(def, -1, (unsigned char *)value, DEF_CMDLINE);
    return result;
}


#if 0
/* Not needed anymore, we now have os_locate */

static c_char*
getIdlcppPath()
{
    char *path;
    char *d;
    char *c;
    const char *fsep;
    char* resultPath;
    c_iter dirs;

    resultPath = NULL;

    /* If the command contains an absolute or relative path,
       only check the permissions, otherwise search the file
       in the PATH environment
    */
    fsep = os_fileSep();

    /*First check if idlcpp is available as $(OSPL_HOME)/bin/idlcpp */
    path = os_getenv("OSPL_HOME");

    if (path != NULL) {
        c = (char *)os_malloc(strlen(path) + strlen(fsep) + strlen(fsep) + strlen(IDLCPP_COMMAND) + 4);

        if (c != NULL) {
            sprintf(c, "%s%sbin%s%s", path, fsep, fsep, IDLCPP_COMMAND);

            if (os_access(c, OS_ROK | OS_XOK) == os_resultSuccess) {
                resultPath = c;
                c = NULL;
            } else {
                os_free(c);
            }
        }
    }
    /*idlcpp is not available as $(OSPL_HOME)/bin/idlcpp. Now checking PATH */
    if (!resultPath) {
        path = os_getenv("PATH");
        dirs = c_splitString(path, os_pathSep()); /* ':' for unix and ';' for windows */
        d = (char *)c_iterTakeFirst(dirs);

        while (d != NULL) {
            if (resultPath == NULL) {
                c = (char *)os_malloc(strlen(d) + strlen(fsep) + strlen(IDLCPP_COMMAND) + 1);

                if (c != NULL) {
                    strcpy(c, d);
                    strcat(c, fsep);
                    strcat(c, IDLCPP_COMMAND);
                    /* Check file permissions. Do not have to check if file exists, since
                       permission check fails when the file does not exist.
                    */
                    if (os_access(c, OS_ROK | OS_XOK) == os_resultSuccess) {
                        resultPath = c;
                        c = NULL;
                    } else {
                        os_free(c);
                    }
                }
            }
            os_free(d);
            d = (char *)c_iterTakeFirst(dirs);
        }
        c_iterFree(dirs);
    }
    return resultPath;
}
#endif

static char*
getDefaultCcppOrbPath()
{
    char* ccppOrbPath;
    const char* api = "OSPL_ORB_PATH=CCPP";
    const char* filesep = os_fileSep();

    ccppOrbPath = (char*)(os_malloc(strlen(api) + strlen(filesep) + strlen(DEFAULT_ORB) + 1));
    sprintf(ccppOrbPath, "%s%s%s", api, filesep, DEFAULT_ORB);

    return ccppOrbPath;
}

static void
addIncludeToEorbGeneratedFile(
    char *fileName)
{
    c_char *fileContent, *insertionPoint;
    const char *includeStatement;
    const char *insertionTarget = "#include";
    struct os_stat fileStat;
    int fileHandle, nrBytes;
    unsigned int nRead;

    /* Only proceed when the file is accessible and an include statement needs to be inserted. */
    includeStatement = idl_dllGetHeader();
    if ((includeStatement != NULL) &&
        (os_stat(fileName, &fileStat) == os_resultSuccess) &&
        (os_access(fileName, OS_ROK) == os_resultSuccess)) {
        /* Open the file and read it contents into a buffer. */
        fileContent = os_malloc((size_t)((int)fileStat.stat_size+1));
        fileHandle = open(fileName, O_RDONLY);
        nRead = (unsigned int)read(fileHandle, fileContent, (size_t)fileStat.stat_size);
        close(fileHandle);

        /* Adjust end of buffer to compensate for reduction caused by alternative line-endings. */
        memset(&fileContent[nRead], 0, (size_t)((int)fileStat.stat_size+1-nRead));

        /* Determine the first include statement as the insertion point. */
        insertionPoint = fileContent;
        while (strncmp(insertionPoint, insertionTarget, strlen(insertionTarget)) != 0) {
            insertionPoint = strchr(insertionPoint, '\n');
            if (insertionPoint == NULL) return;
            insertionPoint++;
        }

        /* Overwrite the file with the part preceding the insertion point. */
        fileHandle = open(fileName, O_WRONLY);
        nrBytes = (int) (insertionPoint - fileContent);
        write(fileHandle, fileContent, nrBytes);

        /* Insert the specified include statement. */
        write(fileHandle, includeStatement, strlen(includeStatement));
        write(fileHandle, "\n", 1);

        /* Write the remainder of the original file. */
        write(fileHandle, insertionPoint, nRead - nrBytes);
        close(fileHandle);
        os_free(fileContent);
    }
}

int
main (
    int argc,
    char* argv[])
{
    c_base base;
    char *fpathNorm;
    char *ptr;
    char *extension;
    char *filename;
    char *basename;
    char *databaseName;
    char *moduleName;
    char *cpp_arg = NULL;
    char *typeName = NULL;
    char orb[256];
    char* outputDir = NULL;
    char *eOrbOutputFile;
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
    c_bool attachDatabase = FALSE;
    os_sharedAttr sharedAttr;
    os_sharedHandle sHandle;
    os_result result;
    int opt;
    int i;
    os_size_t len;
    c_iter includeDefinitions = NULL;
    c_iter macroDefinitions = NULL;
    c_iter typeNames = NULL;
    os_result osr;
    char* ccppOrbPath;
    int returnCode = 0;

    /* Use a unique name, so pass NULL as parameter */
    osr = os_serviceStart(NULL);
    if (osr != os_resultSuccess) {
        printf("Failed to initialize.\n");
        exit(2);
    }
    os_osInit();

    init_preprocess();

    idl_dllInitialize(); /* initialise */

    ccppOrbPath = getDefaultCcppOrbPath();
    while ((opt = getopt(argc, argv, IDLPP_CMD_OPTIONS)) != -1) {
        switch (opt) {
        case 'h':
            print_help(argv[0]);
            idl_exit(0);
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
        case 'd':
            if (outputDir) {
                printf("Option '-d' can only be set once.\n");
                print_usage(argv[0]);
                idl_exit(-1);
            }
            outputDir = os_fileNormalize(optarg);
        break;
        case 'C':
            if (idl_getCorbaMode() != IDL_MODE_UNKNOWN) {
                printf("ERROR --> Mode already set to: %s\n", idl_getCorbaModeStr());
                print_usage(argv[0]);
                idl_exit(-1);
            }
            idl_setCorbaMode(IDL_MODE_ORB_BOUND);
        break;
#ifdef WIN32
        case 'P':
            if (idl_dllSetOption(optarg)) {
                print_usage(argv[0]);
                idl_exit(-1);
            }
        break;
#endif
        case 'S':
            if (idl_getCorbaMode() != IDL_MODE_UNKNOWN) {
                printf("ERROR --> Mode already set to: %s\n", idl_getCorbaModeStr());
                print_usage(argv[0]);
                idl_exit(-1);
            }
            idl_setCorbaMode(IDL_MODE_STANDALONE);
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
            /* Trace the proeprocessing process */
            tracePreProcessed = TRUE;
        break;
        case 'I':
            /* Define an include PATH, which is used while preprocessing the IDL
               input file. The include path is stored via an iterator.
            */
            /* also append it to the preprocessor include path */
            Ifile(optarg);
            includeDefinitions = c_iterAppend(includeDefinitions, optarg);
        break;
        case 'o':
            if (strcmp(optarg, "dds-types") == 0) {
                dcpsTypes = TRUE;
            } else {
                print_usage(argv[0]);
                idl_exit(-1);
            }
        break;
        case '?':
            print_usage(argv[0]);
            idl_exit(-1);
        break;
        }
    }
    if ((optind >= argc) || ((argc-optind) > 1)) {
        print_usage(argv[0]);
        idl_exit(-1);
    }

    /* check if mode has been set */
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
        printf("ERROR --> Combination of language and corba mode not supported: LANG=%s MODE=%s\n",
            idl_getLanguageStr(), idl_getCorbaModeStr());
        print_usage(argv[0]);
        idl_exit(-1);
    }
	if (outputDir) {
        outputDirSetSuccess = idl_dirOutNew(outputDir);

        if(!outputDirSetSuccess){
            idl_exit(-1);
        }
    }

    fpathNorm = os_fileNormalize(argv[optind]);
    /* Determine the start of the IDL input filename, removing all path related stuff.
       The information is used to determine the basename of the input IDL file. The
       basename is the filename of the input file without path and extension.
       Also take into account non-alphanum characters and replace with '_'
    */
    ptr = os_rindex(fpathNorm, OS_FILESEPCHAR);
    if (ptr == NULL) {
        ptr = fpathNorm;
    } else {
        ptr++;
    }
    extension = os_rindex(ptr, '.');
    if (extension) {
        *extension = '\0';
        len = ((os_size_t)extension - (os_size_t)ptr);
    } else {
        len = strlen(ptr);
    }
    basename = os_malloc(len + 1);
    strncpy(basename, (const char *)ptr, len);
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
            char *fname[1024];
            /* add include path OSPL_TMPL_PATH as it contains the file dds_dcps.idl needed by
             * eOrb compiler.
             */
            if (os_getenv("OSPL_HOME") == NULL) {
                printf ("Variable OSPL_HOME not defined\n");
                idl_exit(1);
            }
            snprintf(fname, 1024, "%s/etc/idlpp",os_getenv("OSPL_HOME"));
            includeDefinitions = c_iterAppend(includeDefinitions,fname);
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
    } else {
        /* Attach to an existing database */
        os_sharedAttrInit(&sharedAttr);
        sHandle = os_sharedCreateHandle(databaseName, &sharedAttr);
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

        /* Allocate space for the output file name */
        c_char *fname = os_malloc((size_t)((int)strlen(basename)+MAX_FILE_POSTFIX_LENGTH));

        /* Initialize class to determine depedencies with other files */
        idl_depDefInit();
        /* Walk through metadata to determine depedencies with other files,
           the dependencies are used to generate include statements
        */
        idl_walk(base, filename, traceWalk, idl_genSpliceDepProgram());

        if (makeAll) {
            /* Do normal generation */

            if (idl_getLanguage() == IDL_LANG_CXX) {
                if (idl_getCorbaMode() == IDL_MODE_STANDALONE) {
                    os_putenv("OSPL_ORB_PATH=SACPP");
                } else if (os_getenv("OSPL_ORB_PATH") == NULL) {
                    os_putenv(ccppOrbPath);
                }

                /* Generate the file that defines all DCPS specialized classes for the
                   user types
                */
                snprintf(fname,
                    (size_t)((int)strlen(basename) + MAX_FILE_POSTFIX_LENGTH),
                    "%sDcps_impl.h", basename);

                idl_fileSetCur(idl_fileOutNew(fname, "w"));
                if (idl_fileCur() == NULL) {
                    idl_reportOpenError(fname);
                }
                idl_walk(base, filename, traceWalk, idl_genCxxTypedClassDefsProgram());
                idl_fileOutFree(idl_fileCur());

                /* Generate the file that implements all DCPS specialized classes for the
                   user types
                */
                snprintf(fname,
                    (size_t)((int)strlen(basename) + MAX_FILE_POSTFIX_LENGTH),
                    "%sDcps_impl.cpp", basename);
                idl_fileSetCur(idl_fileOutNew(fname, "w"));
                if (idl_fileCur() == NULL) {
                    idl_reportOpenError(fname);
                }
                idl_walk(base, filename, traceWalk, idl_genCxxTypedClassImplProgram());
                idl_walk(base, filename, traceWalk, idl_genCxxMetaProgram());
                idl_fileOutFree(idl_fileCur());

                /* Generate the file that specifies all Splice specialized data types for the
                   user types
                */
                snprintf(fname,
                    (size_t)((int)strlen(basename) + MAX_FILE_POSTFIX_LENGTH),
                    "%sSplDcps.h", basename);
                idl_fileSetCur(idl_fileOutNew (fname, "w"));
                if (idl_fileCur() == NULL) {
                    idl_reportOpenError(fname);
                }
                if (dcpsTypes) {
                    idl_fileOutPrintf(idl_fileCur(), "#include \"ccpp_ddsDcpsSplDcps.h\"\n\n");
                }
                idl_walk(base, filename, traceWalk, idl_genSpliceTypeProgram());
                idl_fileOutFree(idl_fileCur());

                /* Generate the file that implements all metadata load functions, Splice helper
                   functions as well as C++/CORBA copy routines to/from Splice for the user types
                */
                snprintf(fname,
                    (size_t)((int)strlen(basename) + MAX_FILE_POSTFIX_LENGTH),
                    "%sSplDcps.cpp", basename);
                idl_fileSetCur(idl_fileOutNew(fname, "w"));
                if (idl_fileCur() == NULL) {
                    idl_reportOpenError(fname);
                }
                idl_fileOutPrintf(idl_fileCur(), "#include \"%sSplDcps.h\"\n", basename);
                idl_fileOutPrintf(idl_fileCur(), "#include \"ccpp_%s.h\"\n", basename);
                idl_fileOutPrintf(idl_fileCur(), "\n");
                idl_walk(base, filename, traceWalk, idl_genCorbaCxxHelperProgram());
                idl_walk(base, filename, traceWalk, idl_genCorbaCxxCopyinProgram());
                idl_walk(base, filename, traceWalk, idl_genCorbaCxxCopyoutProgram());
                idl_fileOutFree(idl_fileCur());

                /* Generate the file that includes ORB specific generated header files
                */
                snprintf(fname,
                    (size_t)((int)strlen(basename) + MAX_FILE_POSTFIX_LENGTH),
                    "ccpp_%s.h", basename);
                idl_fileSetCur(idl_fileOutNew(fname, "w"));
                if (idl_fileCur() == NULL) {
                    idl_reportOpenError(fname);
                }
                idl_walk(base, filename, traceWalk, idl_genCorbaCxxCcppProgram());
                idl_fileOutFree(idl_fileCur());

                if (idl_getCorbaMode() == IDL_MODE_ORB_BOUND) {
                    /* Expand IDL based application TypeSupport, DataReader and DataWriter interfaces
                    */
                    snprintf(fname,
                        (size_t)((int)strlen(basename) + MAX_FILE_POSTFIX_LENGTH),
                        "%sDcps.idl", basename);
                    idl_fileSetCur(idl_fileOutNew(fname, "w"));
                    if (idl_fileCur() == NULL) {
                        idl_reportOpenError(fname);
                    }
                    idl_walk(base, filename, traceWalk, idl_genCxxIdlProgram());
                    idl_fileOutFree(idl_fileCur());
                }

                if (idl_getCorbaMode() == IDL_MODE_STANDALONE) {
#if 0
                    /* Call e*ORB pre-processor for both the user provided IDL file (filename)
                     * and the generated IDL file (fname).
                     */
                    char cpp_command[MAX_CPP_COMMAND];
                    cpp_command[0] = '\0';
                    for (i = 0; i < c_iterLength(includeDefinitions); i++) {
                        /* Extend command line with all include path options */
                        strncat (cpp_command, " -I", (size_t)3);
                        strncat (cpp_command, QUOTE, strlen(QUOTE));
                        strncat (cpp_command, c_iterObject(includeDefinitions, i), (size_t)(sizeof(cpp_command)-strlen(cpp_command)));
                        strncat (cpp_command, QUOTE, strlen(QUOTE));
                    }
                    for (i = 0; i < c_iterLength(macroDefinitions); i++) {
                        /* Extend command line with all macro definitions options */
                        strncat (cpp_command, " -D", (size_t)3);
                        strncat (cpp_command, QUOTE, strlen(QUOTE));
                        strncat (cpp_command, c_iterObject(macroDefinitions, i), (size_t)(sizeof(cpp_command)-strlen(cpp_command)));
                        strncat (cpp_command, QUOTE, strlen(QUOTE));
                    }
                    /* extIdlpp = getIdlcppPath(); */
                    extIdlpp = os_locate(IDLCPP_COMMAND, OS_ROK|OS_XOK);

                    if (!extIdlpp) {
                        unlink(fname);
                        reportErrorAndExit("idlcpp not found.");
                    }
                    osr = os_procAttrInit(&idlcppProcAttr);

                    if (osr != os_resultSuccess) {
                        os_free(extIdlpp);
                        unlink(fname);
                        reportErrorAndExit("idlcpp could not be initialised.");
                    }

                    /* Run idlcpp on the same idl file. */
                    idlcppProcAttr.activeRedirect = TRUE;
                    idlcppStdArgs = " -dds ";

                    if (outputDir) {
                        if (idl_dllGetMacro() != NULL) {
                            idlcppArgs = os_malloc(strlen(cpp_command) +
                                             strlen(idlcppStdArgs) +
                                             16 + strlen(idl_dllGetMacro()) +
                                             9 /* -output=*/+ strlen(QUOTE) + strlen(idl_dirOutCur()) + strlen(QUOTE) + 1 +
                                             strlen(QUOTE) + strlen(filename) + strlen(QUOTE) + 1);
                            sprintf(idlcppArgs, "%s%s-import_export=%s -output=%s%s%s %s%s%s",
                                                cpp_command, idlcppStdArgs, idl_dllGetMacro(),
                                                QUOTE, idl_dirOutCur(), QUOTE,
                                                QUOTE, filename, QUOTE);
                        } else {
                            idlcppArgs = os_malloc(strlen(cpp_command) +
                                             strlen(idlcppStdArgs) +
                                             8 /*-output=*/+ strlen(QUOTE) + strlen(idl_dirOutCur()) + strlen(QUOTE) + 1 +
                                             strlen(QUOTE) + strlen(filename) + strlen(QUOTE) + 1);
                            sprintf(idlcppArgs, "%s%s-output=%s%s%s %s%s%s",
                                                    cpp_command, idlcppStdArgs,
                                                    QUOTE, idl_dirOutCur(), QUOTE,
                                                    QUOTE, filename, QUOTE);
                        }
                    } else {
                        if (idl_dllGetMacro() != NULL) {
                            idlcppArgs = os_malloc(strlen(cpp_command) +
                                             strlen(idlcppStdArgs) +
                                             16 + strlen(idl_dllGetMacro()) +
                                             strlen(QUOTE) + strlen(filename) + strlen(QUOTE) + 1);
                            sprintf(idlcppArgs, "%s%s-import_export=%s %s%s%s",
                                        cpp_command, idlcppStdArgs, idl_dllGetMacro(),
                                        QUOTE, filename, QUOTE);
                        } else {
                            idlcppArgs = os_malloc(strlen(cpp_command) +
                                             strlen(idlcppStdArgs) +
                                             strlen(QUOTE) + strlen(filename) + strlen(QUOTE) + 1);
                            sprintf(idlcppArgs, "%s%s%s%s%s", cpp_command, idlcppStdArgs, QUOTE, filename, QUOTE);
                        }
                    }
                    printf("Running: %s%s\n", extIdlpp, idlcppArgs);
                    osr = os_procCreate(extIdlpp, "idlcpp",  idlcppArgs,
                                &idlcppProcAttr, &idlcppProcId);
                    os_free(idlcppArgs);

                    if (osr != os_resultSuccess) {
                        unlink(fname);
                        reportErrorAndExit("idlcpp could not be started.");
                    }

                    sleepTime.tv_sec  = 0;
                    sleepTime.tv_nsec = 100000000; /*100 ms*/
                    do {
                        osr = os_procCheckStatus(idlcppProcId, &idlcppExitStatus);

                        if (osr != os_resultSuccess) {
                            os_nanoSleep(sleepTime);
                        }
                    } while (osr != os_resultSuccess);

                    if (idlcppExitStatus != 0) {
                        os_free(extIdlpp);
                        unlink(fname);
                        idl_exit(idlcppExitStatus);
                    }
                    if (idl_dirOutCur() == NULL) {
                        eOrbOutputFile = os_malloc(strlen(basename) + 2 /* ".h" */ + 1);
                        sprintf(eOrbOutputFile, "%s.h", basename);
                    } else {
                        eOrbOutputFile = os_malloc(strlen(idl_dirOutCur()) + 1 /* file separator */ + strlen(basename) + 2 /* ".h" */ + 1);
                        sprintf(eOrbOutputFile, "%s%c%s.h", idl_dirOutCur(), OS_FILESEPCHAR, basename);
                    }
                    addIncludeToEorbGeneratedFile(eOrbOutputFile);
                    os_free(eOrbOutputFile);
#else
                    /* Generate the header file for SACPP types */
                    snprintf(fname,
                             (size_t)((int)strlen(basename) + MAX_FILE_POSTFIX_LENGTH),
                             "%s.h", basename);

                    idl_fileSetCur(idl_fileOutNew(fname, "w"));
                    if (idl_fileCur() == NULL) {
                        idl_reportOpenError(fname);
                    }
                    idl_walk(base, filename, traceWalk, idl_genSacppTypeProgram());
                    idl_fileOutFree(idl_fileCur());

                    /* Generate the implementation file for SACPP types */
                    snprintf(fname,
                             (size_t)((int)strlen(basename) + MAX_FILE_POSTFIX_LENGTH),
                             "%s.cpp", basename);

                    idl_fileSetCur(idl_fileOutNew(fname, "w"));
                    if (idl_fileCur() == NULL) {
                        idl_reportOpenError(fname);
                    }
                    idl_walk(base, filename, traceWalk, idl_genSacppTypeImplProgram());
                    idl_fileOutFree(idl_fileCur());

#endif

                    /* Generate the header file for the typed readers and writers */
                    snprintf(fname,
                             (size_t)((int)strlen(basename) + MAX_FILE_POSTFIX_LENGTH),
                             "%sDcps.h", basename);

                    idl_fileSetCur(idl_fileOutNew(fname, "w"));
                    if (idl_fileCur() == NULL) {
                        idl_reportOpenError(fname);
                    }
                    idl_walk(base, filename, traceWalk, idl_genSACPPTypedClassDefsProgram());
                    idl_fileOutFree(idl_fileCur());

                    /* Generate the source file for the typed readers and writers */
                    snprintf(fname,
                             (size_t)((int)strlen(basename) + MAX_FILE_POSTFIX_LENGTH),
                             "%sDcps.cpp", basename);

                    idl_fileSetCur(idl_fileOutNew(fname, "w"));
                    if (idl_fileCur() == NULL) {
                        idl_reportOpenError(fname);
                    }
                    idl_walk(base, filename, traceWalk, idl_genSACPPTypedClassImplProgram());
                    idl_fileOutFree(idl_fileCur());

                    if (idl_dirOutCur() == NULL) {
                        eOrbOutputFile = os_malloc(strlen(basename) + 6 /* "Dcps.h" */ + 1);
                        sprintf(eOrbOutputFile, "%sDcps.h", basename);
                    } else {
                        eOrbOutputFile = os_malloc(strlen(idl_dirOutCur()) + 1 /* file separator */ + strlen(basename) + 6 /* "Dcps.h" */ + 1);
                        sprintf(eOrbOutputFile, "%s%c%sDcps.h", idl_dirOutCur(), OS_FILESEPCHAR, basename);
                    }
                    addIncludeToEorbGeneratedFile(eOrbOutputFile);
                    os_free(eOrbOutputFile);
                }
            } else if (idl_getLanguage() == IDL_LANG_C) {

                idl_definitionClean();

                snprintf(fname,
                    (size_t)((int)strlen(basename) + MAX_FILE_POSTFIX_LENGTH),
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
                    (size_t)((int)strlen(basename) + MAX_FILE_POSTFIX_LENGTH),
                    "%sDcps.h", basename);
                idl_fileSetCur(idl_fileOutNew(fname, "w"));
                if (idl_fileCur() == NULL) {
                    idl_reportOpenError(fname);
                }
                idl_walk(base, filename, traceWalk, idl_genSacTypeProgram());
                idl_fileOutFree(idl_fileCur());

                /* Generate the file that specifies all Standalone C specialized data types for the
                   user types
                */
                snprintf(fname,
                    (size_t)((int)strlen(basename) + MAX_FILE_POSTFIX_LENGTH),
                    "%sSplDcps.h", basename);
                idl_fileSetCur(idl_fileOutNew (fname, "w"));
                if (idl_fileCur() == NULL) {
                    idl_reportOpenError(fname);
                }
                if (dcpsTypes) {
                    idl_fileOutPrintf(idl_fileCur(), "#include \"dds_dcpsSplDcps.h\"\n\n");
                }
                idl_walk(base, filename, traceWalk, idl_genSpliceTypeProgram());
                idl_fileOutFree(idl_fileCur());

                /* Generate the file that implements all metadata load functions, Splice helper
                   functions as well as C/CORBA copy routines to/from Splice for the user types
                */
                snprintf(fname,
                    (size_t)((int)strlen(basename) + MAX_FILE_POSTFIX_LENGTH),
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

                idl_walk(base, filename, traceWalk, idl_genCorbaCHelperProgram());
                idl_walk(base, filename, traceWalk, idl_genCorbaCCopyinProgram());
                idl_walk(base, filename, traceWalk, idl_genCorbaCCopyoutProgram());
                idl_walk(base, filename, traceWalk, idl_genSacObjectControlProgram());
                idl_walk(base, filename, traceWalk, idl_genSacMetaProgram());

                idl_fileOutPrintf(idl_fileCur(), "#if defined (__cplusplus)\n", basename);
                idl_fileOutPrintf(idl_fileCur(), "}\n", basename);
                idl_fileOutPrintf(idl_fileCur(), "#endif\n", basename);

                idl_fileOutFree(idl_fileCur());

                /* Generate the file that defines all DCPS specialized classes for the
                   user types
                */
                snprintf(fname,
                    (size_t)((int)strlen(basename) + MAX_FILE_POSTFIX_LENGTH),
                    "%sSacDcps.h", basename);
                idl_fileSetCur(idl_fileOutNew(fname, "w"));
                if (idl_fileCur() == NULL) {
                    idl_reportOpenError(fname);
                }
                idl_walk(base, filename, traceWalk, idl_genSacTypedClassDefsProgram());
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

                idl_walk(base, filename, traceWalk, idl_genSacTypedClassImplProgram());

                idl_fileOutPrintf(idl_fileCur(), "#if defined (__cplusplus)\n", basename);
                idl_fileOutPrintf(idl_fileCur(), "}\n", basename);
                idl_fileOutPrintf(idl_fileCur(), "#endif\n", basename);

                idl_fileOutFree(idl_fileCur());

                idl_definitionClean();

            } else if (idl_getLanguage() == IDL_LANG_CS) {
                /**
                 * It is not possible to generate type-descriptors while walking over
                 * the data. This is caused by the fact that the walk holds a lock for
                 * the type it is currently processing, while the serializer that needs
                 * to generate the typedescriptor will try to claim the same lock.
                 * For that purpose we will use an iterator that cashes the relevant
                 * meta-data on the first walk, and then process and generate the type
                 * descriptors in the template inititialization phase of the 3rd walk,
                 * where it does not hold any type locks yet.
                 */
                os_iter idlpp_metaList = NULL;

                idl_definitionClean();

                /* Generate the file that defines all DCPS specialized classes for the
                   user types
                */
                snprintf(fname,
                    (size_t)((int)strlen(basename) + MAX_FILE_POSTFIX_LENGTH),
                    "%s.cs", basename);
                idl_fileSetCur(idl_fileOutNew(fname, "w"));
                if (idl_fileCur() == NULL) {
                    idl_reportOpenError(fname);
                }

                idl_walk(base, filename, traceWalk, idl_genSACSTypeProgram(&idlpp_metaList));
                idl_fileOutFree(idl_fileCur());

                /* Expand Typed Csharp DataReader and DataWriter interfaces.
                */
                snprintf(fname,
                    (size_t)((int)strlen(basename) + MAX_FILE_POSTFIX_LENGTH),
                    "I%sDcps.cs", basename);
                idl_fileSetCur(idl_fileOutNew(fname, "w"));
                if (idl_fileCur() == NULL) {
                    idl_reportOpenError(fname);
                }
                idl_walk(
                        base,
                        filename,
                        traceWalk,
                        idl_genSACSTypedClassDefsProgram("SacsTypedClassSpec", NULL));
                idl_fileOutFree(idl_fileCur());

                /* Expand Typed Csharp TypeSupport, DataReader and DataWriter implementation classes.
                */
                snprintf(fname,
                    (size_t)((int)strlen(basename) + MAX_FILE_POSTFIX_LENGTH),
                    "%sDcps.cs", basename);
                idl_fileSetCur(idl_fileOutNew(fname, "w"));
                if (idl_fileCur() == NULL) {
                    idl_reportOpenError(fname);
                }
                idl_walk(
                        base,
                        filename,
                        traceWalk,
                        idl_genSACSTypedClassDefsProgram("SacsTypedClassBody", &idlpp_metaList));
                idl_fileOutFree(idl_fileCur());
            } else if (idl_getLanguage() == IDL_LANG_JAVA) {
                idl_walk(base, filename, traceWalk, idl_genSajTypeProgram());
                idl_walk(base, filename, traceWalk, idl_genSajHolderProgram());
                idl_walk(base, filename, traceWalk, idl_genSajMetaProgram());
                idl_walk(base, filename, traceWalk, idl_genSajTypedClassProgram());
            }
        }

        if (makeSpliceType) {
            /* For design based tests, generate Splice types from user types */
            snprintf(fname,
                (size_t)((int)strlen(basename) + MAX_FILE_POSTFIX_LENGTH),
                "%sSplType.h", basename);
            idl_fileSetCur(idl_fileOutNew(fname, "w"));
            if (idl_fileCur() == NULL) {
                idl_reportOpenError(fname);
            }
            idl_walk(base, filename, traceWalk, idl_genSpliceTypeProgram());
            idl_fileOutFree(idl_fileCur());
        }

        if (makeSpliceLoad) {
            /* For design based tests, generate metadata load functions */
            snprintf(fname,
                (size_t)((int)strlen(basename) + MAX_FILE_POSTFIX_LENGTH),
                "%sSplLoad.c", basename);
            idl_fileSetCur(idl_fileOutNew(fname, "w"));
            if (idl_fileCur() == NULL) {
                idl_reportOpenError(fname);
            }
            idl_walk(base, filename, traceWalk, idl_genSpliceLoadProgram());
            idl_fileOutFree(idl_fileCur());
        }

        if (makeSpliceCopy) {
            /* For design based tests, generate C++/CORBA copy functions */
            snprintf(fname,
                (size_t)((int)strlen(basename) + MAX_FILE_POSTFIX_LENGTH),
                "%sSplCopy.c", basename);
            idl_fileSetCur(idl_fileOutNew(fname, "w"));
            if (idl_fileCur() == NULL) {
                idl_reportOpenError(fname);
            }
            switch (idl_getLanguage()) {
            case IDL_LANG_CXX:
                idl_walk(base, filename, traceWalk, idl_genCorbaCxxCopyinProgram());
                idl_walk(base, filename, traceWalk, idl_genCorbaCxxCopyoutProgram());
            break;
            case IDL_LANG_C:
                idl_walk(base, filename, traceWalk, idl_genCorbaCCopyinProgram());
                idl_walk(base, filename, traceWalk, idl_genCorbaCCopyoutProgram());
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
                (size_t)((int)strlen(basename) + MAX_FILE_POSTFIX_LENGTH),
                "%sSplHelp.c", basename);
            idl_fileSetCur(idl_fileOutNew (fname, "w"));
            if (idl_fileCur() == NULL) {
                idl_reportOpenError(fname);
            }
            switch (idl_getLanguage()) {
            case IDL_LANG_CXX:
                idl_walk(base, filename, traceWalk, idl_genCorbaCxxHelperProgram());
            break;
            case IDL_LANG_C:
                idl_walk(base, filename, traceWalk, idl_genCorbaCHelperProgram());
            break;
            case IDL_LANG_JAVA:
                idl_walk(base, filename, traceWalk, idl_genCorbaJavaHelperProgram());
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
                (size_t)((int)strlen(basename) + MAX_FILE_POSTFIX_LENGTH),
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
                idl_walk(base, filename, traceWalk, idl_genSacTypeProgram());
            break;
            case IDL_LANG_JAVA:
                idl_walk(base, filename, traceWalk, idl_genSajTypeProgram());
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
                (size_t)((int)strlen(basename) + MAX_FILE_POSTFIX_LENGTH),
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
                idl_walk(base, filename, traceWalk, idl_genSacObjectControlProgram());
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
    os_free(filename);
    os_free(basename);
    os_free(ccppOrbPath);

    if (outputDir) {
        os_free(outputDir);
        idl_dirOurFree();
    }
    os_serviceStop();

    idl_dllExit();

    return returnCode;
}
