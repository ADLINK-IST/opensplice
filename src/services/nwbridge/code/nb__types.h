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
#ifndef NB__TYPES_H_
#define NB__TYPES_H_

#include "c_typebase.h"

#define C_TYPEDEF(type, name) C_STRUCT(type) *name
/* C_SIZEOF uses C_STRUCT to determine the size; this is not safe for nb_object;
 * instead the macro could be defined as C_SIZEOF(name) sizeof(*name), but to
 * prevent confusion the macro is undeffed. */
#undef C_SIZEOF

C_CLASS(nb_object);
C_CLASS(nb_topicObject); /* Abstract class */
    C_CLASS(nb_dcpsTopic);
    C_CLASS(nb_dcpsParticipant);
    C_CLASS(nb_cmParticipant);
    C_CLASS(nb_dcpsPublication);
    C_CLASS(nb_cmWriter);
    C_CLASS(nb_dcpsSubscription);
    C_CLASS(nb_cmReader);
    C_CLASS(nb_cmPublisher);
    C_CLASS(nb_cmSubscriber);
C_CLASS(nb_thread);
C_CLASS(nb_service);
C_CLASS(nb_configuration);
C_CLASS(nb_group);

/* Classes that aren't reference-counted */
C_CLASS(nb_threadStates);
C_CLASS(nb_logConfig);

#endif /* NB__TYPES_H_ */
