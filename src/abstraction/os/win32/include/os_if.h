/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2011 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE 
 *
 *   for full copyright notice and license terms. 
 *
 */
#ifndef OS_WIN32_IF_H
#define OS_WIN32_IF_H

#if defined (__cplusplus)
extern "C" {
#endif

#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>

#define OS_API_EXPORT __declspec(dllexport)
#define OS_API_IMPORT __declspec(dllimport)

#if defined (__cplusplus)
}
#endif

#endif /* OS_WIN32_IF_H */
