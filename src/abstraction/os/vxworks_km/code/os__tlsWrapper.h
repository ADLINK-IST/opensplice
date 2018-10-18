/*
 *                         Vortex OpenSplice
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
 *
 *   Licensed under the ADLINK Software License Agreement Rev 2.7 2nd October
 *   2014 (the "License"); you may not use this file except in compliance with
 *   the License.
 *   You may obtain a copy of the License at:
 *                      $OSPL_HOME/LICENSE
 *
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */

#ifndef OS_VXWORKS__TLS_WRAPPER_H
#define OS_VXWORKS__TLS_WRAPPER_H

#ifdef VXWORKS_55

#include <taskVarLib.h>

#define readTLSVarSelf(variable)		\
   (variable)

#define readTLSVar(taskid, variable ) \
   taskVarGet((taskid), (&(variable)))

#define addTLSVar(taskid, variable ) \
   taskVarAdd((taskid), variable)

#define setTLSVar(taskid, variable, value ) \
   taskVarSet((taskid), (variable), (value))

#define deleteTLSVar(taskid, variable) \
   taskVarDelete( (taskid), (variable) )

#else

#include "os__tls.h"

#define addTLSVar( taskid, variable ) \
   (os__tlsKeyCreate((taskid), (variable)) !=0 ? OK : ERROR)

#define readTLSVarSelf(variable ) \
   (os__tlsGet(taskIdSelf(), (&(variable))))

#define readTLSVar(taskid, variable ) \
   (os__tlsGet((taskid), (&(variable))))

#define setTLSVar(taskid, variable, value ) \
   (os__tlsSet((taskid), (variable), (void *)value ) !=0 ? OK : ERROR)

#define deleteTLSVar(taskid, variable) \
   (os__tlsKeyDestroy( (taskid), (variable) ) !=0 ? OK : ERROR)


#endif

#endif
