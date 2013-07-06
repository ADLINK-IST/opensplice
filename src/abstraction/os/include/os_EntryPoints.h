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

#ifndef OS_ENTRYPOINTS_H
#define OS_ENTRYPOINTS_H

typedef struct os_entryPoint
{
  const char *symname;
#ifdef _WRS_KERNEL
  int (*entrypoint)(char *args);
#else
  int (*entrypoint)(int argc, char **argv);
#endif
} os_entryPoint;

typedef struct os_librarySymbols
{
  const char *execname;
  os_entryPoint *entryPoints;
} os_librarySymbols;


extern os_librarySymbols os_staticLibraries[];

#endif
