#ifndef __EXAMPLE_UTILITIES_H__
#define __EXAMPLE_UTILITIES_H__

#include <stdlib.h>
#ifndef _WIN32
#include <unistd.h>
#endif
#include "os.h"

/**
 * @file
 * This file defines some simple common utility functions for use in the OpenSplice C and C++ examples.
 */

#if defined (__cplusplus)
namespace {
#else
static
#endif

/**
 * Sleep for the specified period of time.
 * @param milliseconds The period that should be slept for in milliseconds.
 */
void exampleSleepMilliseconds(int milliseconds)
{
#ifdef _WIN32
   Sleep(milliseconds);
#else
   usleep(milliseconds * 1000);
#endif
}

#if defined (__cplusplus)
}
#endif

/*
 * The below define is normally off. It enables richer doxygen
 * documentation auto-linking.
 */
#ifdef DOXYGEN_FOR_ISOCPP
#include "workaround.h"
#endif

#endif
