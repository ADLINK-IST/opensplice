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
#ifndef SACPP_USEREXCEPTION_H
#define SACPP_USEREXCEPTION_H

#include "sacpp_DDS_DCPS.h"
#include "sacpp_Exception.h"
#include "sacpp_if.h"

class SACPP_API DDS::UserException : public DDS::Exception
{
public:

   static DDS::UserException* _downcast (DDS::Exception* exc);
   static const DDS::UserException* _downcast (const DDS::Exception* exc);

   virtual UserException *_as_user();
};

#undef SACPP_API
#endif
