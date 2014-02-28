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

#ifndef __EXAMPLE_UTILITIES_H__
#define __EXAMPLE_UTILITIES_H__

#include <stdlib.h>
#include <time.h>
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

#define NS_IN_ONE_US 1000
#define US_IN_ONE_MS 1000
#define US_IN_ONE_SEC 1000000

/**
 * Sleep for the specified period of time.
 * @param milliseconds The period that should be slept for in milliseconds.
 */
void exampleSleepMilliseconds(unsigned long milliseconds)
{
#ifdef _WIN32
    Sleep((DWORD)milliseconds);
#elif defined USE_NANOSLEEP
    struct timespec sleeptime;
    struct timespec remtime;
    sleeptime.tv_sec = milliseconds / 1000;
    sleeptime.tv_nsec = (milliseconds % US_IN_ONE_MS) * US_IN_ONE_SEC;
    nanosleep(&sleeptime, &remtime);
#else
    usleep(milliseconds * US_IN_ONE_MS);
#endif
}

#define TIME_STATS_SIZE_INCREMENT 50000

/**
 * Struct to keep a running average time as well as recording the minimum
 * and maximum times
 */
typedef struct ExampleTimeStats
{
    unsigned long* values;
    unsigned long valuesSize;
    unsigned long valuesMax;
    double average;
    unsigned long min;
    unsigned long max;
    unsigned long count;
} ExampleTimeStats;

/**
 * Returns an ExampleTimeStats struct with zero initialised variables
 * @return An ExampleTimeStats struct with zero initialised variables
 */
ExampleTimeStats exampleInitTimeStats()
{
    ExampleTimeStats stats;
    stats.values = (unsigned long*)malloc(TIME_STATS_SIZE_INCREMENT * sizeof(unsigned long));
    stats.valuesSize = 0;
    stats.valuesMax = TIME_STATS_SIZE_INCREMENT;
    stats.average = 0;
    stats.min = 0;
    stats.max = 0;
    stats.count = 0;

    return stats;
}

/**
 * Resets an ExampleTimeStats struct variables to zero
 * @param stats An ExampleTimeStats struct to reset
 */
void exampleResetTimeStats(ExampleTimeStats* stats)
{
    memset(stats->values, 0, stats->valuesMax * sizeof(unsigned long));
    stats->valuesSize = 0;
    stats->average = 0;
    stats->min = 0;
    stats->max = 0;
    stats->count = 0;
}

/**
 * Deletes the ExampleTimeStats values
 * @param stats An ExampleTimeStats struct delete the values from
 */
void exampleDeleteTimeStats(ExampleTimeStats* stats)
{
    free(stats->values);
}

/**
 * Updates an ExampleTimeStats struct with new time data, keeps a running average
 * as well as recording the minimum and maximum times
 * @param stats ExampleTimeStats struct to update
 * @param microseconds A time in microseconds to add to the stats
 * @return The updated ExampleTimeStats struct
 */
ExampleTimeStats* exampleAddMicrosecondsToTimeStats(ExampleTimeStats* stats, unsigned long microseconds)
{
    if(stats->valuesSize > stats->valuesMax)
    {
        unsigned long* temp = (unsigned long*)realloc(stats->values, stats->valuesMax + TIME_STATS_SIZE_INCREMENT);
        if(temp)
        {
            stats->values = temp;
            stats->valuesMax += TIME_STATS_SIZE_INCREMENT;
        }
        else
        {
            printf("ERROR: Failed to expand values array");
        }
    }
    if(stats->valuesSize < stats->valuesMax)
    {
        stats->values[stats->valuesSize++] = microseconds;
    }
    stats->average = (stats->count * stats->average + microseconds)/(stats->count + 1);
    stats->min = (stats->count == 0 || microseconds < stats->min) ? microseconds : stats->min;
    stats->max = (stats->count == 0 || microseconds > stats->max) ? microseconds : stats->max;
    stats->count++;

    return stats;
}

/**
 * Compares two unsigned longs
 *
 * @param a an unsigned long
 * @param b an unsigned long
 * @param int -1 if a < b, 1 if a > b, 0 if equal
 */
int exampleCompareul(const void* a, const void* b)
{
    unsigned long ul_a = *((unsigned long*)a);
    unsigned long ul_b = *((unsigned long*)b);

    if(ul_a < ul_b) return -1;
    if(ul_a > ul_b) return 1;
    else return 0;
}

/**
 * Calculates the median time from an ExampleTimeStats
 *
 * @param stats the ExampleTimeStats
 * @return the median time
 */
double exampleGetMedianFromTimeStats(ExampleTimeStats* stats)
{
    double median = 0;

    qsort(stats->values, stats->valuesSize, sizeof(unsigned long), exampleCompareul);

    if(stats->valuesSize % 2 == 0)
    {
        median = (double)(stats->values[stats->valuesSize / 2 - 1] + stats->values[stats->valuesSize / 2]) / 2;
    }
    else
    {
        median = (double)stats->values[stats->valuesSize / 2];
    }

    return median;
}

/**
 * Gets the current time
 * @return A timeval struct representing the current time
 */
struct timeval exampleGetTime()
{
    struct timeval current_time;
    struct os_time os_current_time;

    os_current_time = os_timeGet();
    current_time.tv_sec = os_current_time.tv_sec;
    current_time.tv_usec = os_current_time.tv_nsec / NS_IN_ONE_US;

    return current_time;
}

/**
 * Adds one timeval to another
 * @param t1 to add to
 * @param t2 to add
 * @return A timeval struct representing the sum of two times
 */
struct timeval exampleAddTimevalToTimeval(const struct timeval* t1, const struct timeval* t2)
{
    struct timeval tr;

    if(t1->tv_usec + t2->tv_usec > US_IN_ONE_SEC)
    {
        tr.tv_usec = t1->tv_usec + t2->tv_usec - US_IN_ONE_SEC;
        tr.tv_sec = t1->tv_sec + t2->tv_sec + 1;
    }
    else
    {
        tr.tv_usec = t1->tv_usec + t2->tv_usec;
        tr.tv_sec = t1->tv_sec + t2->tv_sec;
    }

    return tr;
}

/**
 * Subtracts one timeval from another
 * @param t1 to subtract from
 * @param t2 to subtract
 * @return A timeval struct representing the difference between two times
 */
struct timeval exampleSubtractTimevalFromTimeval(const struct timeval* t1, const struct timeval* t2)
{
    struct timeval tr;

    if(t1->tv_usec >= t2->tv_usec)
    {
        tr.tv_usec = t1->tv_usec - t2->tv_usec;
        tr.tv_sec = t1->tv_sec - t2->tv_sec;
    }
    else
    {
        tr.tv_usec = t1->tv_usec - t2->tv_usec + US_IN_ONE_SEC;
        tr.tv_sec = t1->tv_sec - t2->tv_sec - 1;
    }

    return tr;
}

/**
 * Converts a timeval to microseconds
 * @param t The time to convert
 * @return The result of converting a timeval to microseconds
 */
unsigned long exampleTimevalToMicroseconds(const struct timeval* t)
{
    unsigned long tr;

    tr = ((unsigned long)t->tv_sec * US_IN_ONE_SEC) + t->tv_usec;
    return tr;
}

/**
 * Converts microseconds to a timeval
 * @param microseconds the number of microseconds to convert
 * @return The result of converting a timeval to microseconds
 */
struct timeval exampleMicrosecondsToTimeval(const unsigned long microseconds)
{
    struct timeval tr;

    tr.tv_sec = (long)(microseconds / US_IN_ONE_SEC);
    tr.tv_usec = (long)(microseconds - (tr.tv_sec * US_IN_ONE_SEC));
    return tr;
}

#if defined (__cplusplus)
void exampleResetTimeStats(ExampleTimeStats& stats)
{
    exampleResetTimeStats(&stats);
}

void exampleDeleteTimeStats(ExampleTimeStats& stats)
{
    exampleDeleteTimeStats(&stats);
}

ExampleTimeStats& operator+=(ExampleTimeStats& stats, unsigned long microseconds)
{
    return *exampleAddMicrosecondsToTimeStats(&stats, microseconds);
}

double exampleGetMedianFromTimeStats(ExampleTimeStats& stats)
{
    return exampleGetMedianFromTimeStats(&stats);
}

timeval operator+(const timeval& t1, const timeval& t2)
{
    return exampleAddTimevalToTimeval(&t1, &t2);
}

timeval operator-(const timeval& t1, const timeval& t2)
{
    return exampleSubtractTimevalFromTimeval(&t1, &t2);
}

unsigned long exampleTimevalToMicroseconds(const timeval& t)
{
    return exampleTimevalToMicroseconds(&t);
}

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
