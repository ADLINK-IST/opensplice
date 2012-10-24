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
#ifndef SACPP_EXCEPTION_H
#define SACPP_EXCEPTION_H

#include "sacpp_DDS_DCPS.h"
#include "sacpp_if.h"

class SACPP_API DDS::Exception
{
   public:

      Exception () {};
      virtual ~Exception () {};

      static DDS::Exception* _downcast (DDS::Exception * e);
      static const DDS::Exception* _downcast (const DDS::Exception * e);

      virtual const char * _name () const = 0;
      virtual const char * _rep_id () const = 0;
      virtual void _raise () const = 0;
      virtual DDS::Exception_ptr _clone () const = 0;

      virtual DDS::UserException * _as_user ();
      virtual DDS::SystemException * _as_system ();
};

#undef SACPP_API
#endif
