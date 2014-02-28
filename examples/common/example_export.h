#ifndef __EXAMPLE_EXPORT_H__
#define __EXAMPLE_EXPORT_H__

/**
 * @file
 * This file defines some macros to control symbol visibility when building libraries
 * and handles portability issues for platforms that don't utilise a 'traditional'
 * int main(int, char**) function.
 */

#ifndef EXAMPLE_ENTRYPOINT
/**
 * Generate a C-ish entrypoint function taking program arguments as a single string
 * for use in the C++ API examples
 * @see C_EXAMPLE_ENTRYPOINT
 * @param tocreate The name of the C function to be created
 * @param tocall The argc/argv C++ function that is to be called onto.
 */
#define EXAMPLE_ENTRYPOINT(tocreate,tocall) \
extern "C" { \
    int tocreate (char * args) \
    { \
    int argc=1; \
    char *argv[256]; \
    char *str1; \
    argv[0] = strdup (#tocreate);\
    str1 = strtok(args, " "); \
    while (str1) \
    { \
        argv[argc] = strdup (str1); \
        argc++; \
        str1 = strtok(NULL, " "); \
    } \
    argv[argc] = NULL; \
    return tocall (argc, argv); \
    } \
}
#endif

#ifndef C_EXAMPLE_ENTRYPOINT
/**
 * Generate a C-ish entrypoint function taking program arguments as a single string
 * for use in the C API examples
 * On some embedded platforms it is useful to be able to export an entrypoint with
 * parameters as a single string with each individual argument separated
 * by a space. That is in contrast to the argc/argv style of parameter
 * passing. This macro generates such function defintions.
 * @param tocreate The name of the C function to be created
 * @param tocall The argc/argv C function that is to be called onto.
 */
#define C_EXAMPLE_ENTRYPOINT(tocreate,tocall) \
    int tocreate (char * args) \
    { \
    int argc=1; \
    char *argv[256]; \
    char *str1; \
    argv[0] = strdup (#tocreate);\
    str1 = strtok(args, " "); \
    while (str1) \
    { \
        argv[argc] = strdup (str1); \
        argc++; \
        str1 = strtok(NULL, " "); \
    } \
    argv[argc] = NULL; \
    return tocall (argc, argv); \
    }
#endif

/* Symbol visibility */
#include "os_if.h"
#ifdef OS_EXAMPLE_TYPES_EXPORT
#undef OS_EXAMPLE_TYPES_EXPORT
#endif

#ifdef OSPL_BUILDEXAMPLE_TYPES_LIB
#define OS_EXAMPLE_TYPES_EXPORT OS_API_EXPORT
#else
/** Symbol import / export macro for an example typesupport library */
#define OS_EXAMPLE_TYPES_EXPORT OS_API_IMPORT
#endif

#ifdef OS_EXAMPLE_IMPL_EXPORT
#undef OS_EXAMPLE_IMPL_EXPORT
#endif

#ifdef OSPL_BUILDEXAMPLE_IMPL_LIB
#define OS_EXAMPLE_IMPL_EXPORT OS_API_EXPORT
#else
/** Symbol import / export macro for an example implementation library */
#define OS_EXAMPLE_IMPL_EXPORT OS_API_IMPORT
#endif

#ifndef EXAMPLE_MAIN
/** This macro optionally replaces 'main' with the platform specific alternative */
#define EXAMPLE_MAIN main
#ifdef WINCE
#include "os_stdlib.h"
int main (int argc, char* argv[]);
int WINAPI WinMain
    (HINSTANCE hInst, HINSTANCE hPrevInst, LPTSTR lpCmdLine, int nCmdShow)
{
    int argc;
    char *argv[256];
    argc = os_mainparams (lpCmdLine, argv);
    return main (argc, argv);
}
#endif /* WINCE */
#endif /* ! EXAMPLE_MAIN */

#endif
