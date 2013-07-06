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

#ifndef U__WRITER_H
#define U__WRITER_H

#include "u_writer.h"

void     u_writerResend(u_writer w, os_time *nextPeriod);
u_result u_writerDeinit(u_writer w);


#endif
