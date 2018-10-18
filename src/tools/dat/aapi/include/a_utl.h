/*
 *                         Vortex OpenSplice
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */
 */


#ifndef A_UTL_H
#define A_UTL_H

#include "c_typebase.h"


/**
 * \brief
 * Context for keeping track of start- and stop-time by means of a
 * stop watch.
 *
 * \see
 * a_utlStopWatchStart a_utlStopWatchStop
 * a_utlGetStopWatchTimeMilSecs a_utlGetStopWatchTimeMicroSecs
 */
typedef struct a_utlContext_s *a_utlContext;


/**
 * \brief
 * Initialises the context and returns a new pointer to it. It must
 * be called before using any of the stop watch functions.
 *
 * \note
 * Remember to de-init after use!
 *
 * \return
 * Pointer to a newly created context, or NULL if anything failed
 * (out of memory?).
 *
 * \see
 * a_utlDeInit
 */
a_utlContext a_utlInit();


/**
 * \brief
 * De-initialises the context
 *
 * \param context
 * The context to de-initialise
 *
 * \see
 * a_utlInit
 */
void a_utlDeInit(a_utlContext context);


/**
 * \brief
 * Starts the Stop Watch
 *
 * \param context
 * This file's context
 *
 * \see
 * a_utlContext a_utlStopWatchStop
 */
void a_utlStopWatchStart(a_utlContext context);


/**
 * \brief
 * Stops the Stop Watch
 *
 * \param context
 * This file's context
 *
 * \see
 * a_utlStopWatchStart
 */
void a_utlStopWatchStop(a_utlContext context);


/**
 * \brief
 * Returns the elapsed time, between starting and stopping the stop
 * watch, in milliseconds.
 *
 * \param context
 * This file's context
 *
 * \return
 * Elapsed time, between starting and stopping the stop watch, in
 * milliseconds.
 *
 * \see
 * a_utlGetStopWatchTimeMicroSecs
 * a_utlStopWatschStart a_utlStopWatchStop
 */
unsigned long a_utlGetStopWatchTimeMilSecs(a_utlContext context);


/**
 * \brief
 * Returns the elapsed time, between starting and stopping the stop
 * watch, in microseconds.
 *
 * \param context
 * This file's context
 *
 * \return
 * Elapsed time, between starting and stopping the stop watch, in
 * milliseconds.
 *
 * \see
 * a_utlGetStopWatchTimeMilSecs
 * a_utlStopWatschStart a_utlStopWatchStop
 */
unsigned long a_utlGetStopWatchTimeMicroSecs(a_utlContext context);


/**
 * \brief
 * Returns a (static) string representation of a c_metaKind value.
 *
 * \param kind
 * Kind of c_metaKind
 *
 * \return
 * Human readable string (static) of \a kind.
 */
char *a_utlMetaKindStr(c_metaKind kind);


/**
 * \brief
 * Returns a (static) string representation of a c_collKind value.
 *
 * \param kind
 * Kind of c_metaKind
 *
 * \return
 * Human readable string (static) of \a kind.
 */
char *a_utlCollKindStr(c_collKind kind);


/**
 * \brief
 * Returns a (static) string representation of a c_primKind value.
 *
 * \param kind
 * Kind of c_metaKind
 *
 * \return
 * Human readable string (static) of \a kind.
 */
char *a_utlPrimKindStr(c_primKind kind);


/**
 * \brief
 * Returns a (static) string representation of a c_exprKind value.
 *
 * \param kind
 * Kind of c_metaKind
 *
 * \return
 * Human readable string (static) of \a kind.
 */
char *a_utlExprKindStr(c_exprKind kind);


/**
 * \brief
 * Returns a (static) string representation of a c_direction value.
 *
 * \param dir
 * Kind of c_direction
 *
 * \return
 * Human readable string (static) of \a dir.
 */
char *a_utlDirectionStr(c_direction dir);


/**
 * \brief
 * Returns a (static) string representation of a c_valueKind value.
 *
 * \param kind
 * Kind of c_metaKind
 *
 * \return
 * Human readable string (static) of \a kind.
 */
char *a_utlValueKindStr(c_valueKind kind);


#endif

//END a_utl.h
